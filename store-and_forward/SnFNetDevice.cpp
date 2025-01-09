/***********************************************************************************************//**
 *  Class that crudely emulates Store&Forward behaviour in a node
 *  @class      SnFNetDevice
 *  @author     Oriol Fusté (OF), oriol.fuste@i2cat.net
 *  @date       2024-nov-28
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#include "SnFNetDevice.hpp"

namespace ns3 {

LOG_COMPONENT_DEFINE("SnFNetDevice");

TypeId SnFNetDevice::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SnFNetDevice")
        .SetParent<NetDevice>()
        .AddConstructor<SnFNetDevice>()
    ;
    return tid;
}

SnFNetDevice::SnFNetDevice()
    : m_node(nullptr)
    , m_ifIndex(0)
    , m_mtu(1500)
{
    // Generate a random MAC address
    m_address = Mac48Address::Allocate();
}

SnFNetDevice::~SnFNetDevice()
{
}

void SnFNetDevice::SetIfIndex(const uint32_t index)
{
    m_ifIndex = index;
}

uint32_t SnFNetDevice::GetIfIndex() const
{
    return m_ifIndex;
}

Ptr<Channel> SnFNetDevice::GetChannel() const
{
    return nullptr; // Not using a channel in this example
}

void SnFNetDevice::SetAddress(Address address)
{
    m_address = Mac48Address::ConvertFrom(address);
}

Address SnFNetDevice::GetAddress() const
{
    return m_address;
}

bool SnFNetDevice::SetMtu(const uint16_t mtu)
{
    m_mtu = mtu;
    return true;
}

uint16_t SnFNetDevice::GetMtu() const
{
    return m_mtu;
}

bool SnFNetDevice::IsLinkUp() const
{
    return true; // Always considered link up for this example
}

void SnFNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    // Not implemented for this example
}

bool SnFNetDevice::IsBroadcast() const
{
    return false;
}

Address SnFNetDevice::GetBroadcast() const
{
    return Address();
}

bool SnFNetDevice::IsMulticast() const
{
    return false;
}

Address SnFNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    return Address();
}

Address SnFNetDevice::GetMulticast(Ipv6Address addr) const
{
    return Address();
}

bool SnFNetDevice::IsBridge() const
{
    return false;
}

bool SnFNetDevice::IsPointToPoint() const
{
    return false;
}

bool SnFNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    if (!ipv4) return false;
    Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();
    Ipv4Header header;
    packet->Copy()->RemoveHeader(header);
    Socket::SocketErrno errcod;
    Ptr<SnFNetDevice> nd_out;
    Ptr<Ipv4Route> route = routingProtocol->RouteOutput(
        nullptr, 
        header, 
        nd_out,  // Explicitly pass current device as NetDevice
        errcod
    );

    // Check route availability and interface
    if (route && route->GetOutputDevice() != this)
    {
        // LOG_DBG("Stored packet will be forwaded");
        ForwardPacketUp(packet, protocolNumber);
        return true;
    }
    
    // No route or route through same interface: schedule retry
    // LOG_DBG("Storing for 60 seconds......");
    Simulator::Schedule(Seconds(60), 
        &SnFNetDevice::Send, 
        this, 
        packet, 
        dest, 
        protocolNumber
    );
    
    return true;
}

bool SnFNetDevice::SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
   Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    if (!ipv4) return false;
    Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();
    Ipv4Header header;
    packet->Copy()->RemoveHeader(header);
    Socket::SocketErrno errno_;
    Ptr<SnFNetDevice> nd_out;
    Ptr<Ipv4Route> route = routingProtocol->RouteOutput(
        nullptr, 
        header, 
        nd_out,  // Explicitly pass current device as NetDevice
        errno_
    );

    // Check route availability and interface
    if (route && nd_out != this)
    {
        ForwardPacketUp(packet, protocolNumber);
        return true;
    }
    
    // No route or route through same interface: schedule retry
    Simulator::Schedule(Seconds(60), 
        &SnFNetDevice::SendFrom, 
        this, 
        packet, 
        source, 
        dest, 
        protocolNumber
    );
    
    return true;
}

void SnFNetDevice::ForwardPacketUp(Ptr<Packet> packet, uint16_t protocol)
{
    // Forward packet up to the upper layers
    if (!m_rxCallback.IsNull())
    {
        m_rxCallback(this, packet, protocol, Address());
    }
}

Ptr<Node> SnFNetDevice::GetNode() const
{
    return m_node;
}

void SnFNetDevice::SetNode(Ptr<Node> node)
{
    m_node = node;
}

bool SnFNetDevice::NeedsArp() const
{
    return false; // No ARP needed
}

void SnFNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    m_rxCallback = cb;
}

void SnFNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
    m_promiscRxCallback = cb;
}

bool SnFNetDevice::SupportsSendFrom() const
{
    return true;
}

} // namespace ns3