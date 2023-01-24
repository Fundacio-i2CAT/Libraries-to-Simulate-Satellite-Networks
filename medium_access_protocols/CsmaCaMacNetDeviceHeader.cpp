/***********************************************************************************************//**
 *  Class that represents the packet header to be added in CsmaCaMacNetDevice layer
 *  @class      CsmaCaMacNetDeviceHeader
 *  @author     Sergi Salvà Pibernat (SSP), sergi.salva@estudiantat.upc.edu
 *  @date       2022-march-09
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#include "CsmaCaMacNetDeviceHeader.hpp"

LOG_COMPONENT_DEFINE("CsmaCaMacNetDeviceHeader");

CsmaCaMacNetDeviceHeader::CsmaCaMacNetDeviceHeader()
    : ns3::Header()
    , m_source("00:00:00:00:00:00")
    , m_destination("00:00:00:00:00:00")
    , m_protocol_num(0)
{

}

CsmaCaMacNetDeviceHeader::CsmaCaMacNetDeviceHeader(const ns3::Mac48Address source_addr,
    const ns3::Mac48Address dest_addr, 
    uint8_t type)
    : ns3::Header()
    , m_source(source_addr)
    , m_destination(dest_addr)
    , m_protocol_num(0)
    , m_type(type) 
{

}

ns3::TypeId CsmaCaMacNetDeviceHeader::GetInstanceTypeId(void) const
{
    return ns3::Header::GetTypeId();
}

uint32_t CsmaCaMacNetDeviceHeader::Deserialize(ns3::Buffer::Iterator start)
{
    ns3::Buffer::Iterator start_it = start;
    m_type = start_it.ReadU8 ();
    m_duration = start_it.ReadLsbtohU16 ();
    switch(m_type) {
        case SW_PKT_TYPE_RTS:
        case SW_PKT_TYPE_CTS:
        case SW_PKT_TYPE_ACK:
            ns3::ReadFrom (start_it, m_source);
            ns3::ReadFrom (start_it, m_destination);
            m_protocol_num = start_it.ReadU16();
        break;
        case SW_PKT_TYPE_DATA:
            ns3::ReadFrom (start_it, m_source);
            ns3::ReadFrom (start_it, m_destination);
            m_protocol_num = start_it.ReadU16();
            m_sequence = start_it.ReadU16 ();
        break;
    }

    return start_it.GetDistanceFrom(start);
}

uint32_t CsmaCaMacNetDeviceHeader::GetSerializedSize(void) const
{
    return getSize();
}

void CsmaCaMacNetDeviceHeader::Serialize(ns3::Buffer::Iterator start) const
{
    ns3::Buffer::Iterator start_it = start;
    start_it.WriteU8 (m_type);
    start_it.WriteHtolsbU16 (m_duration);
    switch(m_type) {
        case SW_PKT_TYPE_RTS:
        case SW_PKT_TYPE_CTS:
        case SW_PKT_TYPE_ACK:
            ns3::WriteTo(start_it, m_source);           /* First the source address */
            ns3::WriteTo(start_it, m_destination);      /* Second the destination address */
            start_it.WriteU16(m_protocol_num);          /* Third the protocol number */
            break;
        case SW_PKT_TYPE_DATA:
            ns3::WriteTo(start_it, m_source);           /* First the source address */
            ns3::WriteTo(start_it, m_destination);      /* Second the destination address */
            start_it.WriteU16(m_protocol_num);          /* Third the protocol number */
            start_it.WriteU16 (m_sequence);             /* Fourth the packet sequence */
            break;
    }
}

void CsmaCaMacNetDeviceHeader::Print(std::ostream &os) const 
{   
    os << "Source= " << m_source << ", Destination= " << m_destination 
       << ", Protocol Number= " << m_protocol_num << " type=" << (uint32_t) m_type 
       << ", Sequence= " << m_sequence;  
}

void CsmaCaMacNetDeviceHeader::setDuration (ns3::Time duration)
{
    int64_t duration_us = duration.GetMicroSeconds ();
    m_duration = static_cast<uint16_t> (duration_us);
}

uint32_t CsmaCaMacNetDeviceHeader::getSize () const
{
    uint32_t size = 0;
    switch(m_type) {
        case SW_PKT_TYPE_RTS:
        case SW_PKT_TYPE_CTS:
        case SW_PKT_TYPE_ACK:
            size = sizeof(m_type) + sizeof(m_duration) + ADDRESS_SIZE_BYTES * 2 
                 + PROTOCOL_NUMBER_BYTES;
        break;
        case SW_PKT_TYPE_DATA:
            size = sizeof(m_type) + sizeof(m_duration) + ADDRESS_SIZE_BYTES * 2 
                 + PROTOCOL_NUMBER_BYTES + sizeof(m_sequence);
        break;
    }
    return size;
}