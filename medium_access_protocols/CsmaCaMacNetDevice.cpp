/***********************************************************************************************//**
 *  Class that represents the Csma CA MAC device
 *  @class      CsmaCaNetDevice
 *  @author     Sergi Salvà Pibernat (SSP), sergi.salva@estudiantat.upc.edu
 *  @date       2022-march-2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#include "CsmaCaMacNetDevice.hpp"
#include "SpaceNetDevice.hpp"

LOG_COMPONENT_DEFINE("CsmaCaMacNetDevice");

CsmaCaMacNetDevice::CsmaCaMacNetDevice()
    : m_state(IDLE)
    , m_retry(0)
    , m_pkt_tx(0)
    , m_pkt_data(0)
    , m_cca_timeout_event()
    , m_backoff_timeout_event()
    , m_cts_timeout_event()
    , m_ack_timeout_event()
    , m_send_cts_event()
    , m_send_ack_event()
    , m_send_data_event()
{
    m_cw = m_cw_min;
    m_nav = ns3::Simulator::Now();
    m_local_nav = ns3::Simulator::Now();
    m_backoff_remain = ns3::Seconds(0);
    m_backoff_start = ns3::Seconds(0);
    m_sequence = 0;
}

CsmaCaMacNetDevice::~CsmaCaMacNetDevice(void)
{
    clear();
}

void CsmaCaMacNetDevice::clear(void)
{
    m_pkt_tx = 0;
    m_pkt_data = 0;
    m_queue->Initialize();
    m_seq_list.clear();
}

ns3::TypeId CsmaCaMacNetDevice::getTypeId(void)
{
    static ns3::TypeId tid = ns3::TypeId("CsmaCaMacNetDevice").SetParent<ns3::NetDevice>();
    return tid;
}

void CsmaCaMacNetDevice::setDevice(ns3::Ptr<ns3::NetDevice> dev)
{
    m_device = dev;
    setCw(m_cw_min);
}

ns3::Address CsmaCaMacNetDevice::GetBroadcast(void) const
{
    return ns3::Mac48Address::GetBroadcast();
}

bool CsmaCaMacNetDevice::enqueue(ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address destination)
{
    if(m_queue->GetCurrentSize() >= m_queue->GetMaxSize()) {
        return false;
    }
    packet->AddHeader(CsmaCaMacNetDeviceHeader(m_address, m_address, SW_PKT_TYPE_DATA));
    m_queue->Enqueue(packet);
    
    if(m_state == IDLE) { 
        ccaForDifs();
    }
    
    return false;
}

void CsmaCaMacNetDevice::sendPacketDone(ns3::Ptr<ns3::Packet> packet)
{
    if(m_state != TX || m_pkt_tx != packet){
        return;
    }

    m_state = IDLE;
    CsmaCaMacNetDeviceHeader header;
    packet->PeekHeader(header);
    
    switch(header.getType()) { 
        case SW_PKT_TYPE_RTS:
        case SW_PKT_TYPE_CTS:
            break;
        case SW_PKT_TYPE_DATA:
            if(header.getDestinationAddress() == GetBroadcast()) {
                sendDataDone(true);
                ccaForDifs();
                return;
            }
            break;
        case SW_PKT_TYPE_ACK:
            ccaForDifs();
            break;
        default:
            ccaForDifs();
            break;
    }
}

void CsmaCaMacNetDevice::setForwardUpCb(
    ns3::Callback<void,
    ns3::Ptr<ns3::Packet>,
    ns3::Mac48Address,
    ns3::Mac48Address> cb
)
{
    m_forward_up_cllbk = cb;
}

void CsmaCaMacNetDevice::SetAddress(ns3::Address address)
{
    m_address = ns3::Mac48Address::ConvertFrom(address);
    uint8_t temp[6];
    m_address.CopyTo (temp);
    ns3::SeedManager::SetSeed(temp[5]+9); 
}

ns3::Address CsmaCaMacNetDevice::GetMulticast(ns3::Ipv6Address addr) const
{
    return ns3::Mac48Address::GetMulticast(addr);
}

bool CsmaCaMacNetDevice::SetMtu(const uint16_t mtu)
{
    m_mtu = mtu;
    return true;
}

void CsmaCaMacNetDevice::AddLinkChangeCallback(ns3::Callback<void> callback)
{
    m_linkchange_cllbk.ConnectWithoutContext(callback);
}

void CsmaCaMacNetDevice::SetPromiscReceiveCallback(ns3::NetDevice::PromiscReceiveCallback cb)
{
    m_rxpromisc_cllbk = cb;
}

ns3::Address CsmaCaMacNetDevice::GetMulticast(ns3::Ipv4Address multicast_group) const
{
    return ns3::Mac48Address::GetMulticast(multicast_group);
}

bool CsmaCaMacNetDevice::Send(
    ns3::Ptr<ns3::Packet> packet,
    const ns3::Address& dest,
    uint16_t protocol_num
)
{
    return SendFrom(packet, m_device->GetAddress(), dest, protocol_num);
}

ns3::Time CsmaCaMacNetDevice::getCtrlDuration(uint16_t type)
{
    CsmaCaMacNetDeviceHeader header = CsmaCaMacNetDeviceHeader(m_address, m_address, type);
    return m_space_device->CalTxDuration(header.getSize(), 0, m_basic_rate, m_data_rate);
}

ns3::Time CsmaCaMacNetDevice::getDataDuration(ns3::Ptr<ns3::Packet> packet)
{   
    return m_space_device->CalTxDuration(0, packet->GetSize(), m_basic_rate, m_data_rate);
}

std::string CsmaCaMacNetDevice::stateToString(State state)
{
    switch(state) {
        case IDLE:
            return "IDLE";
        case BACKOFF:
            return "BACKOFF";
        case WAIT_TX:
            return "WAIT_TX";
        case TX:
            return "TX";
        case WAIT_RX:
            return "WAIT_RX";
        case RX:
            return "RX";
        case COLLISION:
            return "COLLISION";
        default:
            return "??";
    }
}

void CsmaCaMacNetDevice::ccaForDifs(void)
{   
    ns3:: Time now = ns3::Simulator::Now();
    
    if(m_queue->GetCurrentSize().GetValue() == 0 || m_cca_timeout_event.IsRunning()) {
        return;
    }
    
    ns3::Time nav = std::max(m_nav, m_local_nav);
    if(nav > now + getSlotTime()) {
        m_cca_timeout_event = 
            ns3::Simulator::Schedule(nav - now, &CsmaCaMacNetDevice::ccaForDifs, this);
        return;
    }
    
    if(m_state != IDLE || !m_space_device->IsIdle()) {
        m_cca_timeout_event = 
            ns3::Simulator::Schedule(getDifs(), &CsmaCaMacNetDevice::ccaForDifs, this);
        return;
    }

    m_cca_timeout_event = 
        ns3::Simulator::Schedule(getDifs(), &CsmaCaMacNetDevice::backoffStart, this);
}

void CsmaCaMacNetDevice::backoffStart(void)
{   
    if(m_state != IDLE || !m_space_device->IsIdle()) {
        ccaForDifs();
        return;
    }
    if(m_backoff_remain == ns3::Seconds(0)) {
        ns3::UniformRandomVariable uv;
        uint32_t cw = uv.GetInteger(0, m_cw - 1);
        m_backoff_remain = ns3::Seconds((double)(cw) * getSlotTime().GetSeconds());
    }
    m_backoff_start = ns3::Simulator::Now();
    m_backoff_timeout_event = 
        ns3::Simulator::Schedule (m_backoff_remain, &CsmaCaMacNetDevice::channelAccessGranted, this);
}

void CsmaCaMacNetDevice::channelBecomesBusy(void)
{
    if(m_backoff_timeout_event.IsRunning()) {
        m_backoff_timeout_event.Cancel();
        ns3::Time elapse;
        if(ns3::Simulator::Now() > m_backoff_start) {
            elapse = ns3::Simulator::Now() - m_backoff_start;
        }
        if(elapse < m_backoff_remain) {
            m_backoff_remain = m_backoff_remain - elapse;
            m_backoff_remain = roundOffTime(m_backoff_remain);
        }
    }
    ccaForDifs();
}

void CsmaCaMacNetDevice::channelAccessGranted(void)
{
    if(m_queue->GetCurrentSize().GetValue() == 0) { 
        return; 
    }
    
    m_backoff_start = ns3::Seconds(0);
    m_backoff_remain = ns3::Seconds(0);
    m_state = WAIT_TX;
    m_pkt_data = m_queue->Remove();
    
    CsmaCaMacNetDeviceHeader header;
    m_pkt_data->PeekHeader(header);
    
    if(header.getDestinationAddress() != GetBroadcast() &&  m_rts_enable == true) {
        sendRts(m_pkt_data);
    } else {
        sendData();
    }
}

void CsmaCaMacNetDevice::updateNav(ns3::Time nav)
{
    ns3::Time newNav;
    newNav = ns3::Simulator::Now() + nav;
    if(newNav > m_nav) { 
        m_nav = newNav;
    }
}

void CsmaCaMacNetDevice::updateLocalNav(ns3::Time nav)
{
    m_local_nav = ns3::Simulator::Now() + nav;
}

void CsmaCaMacNetDevice::sendRts(ns3::Ptr<ns3::Packet> pktData)
{
    CsmaCaMacNetDeviceHeader dataHeader;
    pktData->PeekHeader(dataHeader);
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(0);
    CsmaCaMacNetDeviceHeader rtsHeader = 
        CsmaCaMacNetDeviceHeader(m_address, dataHeader.getDestinationAddress(), SW_PKT_TYPE_RTS);
    
    ns3::Time nav = getSifs() + getCtrlDuration(SW_PKT_TYPE_CTS)
        + getSifs() + getDataDuration(pktData) + getSifs() 
        + getCtrlDuration(SW_PKT_TYPE_ACK) + getSlotTime();
    
    rtsHeader.setDuration(nav);
    packet->AddHeader(rtsHeader);
    
    ns3::Time ctsTimeout = getCtrlDuration(SW_PKT_TYPE_RTS) + getSifs() 
        + getCtrlDuration(SW_PKT_TYPE_CTS) + getSlotTime();
    if(sendPacket(packet, 0)) {
        updateLocalNav(ctsTimeout);
        m_cts_timeout_event = 
            ns3::Simulator::Schedule(ctsTimeout, &CsmaCaMacNetDevice::ctsTimeout, this);
    } else {
        startOver();
    }
}

void CsmaCaMacNetDevice::sendCts(ns3::Mac48Address dest, ns3::Time duration)
{
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(0);
    CsmaCaMacNetDeviceHeader ctsHeader = CsmaCaMacNetDeviceHeader(m_address, dest ,SW_PKT_TYPE_CTS);
    
    ns3::Time nav = duration - getSifs() - getCtrlDuration(SW_PKT_TYPE_CTS);
    ctsHeader.setDuration(nav);
    packet->AddHeader(ctsHeader);
    if(sendPacket(packet, 0)) {
        updateLocalNav(duration - getSifs());
    }
}

void CsmaCaMacNetDevice::sendData(void)
{
    CsmaCaMacNetDeviceHeader header;
    m_pkt_data->RemoveHeader(header);
    
    if(header.getDestinationAddress() != GetBroadcast()) {                            /* Unicast. */
        ns3::Time nav = getSifs() + getCtrlDuration(SW_PKT_TYPE_ACK);
        header.setDuration(nav);
        header.setSequence(m_sequence);
        m_pkt_data->AddHeader(header);
        if(sendPacket(m_pkt_data, 1)) {
            ns3::Time ackTimeout = getDataDuration(m_pkt_data) + getSifs() + 
                getCtrlDuration(SW_PKT_TYPE_ACK) + getSlotTime();
            updateLocalNav(ackTimeout);
            m_ack_timeout_event = 
                ns3::Simulator::Schedule(ackTimeout, &CsmaCaMacNetDevice::ackTimeout, this);
        } else {
            startOver();
        }
    } else {                                                                        /* Broadcast. */
        header.setDuration(ns3::Seconds(0));
        header.setSequence(m_sequence);
        m_pkt_data->AddHeader(header);
        if(sendPacket(m_pkt_data, 0)) {
            updateLocalNav(getDataDuration(m_pkt_data) + getSlotTime());
        } else {
            startOver();
        }
    }
}

void CsmaCaMacNetDevice::sendAck(ns3::Mac48Address dest)
{  
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(0);
    CsmaCaMacNetDeviceHeader ackHeader = CsmaCaMacNetDeviceHeader(m_address, dest, SW_PKT_TYPE_ACK);
    packet->AddHeader(ackHeader);
    
    ns3::Time nav = getCtrlDuration(SW_PKT_TYPE_ACK);
    ackHeader.setDuration(ns3::Seconds(0));
    updateLocalNav(nav + getSlotTime());
    sendPacket(packet, 0);
}

bool CsmaCaMacNetDevice::sendPacket(ns3::Ptr<ns3::Packet> packet, bool rate)
{

    if(m_state == IDLE || m_state == WAIT_TX) {
        if(m_space_device->transmitPacket(packet)) {
            m_state = TX;
            m_pkt_tx = packet;
            return true;
        } else { 
            m_state = IDLE;
        }
    }
    return false;
}

void CsmaCaMacNetDevice::startOver(void)
{
    m_queue->Enqueue(m_pkt_data);
    m_backoff_start = ns3::Seconds(0);
    m_backoff_remain = ns3::Seconds(0);
    ccaForDifs();
}

void CsmaCaMacNetDevice::sendDataDone(bool success)
{
    m_sequence++;
    m_pkt_data = 0;
    m_retry = 0;
    m_backoff_start = ns3::Seconds(0);
    m_backoff_remain = ns3::Seconds(0);
    setCw(m_cw_min);
    ccaForDifs();
}

void CsmaCaMacNetDevice::receiveRts(ns3::Ptr<ns3::Packet> packet)
{
    CsmaCaMacNetDeviceHeader header;
    packet->RemoveHeader(header);
    
    if(header.getDestinationAddress() != m_address) {
        updateNav(header.getDuration());
        m_state = IDLE;
        ccaForDifs();
        return;
    }
    
    /* If NAV indicates the channel is not busy, do not respond to RTS (802.11 std) */
    if(std::max(m_nav, m_local_nav) > ns3::Simulator::Now()) {
        return;
    }
    
    updateLocalNav(header.getDuration());
    m_state = WAIT_TX;
    m_send_cts_event = ns3::Simulator::Schedule(getSifs(), &CsmaCaMacNetDevice::sendCts, this,
        header.getSourceAddress(), header.getDuration());
}

void CsmaCaMacNetDevice::receiveCts(ns3::Ptr<ns3::Packet> packet)
{
    CsmaCaMacNetDeviceHeader header;
    packet->RemoveHeader(header);
    
    if(header.getDestinationAddress() != m_address) {
        updateNav(header.getDuration());
        m_state = IDLE;
        ccaForDifs();
        return;
    }
    
    m_retry = 0;
    updateLocalNav(header.getDuration());
    m_cts_timeout_event.Cancel();
    m_state = WAIT_TX;
    m_send_data_event = ns3::Simulator::Schedule(getSifs(), &CsmaCaMacNetDevice::sendData, this);
}

void CsmaCaMacNetDevice::receiveData(ns3::Ptr<ns3::Packet> packet)
{
    CsmaCaMacNetDeviceHeader header;
    packet->RemoveHeader(header);
    header.getDuration();
    
    if(header.getDestinationAddress() == GetBroadcast()) {
        m_state = IDLE;
        if(isNewSequence(header.getSourceAddress(), header.getSequence())) {
            m_forward_up_cllbk(packet, header.getSourceAddress(), header.getDestinationAddress());
        }
        ccaForDifs();
        return;
    }
        
    if(header.getDestinationAddress() !=  m_address) {                  /* Destination is not to me. */
        updateNav(header.getDuration());
        m_state = IDLE;
        ccaForDifs();
        return;
    }
    updateLocalNav(header.getDuration());
    m_state = WAIT_TX;
    m_send_ack_event = ns3::Simulator::Schedule(
        getSifs(), &CsmaCaMacNetDevice::sendAck, this, header.getSourceAddress());

    if(isNewSequence(header.getSourceAddress(), header.getSequence())) {    /* Forward upper layers. */
        m_forward_up_cllbk(packet, header.getSourceAddress(), header.getDestinationAddress());
    }
}

void CsmaCaMacNetDevice::receiveAck(ns3::Ptr<ns3::Packet> packet)
{
    CsmaCaMacNetDeviceHeader header;
    packet->RemoveHeader(header);
    m_state = IDLE;
    if(header.getDestinationAddress() == m_address) {
        m_ack_timeout_event.Cancel();
        sendDataDone(true);
        return;
    }
    ccaForDifs();
}

void CsmaCaMacNetDevice::receivePacket(ns3::Ptr<SpaceNetDevice> dev, ns3::Ptr<ns3::Packet> packet)
{
    channelBecomesBusy();
    switch(m_state) {
        case WAIT_TX:
        case RX:
        case WAIT_RX:
        case BACKOFF:
        case IDLE:
        m_state = RX;
        break;
        case TX:
        case COLLISION:
        break;
    }
}

void CsmaCaMacNetDevice::receivePacketDone (
    ns3::Ptr<SpaceNetDevice> dev,
    ns3::Ptr<ns3::Packet> packet,
    bool success
)
{  
    m_state = IDLE;
    CsmaCaMacNetDeviceHeader header;
    packet->PeekHeader(header);
    
    if (!success){    /* The packet is not encoded correctly. Drop it. */
        ccaForDifs();
        return;
    }
    
    switch (header.getType()) {
        case SW_PKT_TYPE_RTS:
            receiveRts (packet);
            break;
        case SW_PKT_TYPE_CTS:
            receiveCts (packet);
            break;
        case SW_PKT_TYPE_DATA:
            receiveData (packet);
            break;
        case SW_PKT_TYPE_ACK:
            receiveAck (packet);
            break;
        default:
            ccaForDifs ();
            break;
    }
}

void CsmaCaMacNetDevice::ctsTimeout(void)
{
    if(++m_retry > m_rts_retry_limit) {    /* Retransmission is over the limit. Drop packet. */
        sendDataDone(false);
        return;
    }

    m_queue->Enqueue(m_pkt_data);
    doubleCw();
    
    m_backoff_start = ns3::Seconds(0);
    m_backoff_remain = ns3::Seconds(0);
    ccaForDifs();
}

void CsmaCaMacNetDevice::ackTimeout(void)
{
    m_state = IDLE;
    if(++m_retry > m_data_retry_limit){    /* Retransmission is over the limit. Drop packet. */
        sendDataDone(false);
    } else{
        sendData();
    }
}

void CsmaCaMacNetDevice::doubleCw(void)
{
    if(m_cw * 2 > m_cw_max){
        m_cw = m_cw_max;
    } else {
        m_cw = m_cw * 2;
    }
}

ns3::Time CsmaCaMacNetDevice::roundOffTime(ns3::Time time)
{
    int64_t realTime = time.GetMicroSeconds();
    int64_t slotTime = getSlotTime().GetMicroSeconds();
    if(realTime % slotTime >= slotTime / 2) {
        return ns3::Seconds(getSlotTime().GetSeconds() *(double)(realTime / slotTime + 1));
    } else {
        return ns3::Seconds(getSlotTime().GetSeconds() *(double)(realTime / slotTime));
    }
}

bool CsmaCaMacNetDevice::isNewSequence(ns3::Mac48Address addr, uint16_t seq)
{
    std::list<std::pair<ns3::Mac48Address, uint16_t> >::iterator it = m_seq_list.begin();
    for(; it != m_seq_list.end(); ++it) {
        if(it->first == addr) {
            if(it->second == 65536 && seq < it->second) {
                it->second = seq;
                return true;
            } else if(seq > it->second) {
                it->second = seq;
                return true;
            } else {
                return false;
            }
        }
    }
    std::pair<ns3::Mac48Address, uint16_t> newEntry;
    newEntry.first = addr;
    newEntry.second = seq;
    m_seq_list.push_back(newEntry);
    return true;
}

bool CsmaCaMacNetDevice::SendFrom(ns3::Ptr<ns3::Packet> packet, const ns3::Address& source,
    const ns3::Address& dest, uint16_t protocol_num)
{
    enqueue(packet, ns3::Mac48Address::ConvertFrom(dest));
}
