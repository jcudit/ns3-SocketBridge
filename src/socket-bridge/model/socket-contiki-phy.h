/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#ifndef SOCKET_CONTIKI_PHY_H
#define SOCKET_CONTIKI_PHY_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/trace-source-accessor.h"

#include <math.h>
#include <stdio.h>

#include "socket-channel.h"
#include "socket-phy.h"
#include "socket-null-mac.h"



namespace ns3 {

class SocketChannel;
class SocketNullMac;

class SocketContikiPhy : public SocketPhy 
{

  typedef Callback< void,Ptr<Packet> > RxOkCallback;
  typedef std::vector<SocketNullMac *> Listeners;

public:
  static TypeId GetTypeId (void);

  enum PhyMode
  {
    DSSS_BPSK,
    DSSS_O_QPSK_MHz,
    DSSS_O_QPSK_GHz,
    PSSS_ASK
  };


  SocketContikiPhy ();
  virtual ~SocketContikiPhy ();

  void StartReceivePacket (Ptr<Packet> packet, double rxPowerDbm);
  void SetDevice (Ptr<Object> device);
  void SetMobility (Ptr<Object> mobility);
  void SetEdThreshold (double threshold);
  void SetDataRate (uint64_t dataRate);
  void SetMode (PhyMode mode);
  Ptr<Object> GetDevice (void) const;
  Ptr<Object> GetMobility (void);
  uint64_t GetDataRate (void);
  PhyMode GetMode (void);
  virtual void SendPacket (Ptr<const Packet> packet);

  virtual void RegisterListener (SocketNullMac *listener);
  virtual void SetReceiveOkCallback (RxOkCallback callback);

  void SetChannel (Ptr<SocketChannel> channel);
  Ptr<SocketChannel> GetChannel (void) const;


private:
  virtual void DoDispose (void);
  virtual void EndReceive (Ptr<Packet> packet);
  double DbmToW (double dBm) const;

  Time CalculateTxDuration (uint32_t size);

  Ptr<Object> m_device;
  Ptr<Object> m_mobility;
  Ptr<SocketChannel> m_channel;
  Listeners m_listeners;
  uint64_t m_dataRate;
  PhyMode m_mode;
  double m_edThresholdW;

  RxOkCallback m_rxOkCallback;
  EventId m_endRxEvent;
  UniformVariable m_random;
};

} // namespace ns3


#endif /* SOCKET_CONTIKI_PHY_H */
