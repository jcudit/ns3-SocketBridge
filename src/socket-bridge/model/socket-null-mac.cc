/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "socket-null-mac.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SocketNullMac);

TypeId
SocketNullMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SocketNullMac")
    .SetParent<Object> ()
    .AddConstructor<SocketNullMac> ()
    .AddTraceSource ("MacTx",
                     "A packet has been received from higher layers and is being processed in preparation for "
                     "queueing for transmission.",
                     MakeTraceSourceAccessor (&SocketNullMac::m_macTxTrace))
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SocketNullMac::m_macRxTrace))
#if 0
    // Not currently implemented in this device
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&SocketNullMac::m_snifferTrace))
#endif
  ;

  return tid;
}

SocketNullMac::SocketNullMac ()
{
  //NS_LOG_FUNCTION_NOARGS ();
}

SocketNullMac::~SocketNullMac ()
{
  //NS_LOG_FUNCTION_NOARGS ();
}

Mac64Address 
SocketNullMac::GetAddress ()
{
  return m_macAddr;
}

void 
SocketNullMac::SetAddress (Mac64Address address)
{
  m_macAddr = address;
}

void
SocketNullMac::NotifyTx (Ptr<const Packet> packet)
{
  m_macTxTrace (packet);
}

void
SocketNullMac::NotifyRx (Ptr<const Packet> packet)
{
  m_macRxTrace (packet);
}

void 
SocketNullMac::Enqueue (Ptr<const Packet> packet)
{
  /* Forward packet to lower layers without processing */
  m_phy->SendPacket(packet); 
}

void 
SocketNullMac::SetPhy (Ptr<SocketContikiPhy> phy)
{
  /*  Dependant on PHY implementation; this will change */
  m_phy = phy;
  m_phy->SetReceiveOkCallback (MakeCallback (&SocketNullMac::Receive, this));
  phy->RegisterListener (this);
}

void 
SocketNullMac::SetBridge (Ptr<SocketBridge> bridge)
{
  /*  Dependant on PHY implementation; this will change */
  m_bridge = bridge;
}

void 
SocketNullMac::Receive (Ptr<Packet> packet)
{
  /* Optionally Strip unnecessary ns-3 information before passing to socket */
  ForwardUp (packet);
}

void 
SocketNullMac::ForwardUp (Ptr<Packet> packet)
{
  Address nullSource = Address();
  Address nullDest = Address();
  /*  Pass to socket bridge */
  m_bridge->ReceiveFromBridgedDevice(m_bridge, packet, 0, nullSource, nullDest, NetDevice::PACKET_HOST);
  //NS_LOG_FUNCTION(this << packet);
}

} // namespace ns3
