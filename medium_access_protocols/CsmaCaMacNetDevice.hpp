/***********************************************************************************************//**
 *  Class that represents the Csma CA device
 *  @class      CsmaCaNetDevice
 *  @author     Sergi Salvà Pibernat (SSP), sergi.salva@estudiantat.upc.edu
 *  @date       2022-march-2
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

#ifndef __CSMACA_MAC_NET_DEVICE_HPP__
#define __CSMACA_MAC_NET_DEVICE_HPP__

/* Global includes */
#include "dss.hpp"
/* External includes */
#include <ns3/net-device.h>
#include <ns3/callback.h>
#include <ns3/traced-callback.h>
#include <ns3/event-id.h>
#include <ns3/drop-tail-queue.h>
#include "ns3/random-variable-stream.h"

/* Internal includes */
#include "common_networking.hpp"
#include "OrbitTrajectory.hpp"
#include "SpaceNetDeviceHeader.hpp"
#include "CsmaCaMacNetDeviceHeader.hpp"
#include "SpaceNetDeviceTag.hpp"

class SpaceNetDevice;     /* Needed to create a bidirectional relationship */

/***********************************************************************************************//**
 * 
 * @see     ns3::NetDevice
 **************************************************************************************************/
class CsmaCaMacNetDevice : public ns3::NetDevice
{
public:
    /*******************************************************************************************//**
     * Constructs 
     **********************************************************************************************/
    CsmaCaMacNetDevice(void);

    /*******************************************************************************************//**
    *  Auto-generated destructor. This is made virtual to prevent derived classes to be
    *  destructed by calling this destructor through a pointer to SpaceNetDevice.
    ***********************************************************************************************/
    ~CsmaCaMacNetDevice(void);

    /*******************************************************************************************//**
     * Retrieves the object type identifier. In ns3, the classes that inhertits from ns3::ObjectBase
     * class are characterized by an object type idenfitier. This identifier enables the capability
     * to use the different ns3 tools that manage ns3::Objects, such as simple construction, global
     * access, etc. As the CsmaCaMacNetDevice inherits ns3::NetDevice (which inherits the
     * ns3::ObjectBase), it is important to determine a new and unique object type identifier
     * because it has different features than its super-class.
     *
     * @return     Object type identifier
     * @see        ns3::ObjectBase
     * 
     **********************************************************************************************/
    static ns3::TypeId getTypeId (void);

    /*******************************************************************************************//**
     * Method that defines the minimum contention window. A method that defines the minimum value 
     * for the contention window.
     * 
     * @param      cw minimum contention window
     **********************************************************************************************/
    void setCwMin(uint32_t cw) { m_cw_min = cw; }

    /*******************************************************************************************//**
     * Method that defines the slot time. A method that defines the duration of the slot time.
     * 
     * @param      duration slot time
     **********************************************************************************************/
    void setSlotTime(ns3::Time duration) { m_slot_time = duration; }

    /*******************************************************************************************//**
     * Method that retrieves contention window. A method that retrieves the contention window
     *
     * @return     Contention window
     **********************************************************************************************/
    uint32_t getCw(void) { return m_cw; }
    
    /*******************************************************************************************//**
     * Method that retrieves slot time. A method that retrieves slot time duration.
     *
     * @return     Contention window
     **********************************************************************************************/
    ns3::Time getSlotTime(void) { return m_slot_time; }

    /*******************************************************************************************//**
     * Method that defines the device. A method that defines net device.
     * 
     * @param      device
     **********************************************************************************************/
    void setDevice(ns3::Ptr<ns3::NetDevice> dev);

    /*******************************************************************************************//**
     * Methot that resets the packet transmited, the data of the packet, the queue and the 
     * sequence list.m_pktTx = 0;
     * 
     **********************************************************************************************/
    void clear(void);

    /*******************************************************************************************//**
     * Retrieves the central frequency of the device. The device transmits a packet following a data
     * rate which determines the required time to transmits a certain amount of bits. Moreover, a
     * receiver can only understand packets that have been transmitted at the same data rate,
     * because it has to bee (at bit level) synchronized. The data rate is at bps.
     *
     * @return      Data rate of the transceiver [bps]
     **********************************************************************************************/
    ns3::DataRate getDataRate(void) { return m_data_rate; }

    /*******************************************************************************************//**
     * Method that adds a packet with its destination to the queue.
     * 
     **********************************************************************************************/
    bool enqueue(ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address destination);

    /*******************************************************************************************//**
     * Method that activetes the next steps when a packet has been sent completely
     * 
     **********************************************************************************************/
    void sendPacketDone(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that when a packet is received changes the State and activates the ChannelBecomesBusy 
     * method.
     * 
     **********************************************************************************************/    
    void receivePacket (ns3::Ptr<SpaceNetDevice> dev, ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that when a packet is completly received depending on the packet header type activates
     * the corresponding receive method
     * 
     **********************************************************************************************/    
    void receivePacketDone (ns3::Ptr<SpaceNetDevice> dev, ns3::Ptr<ns3::Packet> packet, bool success);

    /*******************************************************************************************//**
     * Method that sets the ForwardUp callback. A method that sets the ForwardUp callback.
     * 
     **********************************************************************************************/
    void setForwardUpCb(ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address, ns3::Mac48Address> cb);

    /*******************************************************************************************//**
     * Provides the reference to communication channel. It is inherited from ns3::NetDevice.
     *
     * @return     Pointer to the Channel
     * @see        ns3::NetDevice
     **********************************************************************************************/
    ns3::Ptr<ns3::Channel> GetChannel(void) const override { return m_channel; }

    /*******************************************************************************************//**
     * Provides the device address. This address is, in this device case, a MAC address. It is
     * inherited from ns3::NetDevice.
     *
     * @return     Device address
     * @see        ns3::NetDevice
     **********************************************************************************************/
    ns3::Address GetAddress(void) const override { return m_address; }

    /*******************************************************************************************//**
     * Defines the reception callback. When a packet is received and after being processed at device
     * level, this callback is used to forward the packet to upper protocols. It is inherited from
     * ns3::NetDevice
     *
     * @param      Function callback
     * @see        ns3::NetDevice
     **********************************************************************************************/
    void SetReceiveCallback(ns3::NetDevice::ReceiveCallback cb) override { m_rx_cllbk = cb; }

    /*******************************************************************************************//**
     * Verifies if the device is in broadcast mode. As the SpaceNetDevice is wireless, it is always
     * broadcast in physical level. It is inherited from ns3::NetDevice
     *
     * @return     It is always true
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool IsBroadcast (void) const override { return true; }

    /*******************************************************************************************//**
     * Verifies if the device is in point-to-point mode. By concept, it is normally the opposite
     * value of IsBroadcast method. SpaceNetDevice 0is never in point-to-point mode, because by
     * nature a wireless device always physically broadcasts packets. It is inherited from
     * ns3::NetDevice
     *
     * @return     It is always false
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool IsPointToPoint(void) const override { return false; }

    /*******************************************************************************************//**
     * Verifies if the device is in multicast mode. As the SpaceNetDevice is always broadcast, a set
     * of devices can receive the packet simultaneously, thus it is always mutlicast. It is
     * inherited from ns3::NetDevice
     *
     * @return     It is always true
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool IsMulticast(void) const override { return true; }

    /*******************************************************************************************//**
     * Verifies the ARP need. A method that verifies if the device needs an Address
     * Resolution Protocol to adapt network addresses to devices one. It is always true. It is
     * inherited from ns3::NetDevice
     *
     * @return     It is always true
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool NeedsArp(void) const override { return true; }

    /*******************************************************************************************//**
     * Method that defines the device address. A method that defines the device MAC address (48 bits).
     * It is inherited from ns3::NetDevice
     *
     * @param      Device address
     * @see        ns3::NetDevice
     **********************************************************************************************/
    void SetAddress(ns3::Address address) override;

    /*******************************************************************************************//**
     * Method that retrieves broadcast address. A method that retrieves broadcast address. It is
     * inherited from ns3::NetDevice
     *
     * @return     Broadcast address
     * @see        ns3::NetDevice
     **********************************************************************************************/
    ns3::Address GetBroadcast (void) const override;

    /*******************************************************************************************//**
     * Method that retrieves mutlicast address. A method that retrieves multicast address. It is
     * inherited from ns3::NetDevice
     *
     * @return     Multicast address
     * @see        ns3::NetDevice
     **********************************************************************************************/
    ns3::Address GetMulticast(ns3::Ipv6Address addr) const override;

    /*******************************************************************************************//**
     * Method that defines the Link Layer MTU. A method that defines the Maximum Transfer Unit at
     * Link Layer. It is inherited from ns3::NetDevice
     *
     * @param      mtu MTU value
     * @return     It is always true
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool SetMtu(const uint16_t mtu) override;

   /*******************************************************************************************//**
    * Method that retrieves the Link Layer MTU. A method that retrieves the Maximum Transfer Unit
    * at Link Layer. It is inherited from ns3::NetDevice
    *
    * @return     MTU value
    * @see        ns3::NetDevice
    ***********************************************************************************************/
    uint16_t GetMtu(void) const override { return m_mtu; }

    /*******************************************************************************************//**
     * Method that verifies if the link is up. A method that verifies if the link is up. It is
     * inherited from ns3::NetDevice
     *
     * @return     The link is up (true), or not (false)
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool IsLinkUp(void) const override { return m_link_up; }

    /*******************************************************************************************//**
     * Method that verifies the device bridge. A method that verifies if the device is a bridge.
     * It is never a bridge. It is inherited from ns3::NetDevice
     *
     * @return     It is always false
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool IsBridge(void) const override { return false; }

    /*******************************************************************************************//**
     * Method that verifies if there is SendFrom method. A method that verifies if the devices
     * support the method SendFrom. SpaceNetDevice always supports this method. It is inherited
     * from ns3::NetDevice
     *
     * @return     It is always true
     * @see        ns3::NetDevice
     **********************************************************************************************/
    bool SupportsSendFrom(void) const override { return true; }

    /*******************************************************************************************//**
     * Method that retrieves the Node. A method that retrieves the Node which contains the device
     * It is inherited from ns3::NetDevice
     *
     * @return     Pointer to the ns3::Node
     * @see        ns3::NetDevice
     * @see        ns3::Node
     **********************************************************************************************/
    ns3::Ptr<ns3::Node> GetNode(void) const override { return m_node; }

    /*******************************************************************************************//**
     * Method that links the device with a Node. A method that links the device with a Node. It is
     * inherited from ns3::NetDevice
     *
     * @param      Pointer to the ns3::Node
     * @see        ns3::NetDevice
     * @see        ns3::Node
     **********************************************************************************************/
    void SetNode(ns3::Ptr<ns3::Node> node) override { m_node = node; }

    /*******************************************************************************************//**
     * Method that sets link modification function callback. A method that sets the function
     * callback for link modifications. It is inherited from ns3::NetDevice
     *
     * @param      callback Function callback
     * @see        ns3::NetDevice
     **********************************************************************************************/
    void AddLinkChangeCallback(ns3::Callback<void> callback) override;

    /*******************************************************************************************//**
     * Method that sets the promiscuous function callback. A method that sets the function callback
     * for promiscuous mode. It is inherited from ns3::NetDevice
     *
     * @param      cb Function callback
     * @see        ns3::NetDevice
     **********************************************************************************************/
    void SetPromiscReceiveCallback(ns3::NetDevice::PromiscReceiveCallback cb) override;

    /*******************************************************************************************//**
     * Method no largely used. A method that retrieves multicast address. Inherited from
     * ns3::NetDevice.
     *
     * @return     Multicast address
     **********************************************************************************************/
    ns3::Address GetMulticast(ns3::Ipv4Address multicast_group) const override;

    /*******************************************************************************************//**
     * Transmits a packet to a specific destination, and a higher protocol. In particular, this
     * method formats the packet accordingly (i.e. including the header), and physically transmits
     * it. When the packet is received, if the device is the destination, it is then forwared to the
     * upper protocol according to the identifier included in the header. Note that the device has a
     * transmission data rate, then a packet expends a certain time for its transmission. Therefore,
     * the device has an internal buffer to store the packets, that when it is filled the packets
     * starts to be discarded (congested device).
     *
     * @param      ns3 pointer to the packet that shall to be transmitted
     * @param      Destination device address
     * @param      Protocol identifier of the higher protocols
     **********************************************************************************************/
    bool Send(ns3::Ptr<ns3::Packet> packet, const ns3::Address& dest, uint16_t protocol_num) override;

    void setDataRate(ns3::DataRate data_rate){
        m_data_rate = data_rate;
    }

    void setQueue(ns3::Ptr<ns3::Queue<ns3::Packet>> queue){ m_queue = queue; }

protected:

    typedef enum {IDLE, BACKOFF, WAIT_TX, TX, WAIT_RX, RX, COLLISION } State;

    ns3::NetDevice::PromiscReceiveCallback m_rxpromisc_cllbk;       /**< Promiscuous Callback */
    ns3::NetDevice::ReceiveCallback m_rx_cllbk;                     /**< Reception Callback */
    ns3::Mac48Address m_address;                                    /**< Address */
    bool m_link_up;                                                 /**< Link status */
    ns3::Ptr<ns3::Node> m_node;                                     /**< Node container*/
    uint16_t m_mtu;                                                 /**< MTU value */
    ns3::Ptr<ns3::Channel> m_channel;                               /**< Channel to communicate */
    State m_state;                                                  /**< State of the mac device */
    ns3::Ptr<ns3::NetDevice> m_device;                              
    ns3::Ptr<SpaceNetDevice> m_space_device;
    bool m_rts_enable;                                              /**< Rts enabled */
    /* Mac parameters */
    uint16_t m_cw;                                                  /**< Contention window values */
    uint16_t m_cw_min;                                              /**< Minimum contention window value */
    uint16_t m_cw_max;                                              /**< Maximum contention window value */
    uint16_t m_rts_retry_limit;                                     /**< RTS retry limit values */
    uint16_t m_data_retry_limit;                                    /**< Data retry limit value */
    uint16_t m_retry;                                               
    uint16_t m_sequence;                                            /**< Sequence value */
    ns3::Time m_slot_time;                                          /**< Slote time */
    ns3::Time m_sifs;                                               /**< SIFS value */
    ns3::Time m_difs;                                               /**< DIFS value */
    ns3::DataRate m_data_rate;                                      /**< Transmission data rate */
    ns3::DataRate m_basic_rate;                                     /**< Transmission basic data rate */
    
    ns3::Ptr<ns3::Packet> m_pkt_tx;                                 /**< Packet trasmited */
    ns3::Ptr<ns3::Packet> m_pkt_data;                               /**< Data packet */
    ns3::Time m_nav;                                                
    ns3::Time m_local_nav;                                          
    ns3::Time m_backoff_remain;                                     /**< Remaining backoff time */
    ns3::Time m_backoff_start;                                      /**< Starting backoff value */
    
    uint32_t m_queue_limit;                                         /**< Maximim queue size */
    ns3::Ptr<ns3::Queue<ns3::Packet>> m_queue;                      /**< Packet queue */
    std::list<std::pair<ns3::Mac48Address, uint16_t> > m_seq_list;  /**< List of sequence numbers */
    ns3::Callback <void, ns3::Ptr<ns3::Packet>, 
        ns3::Mac48Address, ns3::Mac48Address> m_forward_up_cllbk;   
    ns3::TracedCallback<> m_linkchange_cllbk;                       /**< Link changes Callback */
    ns3::EventId m_cts_timeout_event;                               
    ns3::EventId m_ack_timeout_event;
    ns3::EventId m_cca_timeout_event;
    ns3::EventId m_backoff_timeout_event;
    ns3::EventId m_send_cts_event;
    ns3::EventId m_send_ack_event;
    ns3::EventId m_send_data_event;

    /*******************************************************************************************//**
     * A method that retrieves the SIFS time.
     *
     * @return      SIFS duration
     **********************************************************************************************/
    ns3::Time getSifs(void) const { return m_sifs; }

    /*******************************************************************************************//**
     * Method that retrieves DIFS. A method that retrieves the DIFS duration
     *
     * @return     SIFS duration
     **********************************************************************************************/
    ns3::Time getDifs(void) const { return m_difs; }

    /*******************************************************************************************//**
     * Method that retrieves control duration. A method that retrieves the control duration.
     *
     * @return     control duration
     **********************************************************************************************/
    ns3::Time getCtrlDuration (uint16_t type);

    /*******************************************************************************************//**
     * Method that retrieves data duration. A method that retrieves the duration of the data.
     *
     * @return     Data duration
     **********************************************************************************************/
    ns3::Time getDataDuration(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that converts a State constant into a string.
     * 
     **********************************************************************************************/
    std::string stateToString(State state);

    /*******************************************************************************************//**
     * Method that defines the contention window. A method that defines the contention window.
     * 
     * @param      cw contention window
     **********************************************************************************************/
    void setCw(uint32_t cw) { m_cw = cw; }

    /*******************************************************************************************//**
     * Method that calculates the Clear Channel Assesment for DIFS
     * 
     **********************************************************************************************/
    void ccaForDifs(void);

    /*******************************************************************************************//**
     * Method that stars a backoff timmer.
     * 
     **********************************************************************************************/
    void backoffStart(void);

    /*******************************************************************************************//**
     * Method that canges the CsmaCaMacNetDevice states when the channel becomes busy
     * 
     **********************************************************************************************/
    void channelBecomesBusy(void);

    /*******************************************************************************************//**
     * Method that once access is granted to the channel iniciates the process of transmition.
     * 
     **********************************************************************************************/
    void channelAccessGranted(void);

    /*******************************************************************************************//**
     * Method that updates the Network Allocation Vector.
     * 
     **********************************************************************************************/
    void updateNav(ns3::Time nav);

    /*******************************************************************************************//**
     * Method that updates the local Network Allocation Vector.
     * 
     **********************************************************************************************/
    void updateLocalNav(ns3::Time nav);

    /*******************************************************************************************//**
     * Method that does all the steps previuos of sending an RTS packet.
     * 
     **********************************************************************************************/
    void sendRts(ns3::Ptr<ns3::Packet> pktData);

    /*******************************************************************************************//**
     * Method that does all the steps previuos of sending an CTS packet.
     * 
     **********************************************************************************************/
    void sendCts(ns3::Mac48Address dest, ns3::Time duration);

    /*******************************************************************************************//**
     * Method that prepares for transmission of a data packet.
     * 
     **********************************************************************************************/
    void sendData(void);

    /*******************************************************************************************//**
     * Method that prepares for transmission of an ACK packet.
     * 
     **********************************************************************************************/
    void sendAck(ns3::Mac48Address dest);

    /*******************************************************************************************//**
     * Method that transmits a packet.
     * 
     **********************************************************************************************/
    bool sendPacket(ns3::Ptr<ns3::Packet> packet, bool rate);

    /*******************************************************************************************//**
     * Methot that resets the backoff timer.
     * 
     **********************************************************************************************/
    void startOver(void);

    /*******************************************************************************************//**
     * 
     * 
     **********************************************************************************************/
    void sendDataDone(bool success);

    /*******************************************************************************************//**
     * Method that processes a received RTS packet
     * 
     **********************************************************************************************/
    void receiveRts(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that processes a received CTS packet
     * 
     **********************************************************************************************/
    void receiveCts(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that processes a received data packet
     * 
     **********************************************************************************************/
    void receiveData(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Method that processes a received ACK packet
     * 
     **********************************************************************************************/
    void receiveAck(ns3::Ptr<ns3::Packet> packet);

    /*******************************************************************************************//**
     * Methot that restarts all the corresponding timers when a CTS timeout occurs.
     * 
     **********************************************************************************************/
    void ctsTimeout(void);

    /*******************************************************************************************//**
     * Methot that ressends the packet related to the ACK.
     * 
     **********************************************************************************************/
    void ackTimeout(void);

    /*******************************************************************************************//**
     * Method that doubles the existing contention window.
     * 
     **********************************************************************************************/
    void doubleCw(void);

    /*******************************************************************************************//**
     * Method that rounds off any given time.
     * 
     **********************************************************************************************/
    ns3::Time roundOffTime(ns3::Time time);

    /*******************************************************************************************//**
     * Checks if the given sequence is new to this mac net device.
     * 
     **********************************************************************************************/
    bool isNewSequence(ns3::Mac48Address addr, uint16_t seq);

    /*******************************************************************************************//**
     * Transmits a packet to medium indicating the source and the destination, as well as the higher
     * protocol identifier. This method can be used if multiple link addresses are used for the same
     * device. However, in the SpaceNetDevice case this is not possible. Therefore, this method
     * cannot be publicly accessed.
     *
     * @param      ns3 pointer to the packet that shall to be transmitted
     * @param      Source device address
     * @param      Destination device address
     * @param      Protocol identifier of the higher protocols
     **********************************************************************************************/
    bool SendFrom(ns3::Ptr<ns3::Packet> packet, const ns3::Address& source,
        const ns3::Address& dest, uint16_t protocol_num) override;

    /*******************************************************************************************//**
     * Method no largely used. A method that sets the IF index. It is no largely used. Inherited
     * from ns3::NetDevice.
     *
     * @param      index IF Index
     **********************************************************************************************/
    void SetIfIndex(const uint32_t /* index */) override { /* Not used */ }

    /*******************************************************************************************//**
     * Method no largely used. A method that retrieves the IF index. It is no largely used.
     * Inherited from ns3::NetDevice.
     *
     * @return      Index
     **********************************************************************************************/
    uint32_t GetIfIndex(void) const override { return 0; }

};

#endif /* __CSMACA_MAC_NET_DEVICE_HPP__ */
