/***************************************************************************************************
*  File:        dss-sim.cpp                                                                        *
*  Authors:     Arnau Dolz Puig <arnau.dolz@i2cat.net>                                             *
*  Creation:    2024-jul-25                                                                        *
*  Description: Program entry point (main). HIL test                                               *
*                                                                                                  *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date           Author       Description                                                    *
*  0.1  2024-jul-25    ADP          Creation.                                                      *
*  0.2  2024-jul-29    ADP          Added Satellite and Ground Station components.                 *
*  0.3  2024-aug-01    ADP          Added TAP bridge (uselocal).                                   *
*  0.4  2024-aug-05    ADP          Changed TAP interface connections.                             *
*  0.5  2024-aug-06    ADP          Test succesful (Receive ping, send ping, send udp, ARP asnw).  *
*                                                                                                  *
* ------------------------------------------------------------------------------------------------ *
*  Scenario definition:                                                                            *
*   - Satellite (Full simulated node).                                                             *
*   - Ground Station (Ghost node).                                                                 *
*   - TAP Bridge (useBridge).                                                                      *
*   - VM running Ubuntu.                                                                           *
*                                                                                                  *           
*  +---------+                                                                                     *      
*  |    VM   |                                                                                     *
*  | ------- |                                                                                     *  
*  |  Linux  |                                                                                     *
*  | ------- |                                                                                     *
*  |  apps   |                                              +----------+                           *
*  | ------- |                             +----------+     |   SAT    |                           *   
*  |  stack  |                             |   ghost  |     | -------- |                           *
*  | ------- | +--------+                  |   node   |     |   node   |                           *
*  | Virtual | |  TAP   |                  |==========|     | -------- |                           *
*  | Device  | | Device | <---- IPC -----> |   tap    |     |    IP    |                           *
*  +---------+ +--------+                  |  bridge  |     |   stack  |                           *
*      ||          ||                      | -------- |     | -------- |                           *
*  +--------------------+                  |  space   |     |  space   |                           *
*  |     OS Bridge      |                  |   net    |     |   net    |                           *
*  +--------------------+                  |  device  |     |  device  |                           *
*                                          +----------+     +----------+                           *
*                                               ||               ||                                *
*                                          +---------------------------+                           *
*                                          |        SpaceChannel       |                           *
*                                          +---------------------------+                           *
*                                                                                                  *
***************************************************************************************************/

/* Includes: ------------------------------------------------------------------------------------ */
/* Global libraries */
#include "dss.hpp"

/* External libraries */
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"    
#include "ns3/tap-bridge-module.h"

/* Internal libraries */
#include "SpaceChannel.hpp"
#include "SpaceNetDevice.hpp"
#include "KeplerOrbitTrajectory.hpp"
#include "Satellite.hpp"
#include "GroundMobility.hpp"
#include "GroundStation.hpp"
#include "PCAPHelper.hpp"

using namespace ns3;
LOG_COMPONENT_DEFINE("main");

int main(int argc, char** argv)
{
    /** Simulation parameters **/
    std::string mode = "UseBridge";
    std::string tapName = "tap_hil";
    std::string simulation_type = "ns3::RealtimeSimulatorImpl";
    double simulation_time = 900;
    bool checksum = true;

    /** Choose app **/
    int app = 0; // UDP = 1, PING = 2
    bool broadcast = false;
 
    /* Command line arguments */
    CommandLine cmd(__FILE__);
    cmd.AddValue("mode", "Mode setting of TapBridge", mode);
    cmd.AddValue("tapName", "Name of the OS tap device", tapName);
    cmd.Parse(argc, argv);

    /* We are interacting with the outside, real, world.  This means we have to
     * interact in real-time and therefore means we have to use the real-time
     * simulator and take the time to calculate checksums.
     **/
    GlobalValue::Bind("SimulatorImplementationType", StringValue(simulation_type));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (checksum));

    /* Initialize the global variables */
    Globals::init();

    /* Variables:  ------------------------------------------------------------------------------ */
    /** Nodes IDs **/
    std::string sat_id = "SIM-SAT";
    std::string gs_id  = "GHOST-NODE";

    /* Communications parameters */
    uint16_t app_port = 4000;
    double device_queue_size = 200000.0;
    double device_mtu = 1500;
    std::string remote_ipv4_addr = "192.168.56.22";
    std::string remote_mac_addr = "00:00:00:00:00:22";
    std::string sat_ipv4_addr = "192.168.56.11";
    std::string sat_mac_addr = "00:00:00:00:00:11";
    std::string netmask = "255.255.255.0";

    /** Nodes **/
    std::vector<Ptr<Node>> sat_nodes;       /**< Satellite nodes */
    std::vector<Ptr<Node>> gs_nodes;        /**< Ground Station nodes */
    std::vector<Ptr<Satellite>> sats;       /**< Satellite Object*/
    std::vector<Ptr<GroundStation>> gs;    /**< Ground Stations Object */
    /** Trajectories **/
    OrbitalParams orbit_params;                     /**< Orbit parameters */
    std::shared_ptr<OrbitTrajectory> orbit;         /**< Orbit trajectory */
    std::shared_ptr<GroundMobility> gs_trajectory;  /**< Ground Station trajectory */
    /** Comms */
    Ptr<SpaceNetDevice> sat_snd;    /**< Satellite device */
    Ptr<SpaceNetDevice> gs_snd;     /**< Ground Station device */
    InternetStackHelper internet;   /**< Internet stack */
    
    Ptr<Ipv4> ip_v4;                    /**< Ipv4 object */
    int32_t interface;                  /**< Interface */
    Ipv4InterfaceAddress ip_address;    /**< IP address */
    Ptr<ArpCache> arp_cache;            /**< ARP cache */
    ObjectVectorValue interfaces;       /**< Interfaces */
    ArpCache::Entry* entry;             /**< ARP entry */
    SpaceChannel* channel = new SpaceChannel(); /**< Channel */

    TapBridgeHelper tapBridge;              /**< Initialize the TAP Bridge */
    PCAPHelper* helper = new PCAPHelper();  /**< Initialize the Tracing Helper */

    /* Set TAP bridge attributes */
    tapBridge.SetAttribute("Mode", StringValue(mode));
    tapBridge.SetAttribute("DeviceName", StringValue(tapName));

    /* Satabases:  ------------------------------------------------------------------------------ */
    /** Databases declarations **/
    std::map<unsigned int, std::vector<double>> database_orbit;
    std::map<std::string, std::vector<double>> database_transceiver;
    std::map<std::string, double> database_payload;
    std::map<unsigned int, std::vector<std::string>> database_txpl;
    std::map<unsigned int, std::vector<double>> database_gs;
    std::map<unsigned int, std::string> database_txpl_gs;

    /* Satellite database */   
    /** Map with the orbit Configuration with the following structure
     *  | sat_id | semi major axis | eccentricity | inclination | raan | argument of perigee | 
        | mean anomaly | desired mean anomaly |
     **/
    database_orbit.insert({0, {42164e3, 0.0, 90.0, 57.0, 0.0, 50.0, 0.19}});
    /** Map with the transceiver Configuration with the following structure
     *  | transceiver type | data_rate | range |
     **/
    database_transceiver.insert({"instant", {0, 150, 868e6, -111}});
    /** Map with the payload Configuration with the following structure
     *  | transceiver type | data_rate | range |
     **/
    database_payload.insert({"none", 0.0});
    /** Map with the payload Configuration with the following structure
     *  | sat_id | transceiver type | payload type |
     **/
    database_txpl.insert({0, {"instant", "none"}});

    /* Ground Station database */
    /* Ground Station data */
    database_gs.insert({0, {90, -50}});   /* Barcelona */
    /* Ground Station device */
    database_txpl_gs.insert({0, "instant"});

    /* SIMULATED SATELLITE  --------------------------------------------------------------------- */
    /* Initializing all the sat nodes with Protocol Stack */
    
    /* Creation of Node */
    sat_nodes.push_back(CreateObject<Node>());

    /* Configuration of Satellite Component */
    sats.push_back(CreateObject<Satellite, std::string>((std::string) sat_id));

    /* Configuration of Orbit */
    orbit_params = OrbitalParams(database_orbit[0][0], database_orbit[0][1],
        database_orbit[0][2], database_orbit[0][3], database_orbit[0][4]);
    orbit = std::shared_ptr<OrbitTrajectory>(new KeplerOrbitTrajectory(orbit_params, sat_id, 
                                                database_orbit[0][5]));
    sats[0]->setOrbitTrajectory(orbit);

    /** Configuration of SpaceNetDevice **/
    sat_snd = CreateObject<SpaceNetDevice, std::string, double, double, double, double>(
                        (std::string) sat_id,
                        (double) database_transceiver[database_txpl[0][0]][0],
                        (double) database_transceiver[database_txpl[0][0]][1],
                        (double) database_transceiver[database_txpl[0][0]][2],
                        (double) database_transceiver[database_txpl[0][0]][3]);
    sat_snd->SetAddress(Mac48Address(sat_mac_addr.c_str()));    /**< MAC adddress on the SND */
    sat_snd->SetMtu(device_mtu);                                /**< MTU of the device */
    sat_snd->setQueueSize(device_queue_size);                   /**< Queue size of the device */    
    sat_snd->setMobilityModel(orbit);                           /**< Mobility model of the device */
    sat_snd->SetNode(sat_nodes[0]);                             /**< Node attached */    
    channel->addDevice(sat_snd);                                /**< Add device to the channel */

    /* Aggregate the Satellite component to the node */
    sat_nodes[0]->AggregateObject(sats[0]);
    sat_nodes[0]->AddDevice(sat_snd);

    /* Configure Internet Stack */
    internet = InternetStackHelper();
    internet.SetIpv4StackInstall(true);
    internet.SetIpv6StackInstall(false);

    /* Install stack */
    internet.Install(sat_nodes[0]);

    /* Assign IP address */
    ip_v4 = sat_nodes[0]->GetObject<Ipv4>();
    interface = ip_v4->AddInterface(sat_snd);
    ip_address = Ipv4InterfaceAddress(Ipv4Address(sat_ipv4_addr.c_str()), Ipv4Mask(netmask.c_str()));
    ip_v4->AddAddress(interface, ip_address);
    ip_v4->SetMetric(interface, 1);
    ip_v4->SetUp(interface);

    /* Create ARP cache */
    arp_cache = CreateObject<ArpCache>();
    arp_cache->SetAliveTimeout(Seconds(3600*24*365));
    /* Define the different entries to be placed */
    entry = arp_cache->Add(Ipv4Address("192.168.56.22"));
    entry->SetMacAddress (Mac48Address("00:00:00:00:00:22"));
    entry->MarkPermanent ();

    /* Install the arp cache in the node */
    sat_nodes[0]->GetObject<Ipv4L3Protocol> ()->GetAttribute("InterfaceList", interfaces);
    interfaces.Get(0)->GetObject<Ipv4Interface>()->SetAttribute("ArpCache", PointerValue(arp_cache));

    /* Enabling PCAP */
    helper->EnablePcapInternal(sat_id.c_str(), sat_snd, true, false);

        /* Configure Client applications */
        /* Apps  -------------------------------------------------------------------------------- */
        if (app == 1) {
            /* UDP Client (ONlY Sends UDP packets)*/
            UdpClientHelper client;
            client.SetAttribute("MaxPackets", ns3::UintegerValue(simulation_time));
            client.SetAttribute("RemotePort", ns3::UintegerValue(app_port));
            if (broadcast) {
                client.SetAttribute("RemoteAddress",
                                     ns3::AddressValue(ns3::Ipv4Address::GetBroadcast()));
            } else {
                client.SetAttribute("RemoteAddress",
                                     ns3::AddressValue(ns3::Ipv4Address(remote_ipv4_addr.c_str())));
            }
            ApplicationContainer client_apps;
            client_apps = client.Install(sat_nodes[0]);
            client_apps.Start(Seconds(0.0));
            client_apps.Stop(Seconds(simulation_time));
        } else if (app == 2) {
            /* PING */
            Time interPacketInterval{Seconds(1.0)};
            uint32_t size{56};
            uint32_t count{5};
            PingHelper pingHelper(Ipv4Address(remote_ipv4_addr.c_str()),
                                  Ipv4Address(sat_ipv4_addr.c_str()));
            pingHelper.SetAttribute("Interval", TimeValue(interPacketInterval));
            pingHelper.SetAttribute("Size", UintegerValue(size));
            pingHelper.SetAttribute("Count", UintegerValue(count));
            ApplicationContainer apps = pingHelper.Install(sat_nodes[0]);
            apps.Start(Seconds(0.0));
            apps.Stop(Seconds(simulation_time));
        }      

    /* GHOST NODE  ------------------------------------------------------------------------------ */
    /* Creation of Node */
    gs_nodes.push_back(CreateObject<Node>());

    /* Create GroundStation */
    gs.push_back(CreateObject<GroundStation, std::string>((std::string) gs_id));

    /* Configuration of trajectory */
    gs_trajectory = std::make_shared<GroundMobility>(GeographicCoordinates(database_gs[0][0], 
                                                     database_gs[0][1]), 
                                                     gs_id);
    gs[0]->setGroundMobility(gs_trajectory);

    /* Configuration of Device */
    gs_snd = CreateObject<SpaceNetDevice, std::string, double, double, double, double>(
                        (std::string) gs_id,
                        (double) database_transceiver[database_txpl[0][0]][0],
                        (double) database_transceiver[database_txpl[0][0]][1],
                        (double) database_transceiver[database_txpl[0][0]][2],
                        (double) database_transceiver[database_txpl[0][0]][3]);
    gs_snd->SetMtu(device_mtu);
    gs_snd->setQueueSize(device_queue_size);
    gs_snd->setMobilityModel(gs_trajectory);
    gs_snd->SetNode(gs_nodes[0]);
    channel->addDevice(gs_snd);

    /* Aggregate the Satellite component to the node */
    gs_nodes[0]->AggregateObject(gs[0]);
    gs_nodes[0]->AddDevice(gs_snd);

    /* Enabling PCAP */
    helper->EnablePcapInternal("test_gs_DEFAULT", gs_snd, true, false);
       
    /* Setting TAP Bridge */
    tapBridge.Install(gs_nodes[0], gs_snd);
    
    /* Launch framework: ------------------------------------------------------------------------ */
    ns3::Simulator::Stop(ns3::Seconds(simulation_time));
    ns3::Simulator::Run();
    LOG_INFO("Simulation ended, now retrieve final state");
    ns3::Simulator::Destroy();
    LOG_INFO("Simulation ends at %f", simulation_time);

    exit(EXIT_SUCCESS);
}