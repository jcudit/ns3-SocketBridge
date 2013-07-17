/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/enum.h"
#include "ns3/names.h"
#include "ns3/mobility-module.h"
#include "socket-bridge-helper.h"
#include "socket-null-mac-helper.h"
#include "socket-contiki-phy-helper.h"
#include "socket-channel-helper.h"

NS_LOG_COMPONENT_DEFINE ("SocketBridgeHelper");

namespace ns3 {

SocketBridgeHelper::SocketBridgeHelper ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_deviceFactory.SetTypeId ("ns3::SocketBridge");
}

void 
SocketBridgeHelper::SetAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (n1 << &v1);
  m_deviceFactory.Set (n1, v1);
}

Ptr<SocketBridge>
SocketBridgeHelper::Install (Ptr<Node> node)
{
  /* Connect node to bridge */
  Ptr<SocketBridge> bridge = m_deviceFactory.Create<SocketBridge> ();
  node->AddDevice (bridge);
  /* Loop bridge to itself */
  bridge->SetBridgedNetDevice (bridge);

  return bridge;
}

Ptr<NetDevice>
SocketBridgeHelper::Install (Ptr<Node> node, Ptr<NetDevice> nd)
{
  Ptr<SocketBridge> bridge = m_deviceFactory.Create<SocketBridge> ();
  node->AddDevice (bridge);
  bridge->SetBridgedNetDevice (nd);

  return bridge;
}

void
SocketBridgeHelper::Install (NodeContainer nodes, std::string path, std::string mode)
{
  uint32_t nodeCount = nodes.GetN();

  /* Create Pointers */
  Ptr<SocketBridge> bridge [nodeCount + 1];
  Ptr<SocketNullMac> mac [nodeCount + 1];
  Ptr<SocketContikiPhy> phy [nodeCount + 1];

  /* Create Helpers */
  SocketNullMacHelper socketNullMacHelper;
  SocketContikiPhyHelper socketContikiPhyHelper;
  SocketChannelHelper socketChannelHelper;

  /* Create Channel */
  Ptr<SocketChannel> channel = socketChannelHelper.Create();
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  channel->SetPropagationLossModel (log);

  /* Create Position of all Nodes */
  Ptr<MobilityModel> pos = CreateObject<ConstantPositionMobilityModel> ();
  pos->SetPosition (Vector (0.0, 0.0, 0.0));

  /* Build Network Stack for all Nodes */
  for (uint8_t i = 0; i < nodeCount; i++)
  {
    bridge[i] =  m_deviceFactory.Create<SocketBridge> (); 
    bridge[i]->SetExecPath(path);
    bridge[i]->SetMode(mode);
    /* Add Socket Bridge to Node (Spawns Contiki Process */
    nodes.Get(i)->AddDevice(bridge[i]);
    bridge[i]->SetBridgedNetDevice(bridge[i]); 
    /* Add MAC layer to SocketBridge */
    mac[i] = socketNullMacHelper.Install(bridge[i]);
    /* Add PHY to SocketBridge and NullMac */
    phy[i] = socketContikiPhyHelper.Install(bridge[i], mac[i], SocketContikiPhy::DSSS_O_QPSK_GHz); 

    /* Add Physical Components (Channel and Position) */
    socketChannelHelper.Install(channel, bridge[i]);
    phy[i]->SetMobility(pos);
  }
}


} // namespace ns3
