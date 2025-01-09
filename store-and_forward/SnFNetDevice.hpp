/***********************************************************************************************//**
 *  Class that crudely emulates Store&Forward behaviour in a node
 *  @class      SnFNetDevice
 *  @author     Oriol Fusté (OF), oriol.fuste@i2cat.net
 *  @date       2024-nov-28
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef __SNF_NET_DEVICE_HPP__
#define __SNF_NET_DEVICE_HPP__

/* Global includes */
#include "dss.hpp"
/* External includes */
#include <ns3/net-device.h>
#include <ns3/net-device-container.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/event-id.h>
#include <ns3/simulator.h>
#include <ns3/channel.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/ipv4-routing-protocol.h>
#include <ns3/ipv4-list-routing.h>
#include <ns3/ipv4-static-routing.h>


/* Internal includes */
#include "common_networking.hpp"

namespace ns3 {

/***********************************************************************************************//**
 * Device for Store&Forward behaviour emulation. It represents a generic device where packets may
 * be routed to in order to store them when a route to its destination is not available.
 * It does not require a channel and it works by scheduling a check for each stored pacekt every 60
 * seconds. The check looks at the routing table and if a route to the packet destiny is now
 * available, the packet is forwarded back to the IP layer.
 * The implementation is rudimentary and resource inneficient, intended to be used only for
 * demonstration purposes.
 *
 * @see     ns3::NetDevice
 **************************************************************************************************/
class SnFNetDevice : public NetDevice
{
public:
    // Type ID for ns-3 object system
    static TypeId GetTypeId();

    // Constructor
    SnFNetDevice();

    // Destructor
    virtual ~SnFNetDevice() override;

    // Inherited NetDevice interface methods
    virtual void SetIfIndex(const uint32_t index) override;
    virtual uint32_t GetIfIndex() const override;
    virtual Ptr<Channel> GetChannel() const override;
    virtual void SetAddress(Address address) override;
    virtual Address GetAddress() const override;
    virtual bool SetMtu(const uint16_t mtu) override;
    virtual uint16_t GetMtu() const override;
    virtual bool IsLinkUp() const override;
    virtual void AddLinkChangeCallback(Callback<void> callback) override;
    virtual bool IsBroadcast() const override;
    virtual Address GetBroadcast() const override;
    virtual bool IsMulticast() const override;
    virtual Address GetMulticast(Ipv4Address multicastGroup) const override;
    virtual Address GetMulticast(Ipv6Address addr) const override;
    virtual bool IsBridge() const override;
    virtual bool IsPointToPoint() const override;
    virtual Ptr<Node> GetNode() const override;
    virtual void SetNode(Ptr<Node> node) override;
    virtual bool NeedsArp() const override;
    virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
    virtual void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
    virtual bool SupportsSendFrom() const override;
    
    /*******************************************************************************************//**
     * Delivers a packet to the SnFNetDevice for storage. This function will be called periodically
     * to check if a new route to the packet destiny has appeared. When this happens, the packet is
     * returned to the IP layer as if it was just received from a channel. 
     *
     * @param      packet ns3 pointer to the packet that shall to be transmitted
     * @param      dest Destination device address
     * @param      protocolNumber Protocol identifier of the higher protocols
     **********************************************************************************************/
    virtual bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    /*******************************************************************************************//**
     * Delivers a packet to the SnFNetDevice for storage. This function will be called periodically
     * to check if a new route to the packet destiny has appeared. When this happens, the packet is
     * returned to the IP layer as if it was just received from a channel. 
     *
     * @param      packet ns3 pointer to the packet that shall to be transmitted
     * @param      source Source device address
     * @param      dest Destination device address
     * @param      protocolNumber Protocol identifier of the higher protocols
     **********************************************************************************************/
    virtual bool SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber) override;

private:
    // Private methods
    /*******************************************************************************************//**
     * Forwards the packet upwards to the IP layer for re-routing.
     *
     * @param      packet ns3 pointer to the packet that shall be fowarded
     * @param      protocol Protocol number given from the upper layers. IP for this case.
     **********************************************************************************************/
    void ForwardPacketUp(Ptr<Packet> packet, uint16_t protocol);

    // Member variables
    Ptr<Node> m_node;
    uint32_t m_ifIndex;
    NetDevice::ReceiveCallback m_rxCallback;
    NetDevice::PromiscReceiveCallback m_promiscRxCallback;
    uint16_t m_mtu;
    Mac48Address m_address;
};

} // namespace ns3

#endif /* __SNF_NET_DEVICE_HPP__ */