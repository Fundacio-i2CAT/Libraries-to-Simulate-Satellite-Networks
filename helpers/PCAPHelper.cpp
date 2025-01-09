/***********************************************************************************************//**
 *  Class that contains the helpers to trace the communications between satellites. It allows to set
 *  the tracing and create the PCAP files to be then analyzed with tcpdump or Wireshark.
 *  @class      TraceHelpers
 *  @author     Arnau Dolz Puig (ADP), arnau.dolz@i2cat.net
 *  @date       2024-jan-29
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#include "PCAPHelper.hpp"

LOG_COMPONENT_DEFINE("pcaphelper");

PCAPHelper::PCAPHelper()
{}

void PCAPHelper::EnablePcapInternal(std::string prefix, ns3::Ptr<SpaceNetDevice> nd, bool promiscuous, bool explicitFilename)
{  
    ns3::Ptr<SpaceNetDevice> device = nd->GetObject<SpaceNetDevice> ();
    if(!device) {
        NS_LOG_INFO ("EnablePcapInternal(): Device " << device << " not of type SpaceNetDevice");
        return;
    }

    if(promiscuous) {
        NS_LOG_INFO("Promiscuous Tracing.");
    }
 
    ns3::PcapHelper pcapHelper;
 
    std::string filename;
    filename = Globals::system.tmp_path + "/";
    if(explicitFilename) {
        filename += prefix;
    } else {
        filename += pcapHelper.GetFilenameFromDevice (prefix, device);
    }
    ns3::Ptr<ns3::PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, ns3::PcapHelper::DLT_RAW);
    pcapHelper.HookDefaultSink<SpaceNetDevice> (device, "PromiscSniffer", file);
}