/***********************************************************************************************//**
 *  Class that contains the helpers to trace the communications between satellites. It allows to set
 *  the tracing and create the PCAP files to be then analyzed with tcpdump or Wireshark.
 *  @class      PCAPHelper
 *  @author     Arnau Dolz Puig (ADP), arnau.dolz@i2cat.net
 *  @date       2024-jan-29
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef __TRACEHELPER_HPP__
#define __TRACEHELPER_HPP__

/* Global includes */
#include "dss.hpp"

/* External libraries */
#include <ns3/trace-helper.h>

/* Internal libraries */
#include "SpaceNetDevice.hpp"

/***********************************************************************************************//**
 *  Helpers are useful to allow the tracing of the communications. To generate PCAP files NS3 provide
 *  the trace-helper that allows to generate ASCII records or PCAP files. However the netdevice used
 *  does not match the SpaceNetDevice so the correct call must be done in here.
 **************************************************************************************************/
class PCAPHelper
{
public:
    /******************************************************************************************//**
     * Constructor.
     *********************************************************************************************/
    PCAPHelper();
    
    /******************************************************************************************//**
     * Auto-generated destructor. This is made virtual to prevent derived classes to be
     *  destructed by calling this destructor through a pointer to Clouds.
     *********************************************************************************************/
    ~PCAPHelper(void) = default;
    /************************************************************************************************************//**
     * Function that traces the packets received on an SpaceNetDevice. When the function is called, it creates a 
     * new file named with the "prefix". If the expliciFileName is true, then the name will only be the prefix.
     * Otherwise, it will start generating files according to the following format: prefix-x-y.pcap. Where x and
     * y corresponds to the SpaceNetDevices that are connected. So for each different message received from a
     * different SpaceNetDevice the X and y will change its vale starting from 0 and going forward. By using the
     * ns3::PCAPHelper, the packets are written into the file by means of the HookDefaulSink. This function
     * traces all the packets if the promiscous variable is set. If not, it tracks nothing.  
     *
     * @param      prefix name of the PCAP file
     * @param      nd pointer to the spaceNetDevice to trace packets from
     * @param      promiscous variable to set the tracing
     * @param      explicitFilename variable to set only the name but not the nodes.
     ***********************************************************************************************************/
    void EnablePcapInternal(std::string prefix, ns3::Ptr<SpaceNetDevice> nd, bool promiscuous, bool explicitFilename);
};

#endif /* __TRACEHELPER_HPP__ */
