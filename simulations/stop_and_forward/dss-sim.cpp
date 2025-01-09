/************************************************************************************************************//**
 *  Example scenario demonstrating how the SnFNetDevice works.
 *  @author     Oriol Fusté (OF), coriol.fuste@i2cat.net
 *  @date       2024-dec-03
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 ***************************************************************************************************************/

/* INCLUDE SECTION *********************************************************************************************/
/* Global libraries */
#include "dss.hpp"

/* External libraries */
#include <ns3/node.h>
#include <ns3/net-device-container.h>
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-routing-table-entry.h"
#include <ns3/internet-stack-helper.h>
#include <ns3/udp-client-server-helper.h>
#include "ns3/udp-socket-factory.h"
#include "ns3/socket.h"
#include "ns3/arp-cache.h"

/* Internal libraries */
#include "Globals.hpp"
#include "Database.hpp"
#include "ProcessManager.hpp"
#include "Viewer.hpp"
#include "Satellite.hpp"
#include "GroundStation.hpp"
#include "GroundMobility.hpp"
#include "SpaceNetDevice.hpp"
#include "SnFNetDevice.hpp"
#include "KeplerOrbitTrajectory.hpp"
#include "SpaceChannel.hpp"
#include "PCAPHelper.hpp"
#include "TLE.hpp"

using namespace ns3;

LOG_COMPONENT_DEFINE("---");

/*  Route management functions  */
void RemoveRoute(Ptr<Ipv4StaticRouting> ipv4StaticRouting,
                 Ipv4Address network,
                 Ipv4Mask mask) {
    uint32_t numRoutes = ipv4StaticRouting->GetNRoutes();
    for(uint32_t i = 0; i < numRoutes; i++) {
        Ipv4RoutingTableEntry route = ipv4StaticRouting->GetRoute(i);
        if (route.GetDestNetwork() == network && route.GetDestNetworkMask() == mask) {
            ipv4StaticRouting->RemoveRoute(i);
            // std::cout << "Route to " << network << " removed at " << Simulator::Now().GetSeconds() << " seconds\n";
            break;
        }
    }
}

void AddRoute(Ptr<Ipv4StaticRouting> ipv4StaticRouting, 
              Ipv4Address network, 
              Ipv4Address nextHop, 
              uint32_t interface) {
    // Remove any existing route to the same network
    Ipv4Mask mask("255.255.255.255"); // Use the specific mask if applicable
    RemoveRoute(ipv4StaticRouting, network, mask);

    // Add the new route
    ipv4StaticRouting->AddHostRouteTo(network, nextHop, interface);
}

void PrintRoutes(Ptr<Ipv4StaticRouting> ipv4StaticRouting, std::string name = "NOT SPECIFIED"){
    uint32_t numRoutes = ipv4StaticRouting->GetNRoutes();
    std::cout << "--> Routes for " << name << " at second " << Simulator::Now().GetSeconds() << "\n";
    for(uint32_t i = 0; i < numRoutes; i++){
        Ipv4RoutingTableEntry entry = ipv4StaticRouting->GetRoute(i);
        std::cout << "Destination Addr. : " << entry.GetDestNetwork() << "\n";
        std::cout << "Destination Mask : " << entry.GetDestNetworkMask() << "\n";
        std::cout << "Gateway : " << entry.GetGateway() << "\n";
        std::cout << "Interface : " << entry.GetInterface() << "\n";
        std::cout << "****************\n";
    }     
    std::cout << "-------------------------------------------------\n";   
}


int main(int argc, char** argv)
{
    LOG_INFO("DSS Simulator running...");

    /* Execution variables: --------------------------------------------------------------------- */
    bool viewer = false;
    bool processing = false;
    double t_start = 0;
    double t_end = 86400;
    bool tracing = true;

    // LogComponentEnable("UdpClient", LOG_LEVEL_LOGIC);
    // LogComponentEnable("UdpServer", LOG_LEVEL_LOGIC);
    // LogComponentEnable("UdpServer", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4Address", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4Interface", LOG_LEVEL_DEBUG);
    // LogComponentEnable("Ipv4InterfaceAddress", LOG_LEVEL_DEBUG);
    // LogComponentEnable("ArpL3Protocol", LOG_LEVEL_DEBUG);

    /* Settings (TODO): this is just a temporary version ---------------------------------------- */
    srand(time(nullptr));       /* Seeds the random number generation.                            */
    Globals::init();

    /* Initialize process, engine and environment: ---------------------------------------------- */
    ns3::Time::SetResolution(ns3::Time::NS);    /* Defines NS-3 resolution.                       */

    /* Simulation parameters */
    Globals::user.simulation_start_epoch = TimeUtils::convertToUNIX("2024-12-02 00:00:00");
    Globals::user.simulation_start_epoch.tv_sec += t_start;
    Globals::user.simulation_duration = ns3::Seconds(t_end - t_start);

    /* Create channel, nodes and devices-----------------------------------------------------------------*/
    SpaceChannel* channel = new SpaceChannel();

    NetDeviceContainer devices;
    std::string address_str;

    // For UE
    ns3::Ptr<ns3::Node> node1 = ns3::CreateObject<ns3::Node>();
    std::string ue_str_id = "IoT User Equipment";
    ns3::Ptr<SpaceNetDevice> dev_ue = ns3::CreateObject<SpaceNetDevice>(ue_str_id, 100e2, 50, 435e3, -55);
    address_str = "00:00:00:00:00:10";
    dev_ue->SetAddress(ns3::Mac48Address(address_str.c_str()));
    dev_ue->SetMtu(15000);
    dev_ue->setQueueSize(1);
    dev_ue->SetNode(node1);
    node1->AddDevice(dev_ue);
    devices.Add(dev_ue);

    channel->addDevice(dev_ue);
    Globals::user.addGroundStationId(ue_str_id);

    // For SAT
    ns3::Ptr<ns3::Node> node2 = ns3::CreateObject<ns3::Node>();
    std::string sat_str_id = "Sentinel-2A";
    ns3::Ptr<SpaceNetDevice> dev_sat = ns3::CreateObject<SpaceNetDevice>(sat_str_id, 100e2, 50, 435e3, -55);
    address_str = "00:00:00:00:00:20";
    dev_sat->SetAddress(ns3::Mac48Address(address_str.c_str()));
    dev_sat->SetMtu(15000);
    dev_sat->SetNode(node2);
    dev_sat->setQueueSize(1000);
    node2->AddDevice(dev_sat);
    devices.Add(dev_sat);

    channel->addDevice(dev_sat);
    Globals::user.addSatelliteId(sat_str_id);

    // SnFNetDevice for SAT
    Ptr<SnFNetDevice> snfDevice = CreateObject<SnFNetDevice>();
    node2->AddDevice(snfDevice);

    // For GS
    ns3::Ptr<ns3::Node> node3 = ns3::CreateObject<ns3::Node>();
    std::string gs_str_id = "NTN Gateway";
    ns3::Ptr<SpaceNetDevice> dev_gs = ns3::CreateObject<SpaceNetDevice>(gs_str_id, 100e2, 50, 435e3, -55);
    address_str = "00:00:00:00:00:30";
    dev_gs->SetAddress(ns3::Mac48Address(address_str.c_str()));
    dev_gs->SetMtu(15000);
    dev_gs->setQueueSize(1);
    dev_gs->SetNode(node3);
    node3->AddDevice(dev_gs);
    devices.Add(dev_gs);

    channel->addDevice(dev_gs);
    Globals::user.addGroundStationId(gs_str_id);

    /* IoT User Equipment-------------------------------------------------------------------------*/
    ns3::Ptr<GroundStation> ue = ns3::CreateObject<GroundStation, std::string>(std::string(ue_str_id));
    std::shared_ptr<GroundMobility> ue_trajectory = std::make_shared<GroundMobility>
        (GeographicCoordinates(-62.663138,-60.387992, 50.0 + 6.378e6), ue_str_id); // Base Antártida Juan Carlos I
    ue->setGroundMobility(ue_trajectory);
    node1->AggregateObject(ue);
    dev_ue->setMobilityModel(ue_trajectory);

    /* Satellite----------------------------------------------------------------------------------*/
    ns3::Ptr<Satellite> sat = ns3::CreateObject<Satellite, std::string>(std::string(sat_str_id));
    TLE tle(sat_str_id,"1 40697U 15028A   24337.14457616  .00000519  00000-0  21480-3 0  9998",
                    "2 40697  98.5704  48.9920 0001011  93.0957 267.0342 14.30811188493363");
    std::shared_ptr<OrbitTrajectory> orbit(new KeplerOrbitTrajectory(tle, sat_str_id));
    sat->setOrbitTrajectory(orbit);
    node2->AggregateObject(sat);
    dev_sat->setMobilityModel(orbit);

    /* Ground Station-----------------------------------------------------------------------------*/
    ns3::Ptr<GroundStation> gs = ns3::CreateObject<GroundStation, std::string>(std::string(gs_str_id));
    std::shared_ptr<GroundMobility> gs_trajectory = std::make_shared<GroundMobility>
        (GeographicCoordinates(78.228156, 15.4014289, 458.0 + 6.378e6), gs_str_id); // Svalbard
    gs->setGroundMobility(gs_trajectory);
    node3->AggregateObject(gs);
    dev_gs->setMobilityModel(gs_trajectory);

    /* Set up internet helper---------------------------------------------------------------------*/
    // Container
    NodeContainer nodes(node1, node2, node3);
    InternetStackHelper inet_help;
    inet_help.SetIpv4StackInstall(true);
    inet_help.SetIpv6StackInstall(false);
    inet_help.SetRoutingHelper(Ipv4StaticRoutingHelper());
    inet_help.Install(nodes);

    Ipv4AddressHelper ipv4_addr;
    // UE: 192.168.1.1        SAT: 192.168.1.2        GS: 192.168.1.3
    ipv4_addr.SetBase(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer ic = ipv4_addr.Assign(devices);

    // For SAT, add one more interface for SnF
    Ptr<Ipv4> ipv4 = node2->GetObject<Ipv4>();
    uint32_t snf_interface = ipv4->AddInterface(snfDevice);
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress(Ipv4Address("10.0.0.1"), Ipv4Mask("255.255.255.0"));
    ipv4->AddAddress(snf_interface, ipv4Addr);
    ipv4->SetUp(snf_interface);

    /* Applications part--------------------------------------------------------------------------*/
    // UDP server at GS (NTN Gateway)
    UdpServerHelper udpHelper(5555);
    udpHelper.SetAttribute("StopTime", TimeValue(Globals::user.simulation_duration));
    Ptr<UdpServer> udpServer = DynamicCast<UdpServer>(udpHelper.Install(node3).Get(0));
    
    // UDP Client at UE
    UdpClientHelper udpClientHelper(ic.GetAddress(2), 5555);
    udpClientHelper.SetAttribute("Interval", ns3::TimeValue(Seconds(100.0)));
    udpClientHelper.SetAttribute("PacketSize", ns3::UintegerValue(1024));
    udpClientHelper.SetAttribute("MaxPackets", ns3::UintegerValue(1000000));
    udpClientHelper.SetAttribute("StopTime", TimeValue(Globals::user.simulation_duration));
    Ptr<UdpClient> udpClient = DynamicCast<UdpClient>(udpClientHelper.Install(node1).Get(0));

    /* Routing part-------------------------------------------------------------------------------*/
    Ipv4StaticRoutingHelper router;
    Ptr<Ipv4StaticRouting> routingUE = router.GetStaticRouting(node1->GetObject<Ipv4>());
    Ptr<Ipv4StaticRouting> routingSAT = router.GetStaticRouting(node2->GetObject<Ipv4>());
    Ptr<Ipv4StaticRouting> routingGS = router.GetStaticRouting(node3->GetObject<Ipv4>());
    // Signal iface down, otherwise it will try to send through the SND
    routingUE->NotifyInterfaceDown(std::get<1>(ic.Get(0)));
    routingSAT->NotifyInterfaceDown(std::get<1>(ic.Get(1)));
    routingGS->NotifyInterfaceDown(std::get<1>(ic.Get(2)));

    /* ARP part-----------------------------------------------------------------------------------*/
    Ptr<ArpCache> arp_cache;
    ArpCache::Entry* entry;
    ObjectVectorValue interfaces;
        /* Install the arp cache in the UE node */
    arp_cache = CreateObject<ArpCache>();
    arp_cache->SetAliveTimeout(Seconds(3600*24*365));
    entry = arp_cache->Add(Ipv4Address("192.168.1.2"));
    entry->SetMacAddress (Mac48Address("00:00:00:00:00:20")); // Entry for SAT
    entry->MarkPermanent();
    node1->GetObject<Ipv4L3Protocol> ()->GetAttribute("InterfaceList", interfaces);
    interfaces.Get(1)->GetObject<Ipv4Interface>()->SetAttribute("ArpCache", PointerValue(arp_cache));
        /* Install the arp cache in the SAT node */
    arp_cache = CreateObject<ArpCache>();
    arp_cache->SetAliveTimeout(Seconds(3600*24*365));
    entry = arp_cache->Add(Ipv4Address("192.168.1.1"));
    entry->SetMacAddress (Mac48Address("00:00:00:00:00:10")); // Entry for UE
    entry->MarkPermanent();
    arp_cache->SetAliveTimeout(Seconds(3600*24*365));
    entry = arp_cache->Add(Ipv4Address("192.168.1.3"));
    entry->SetMacAddress (Mac48Address("00:00:00:00:00:30")); // Entry for GS
    entry->MarkPermanent();
    node2->GetObject<Ipv4L3Protocol> ()->GetAttribute("InterfaceList", interfaces);
    interfaces.Get(1)->GetObject<Ipv4Interface>()->SetAttribute("ArpCache", PointerValue(arp_cache));
        /* Install the arp cache in the GS node */
    arp_cache = CreateObject<ArpCache>();
    arp_cache->SetAliveTimeout(Seconds(3600*24*365));
    entry = arp_cache->Add(Ipv4Address("192.168.1.2"));
    entry->SetMacAddress (Mac48Address("00:00:00:00:00:20")); // Entry for SAT
    entry->MarkPermanent();
    node3->GetObject<Ipv4L3Protocol> ()->GetAttribute("InterfaceList", interfaces);
    interfaces.Get(1)->GetObject<Ipv4Interface>()->SetAttribute("ArpCache", PointerValue(arp_cache));

    // For the UE to SAT visibility
    Simulator::Schedule(Seconds(4201), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(5751), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(10201), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(11801), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(16201), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(17751), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(22151), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(23651), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(28001), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(29501), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(33851), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(35351), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(39701), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(41251), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(45601), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(46951), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(51551), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(53151), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(57651), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(59201), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(63901), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(65301), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(70201), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(71451), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(76501), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(77751), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(82651), &AddRoute, routingUE, ic.GetAddress(2), ic.GetAddress(1), std::get<1>(ic.Get(0)));
    Simulator::Schedule(Seconds(84051), &Ipv4StaticRouting::NotifyInterfaceDown, routingUE, std::get<1>(ic.Get(0)));


    // For SAT to GS visibility or SnF
    Simulator::Schedule(Seconds(0), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(550), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(850), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(1850), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(2150), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(6700), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(7000), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(7950), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(8250), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(12850), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(13150), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(14050), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(14350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(18950), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(19250), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(20200), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(20500), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(25050), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(25300), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(26350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(26600), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(31100), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(31350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(32450), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(32700), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(37150), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(37350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(38500), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(38750), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(43150), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(43350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(44500), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(44700), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(49100), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(49350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(50450), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(50700), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(55050), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(55300), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(56400), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(56650), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(61000), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(61250), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(62350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(62600), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(67000), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(67200), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(68350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(68550), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(73000), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(73200), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(74350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(74550), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(79000), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(79250), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(80350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(80600), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);
    Simulator::Schedule(Seconds(85100), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), std::get<1>(ic.Get(1)));
    Simulator::Schedule(Seconds(85350), &AddRoute, routingSAT, ic.GetAddress(2), ic.GetAddress(2), snf_interface);


    /* Initialize tracing: ---------------------------------------------------------------------- */
    // Send dummy packet at t=0
    std::string payload = "DUMMY PACKET FOR TRACE TIMESTAMPING PURPOSES";
    Ptr<Packet> pkt = Create<Packet>((uint8_t *)payload.c_str(), payload.size());
    SpaceNetDeviceHeader header;
    pkt->AddHeader(header);
    Simulator::Schedule(Seconds(0), &SpaceNetDevice::receive, dev_ue, pkt->Copy(), double(1000));
    Simulator::Schedule(Seconds(0), &SpaceNetDevice::receive, dev_sat, pkt->Copy(), double(1000));
    Simulator::Schedule(Seconds(0), &SpaceNetDevice::receive, dev_gs, pkt->Copy(), double(1000));
    if(tracing == true) {
        LOG_INFO("Tracing set");
        /* To create the files and enable the tracing we shall use this function: 
         * EnablePcapInternal(). See an example in dev/prototype-pcap-files where a simulation  
         * is available with the tracing options.
         */
        PCAPHelper* helper = new PCAPHelper();
        helper->EnablePcapInternal("UE_caps", dev_ue, true, true);
        helper->EnablePcapInternal("SAT_caps", dev_sat, true, true);
        helper->EnablePcapInternal("GS_caps", dev_gs, true, true);
    }

    /* Launch framework: ------------------------------------------------------------------------ */
    ns3::Simulator::Stop(Globals::user.simulation_duration);
    ns3::Simulator::Run();

    double pkts_sent = udpClient->GetTotalTx() / 1024;
    double pkts_received = udpServer->GetReceived();

    std::cout << "*********************************************\n";
    std::cout << "SENT: " << pkts_sent << "\tRECEIVED: " << pkts_received << "\n";
    std::cout << "*********************************************\n";

    ns3::Simulator::Destroy();

    /* Flush any possible data that remains unstored -------------------------------------------- */
    Database::get().flush();

    /* Launch Processing Units: --------------------------------------------------------------------------- */
    if(processing == true) {
        LOG_INFO("Executing Processing Units");
        ProcessManager processManager;
        processManager.run();
    }

    /* Launch Viewer: --------------------------------------------------------------------------- */
    if(viewer == true) {
        LOG_INFO("Executing Viewer");
        Viewer viewer;
        viewer.run();
    }

    exit(EXIT_SUCCESS);
}
