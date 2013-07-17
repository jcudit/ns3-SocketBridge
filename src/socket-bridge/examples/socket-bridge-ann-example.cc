/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <stdio.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/wifi-module.h"
#include "ns3/socket-bridge-module.h"
#include "ns3/socket-bridge-helper.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SocketBridgeANNExample");

int 
main (int argc, char *argv[])
{
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  /* Create 70 nodes - Remember to adjust NTABLE value to Nodes - 1 */
  NodeContainer nodes;
  nodes.Create(70);

  /* Bridge nodes to Contiki processes */ 
  SocketBridgeHelper socketBridgeHelper;
  socketBridgeHelper.Install(nodes, "/cn8801/contiki/examples/ns3-ann/ns3-ann.ns3");

  Simulator::Stop (Seconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}
