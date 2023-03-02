/***********************************************************************************************//**
 *  Class that represents the packet header to be added in CsmaCaMacNetDevice layer
 *  @class      CsmaCaMacNetDeviceHeader
 *  @author     Sergi Salvà Pibernat (SSP), sergi.salva@estudiantat.upc.edu
 *  @date       2022-march-09
 *  @copyright  This code has been developed by Fundació Privada Internet i Innovació Digital a 
 *              Catalunya (i2CAT). i2CAT is a non-profit research and innovation centre that 
 *              promotes mission-driven knowledge to solve business challenges, co-create solutions
 *              with a transformative impact, empower citizens through open and participative 
 *              digital social innovation with territorial capillarity, and promote pioneering and 
 *              strategic initiatives. i2CAT *aims to transfer* research project results to private 
 *              companies in order to create social and economic impact via the out-licensing of 
 *              intellectual property and the creation of spin-offs.
 *              Find more information of i2CAT projects and IP rights at:
 *              https://i2cat.net/tech-transfer/
 **************************************************************************************************/

#ifndef __CSMACA_MAC_NET_DEVICE_HEADER_HPP__
#define __CSMACA_MAC_NET_DEVICE_HEADER_HPP__

/* Internal includes */
#include "common_networking.hpp"

/* External includes */
#include <ns3/header.h>
#include <ns3/nstime.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/buffer.h>
#include <ns3/address-utils.h>

/* Constants */
#define SW_PKT_TYPE_RTS   0
#define SW_PKT_TYPE_CTS   1
#define SW_PKT_TYPE_ACK   2
#define SW_PKT_TYPE_DATA  3

#define ADDRESS_SIZE_BYTES 6
#define PROTOCOL_NUMBER_BYTES 2

/***********************************************************************************************//**
 * Header for CsmaCaMacNetDevice packets. A Class that represents CsmaCaMacNetDevice header that 
 * shall be added into upper-layer packets.
 *
 * @see        CsmaCaMacNetDevice
 **************************************************************************************************/
class CsmaCaMacNetDeviceHeader : public ns3::Header
{
public:
    CsmaCaMacNetDeviceHeader();                                     /* Constructor */
    CsmaCaMacNetDeviceHeader(const ns3::Mac48Address source_addr, 
        const ns3::Mac48Address dest_addr, uint8_t type);           /* Constructor */
    ~CsmaCaMacNetDeviceHeader() = default;                          /* Auto-generated destructor */

    /*******************************************************************************************//**
     * Method that extracts the header. A method that is used to extract the header from the packet.
     * Inerithed from ns3::Header.
     *
     * @param      start - initial iterator
     * @see        ns3::Header
     **********************************************************************************************/
    uint32_t Deserialize(ns3::Buffer::Iterator start);

    /*******************************************************************************************//**
     * Method that gets header size. A method that retrieves header size. Inerithed from ns3::Header.
     *
     * @return     Header size
     * @see        ns3::Header
     **********************************************************************************************/
    uint32_t GetSerializedSize(void) const;

    /*******************************************************************************************//**
     * Method that inserts the header. A method that is used to insert the header into the packet.
     * Inerithed from ns3::Header.
     *
     *  @param      i - initial iterator
     *  @see        ns3::Header
     **********************************************************************************************/
    void Serialize(ns3::Buffer::Iterator start) const;

    /*******************************************************************************************//**
     * Method that privudes the InstanceType identifier. A method that provides the InstanceType
     * identifier (ns3 behaviour). Inerithed from ns3::Header.
     *
     * @return     object type ID (ns3)
     * @see        ns3::Header
     **********************************************************************************************/
    ns3::TypeId GetInstanceTypeId(void) const;

    /*******************************************************************************************//**
     * Method that prints the header. A method that prints from standard output the header content
     * Inerithed from ns3::Header.
     *
     * @param      os Standard output stream
     * @see        ns3::Header
     **********************************************************************************************/
     void Print(std::ostream &os) const;
    /*******************************************************************************************//**
     * Method that sets the source address. A method that sets the source address in the header
     *
     * @param      src - source address
     * @see        ns3::Header
     **********************************************************************************************/
    void setSourceAddress(const ns3::Address &address) { 
        m_source = ns3::Mac48Address::ConvertFrom(address);
    }

    /*******************************************************************************************//**
     * Method that sets the destination address. A method that sets the destination address in
     * the header
     *
     * @param      dst - destination address
     * @see        ns3::Header
     **********************************************************************************************/
    void setDestinationAddress(const ns3::Address &address) {
        m_destination = ns3::Mac48Address::ConvertFrom(address);
    }

    /*******************************************************************************************//**
     * Method that gets the source address. A method that gets the source address from the header
     *
     * @return     Source address
     * @see        ns3::Header
     **********************************************************************************************/
    ns3::Mac48Address getSourceAddress(void) { return m_source; }

    /*******************************************************************************************//**
     * Method that gets the destination address. A method that gets the destination address from
     * the header
     *
     * @return     Destination address
     * @see        ns3::Header
     **********************************************************************************************/
    ns3::Mac48Address getDestinationAddress(void) { return m_destination; }

    /*******************************************************************************************//**
     * It stores the protocol number in the header. This number indentifies the type of the protocol
     * which is upper the SpaceNetDevice. It is used to forward the packet when it is received.
     * Therefore before to be sent, the number shall be stored in the packet header.
     *
     * @param   prot_num    protocol number of the upper protocol
     **********************************************************************************************/
    void setProtocolNumber (uint16_t prot_num) { m_protocol_num = prot_num; }

    /*******************************************************************************************//**
     * It provides the protocol number that is stored inside the header. This number indentifies
     * the type of the protocol which is upper the SpaceNetDevice. It is used to forward the packet
     * when it is received. Therefore before to be sent, the number shall be stored in the packet
     * header.
     *
     * @return   protocol number of the upper protocol
     **********************************************************************************************/
    uint16_t getProtocolNumber (void) const { return m_protocol_num; }

    /*******************************************************************************************//**
     * Method that sets the type of packet header. A method that sets the type of packet in the 
     * header
     *
     * @param      type - type of packet head
     **********************************************************************************************/
    void setType (uint8_t type) { m_type = type; }

    /*******************************************************************************************//**
     * Method that sets de duration of the header. A method that sets the duretion of the header
     *
     * @param      duration - duration of the header
     **********************************************************************************************/
    void setDuration (ns3::Time duration);

    /*******************************************************************************************//**
     * Method that sets the sequence of the packet header. A methot that sets de sequence of the 
     * packet header
     *
     * @param      seq - sequence of the packet header
     **********************************************************************************************/
    void setSequence (uint16_t seq) { m_sequence = seq; }

    /*******************************************************************************************//**
     * Method that gets the type of packet header. A method that gets the type of packet from 
     * the header
     *
     * @return     Type of the packet header
     **********************************************************************************************/
    uint8_t getType(void) const { return m_type; }

    /*******************************************************************************************//**
     * Method that gets the duration. A method that gets the duration of the header
     *
     * @return     Destination address
     **********************************************************************************************/
    ns3::Time getDuration(void) const { return ns3::MicroSeconds (m_duration); }
  
    /*******************************************************************************************//**
     * Method that gets the size of header. A method that gets size of the header
     *
     * @return     Header length
     **********************************************************************************************/
    uint32_t getSize(void) const;
 
    /*******************************************************************************************//**
     * Method that gets the sequence. A method that gets the sequence of the header
     *
     * @return     Sequence
     **********************************************************************************************/
    uint16_t getSequence(void) const { return m_sequence; }

private:
    ns3::Mac48Address m_source;         /**< Source Address */
    ns3::Mac48Address m_destination;    /**< Destination Address */
    uint16_t m_protocol_num;            /**< Identifier of upper protocol type (e.g. IPv4 2048) */
    uint8_t m_type;                     /**< Type of packet header */   
    uint16_t m_duration;                /**< Duration of the header */
    uint16_t m_sequence;                /**< Sequence of the header */
};

#endif /* __CSMACA_MAC_NET_DEVICE_HEADER_HPP__ */