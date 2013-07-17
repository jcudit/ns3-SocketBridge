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
#ifndef SOCKET_NULL_MAC_H
#define SOCKET_NULL_MAC_H

#include "ns3/packet.h"
#include "ns3/mac64-address.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/trace-source-accessor.h"

#include "socket-contiki-phy.h"
#include "socket-bridge.h"

namespace ns3 {

class SocketContikiPhy;
class SocketBridge;

class SocketNullMac : public Object
{
public:
  static TypeId GetTypeId (void);

  SocketNullMac ();
  virtual ~SocketNullMac ();

  /**
   * \returns the MAC address associated to this MAC layer.
   */
  Mac64Address GetAddress (void);

  /**
   * \param address the current address of this MAC layer.
   */
  void SetAddress (Mac64Address address);

  /**
   * \param packet the packet to send.
   */
  void Enqueue (Ptr<const Packet> packet);

  /**
   * \param phy the physical layer attached to this MAC.
   */
  void SetPhy (Ptr<SocketContikiPhy> phy);

  /**
   * \param bridge the SocketBridge attached to this MAC.
   */
  void SetBridge (Ptr<SocketBridge> bridge);

  /**
   * Public method used to fire a MacTx trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyTx (Ptr<const Packet> packet);

  /**
   * Public method used to fire a MacRx trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyRx (Ptr<const Packet> packet);

  /**
   * Packet received from the channel
   */
  void Receive (Ptr<Packet> packet);
  
  /**
   * Packet is forwarded through the socket and out of the ns-3 domain
   */
  void ForwardUp (Ptr<Packet> packet);

  
private:
  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * are dropped at the MAC layer during transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a non- promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * are dropped at the MAC layer during reception.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * Stores the MAC address assigned to this MAC layer
   */
  Mac64Address m_macAddr;

  /**
   * Holds a reference to the PHY layer associated with this MAC layer
   */
  Ptr<SocketContikiPhy> m_phy;

  /**
   * Holds a reference to the SocketBridge associated with this MAC layer
   */
  Ptr<SocketBridge> m_bridge;


};

} // namespace ns3

#endif /* SOCKET_NULL_MAC_H */
