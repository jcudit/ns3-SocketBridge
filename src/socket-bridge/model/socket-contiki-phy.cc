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

#include "socket-contiki-phy.h"

NS_LOG_COMPONENT_DEFINE ("SocketContikiPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SocketContikiPhy);

TypeId
SocketContikiPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SocketContikiPhy")
    .SetParent<Object> ()
    .AddConstructor<SocketContikiPhy> ()
  ;
  return tid;
}

SocketContikiPhy::SocketContikiPhy ()
{
  NS_LOG_FUNCTION (this);
}

SocketContikiPhy::~SocketContikiPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SocketContikiPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_mobility = 0;
  m_channel = 0;
  m_mode = DSSS_O_QPSK_GHz; 
}

void
SocketContikiPhy::SetDevice (Ptr<Object> device)
{
  m_device = device;
}
void
SocketContikiPhy::SetMobility (Ptr<Object> mobility)
{
  m_mobility = mobility;
}

Ptr<Object>
SocketContikiPhy::GetDevice (void) const
{
  return m_device;
}
Ptr<Object>
SocketContikiPhy::GetMobility (void)
{
  return m_mobility;
}

Time
SocketContikiPhy::CalculateTxDuration (uint32_t size)
{
  uint32_t duration, symbolRate, preambleDuration;
  uint8_t bitsPerSymbol, sfdSymbols;


  switch (m_mode)
  {
    case DSSS_BPSK:
      // IEEE Std 802.15.4-2006 section 6.6.3.3 (868 MHz)
      symbolRate = 20000;
      // IEEE Std 802.15.4-2006 section 6.6.2.3
      bitsPerSymbol = 1;
      // IEEE Std 802.15.4-2006 section 6.3.1 Table 19
      preambleDuration = 1600;
      // IEEE Std 802.15.4-2006 section 6.3.2 Table 20 
      sfdSymbols = 8;
      break;
    case DSSS_O_QPSK_MHz:
      // IEEE Std 802.15.4-2006 section 6.8.3.3 (868 MHz)
      symbolRate = 25000;
      // IEEE Std 802.15.4-2006 section 6.8.2.2
      bitsPerSymbol = 4;
      // IEEE Std 802.15.4-2006 section 6.3.1 Table 19
      preambleDuration = 320;
      // IEEE Std 802.15.4-2006 section 6.3.2 Table 20 
      sfdSymbols = 2;
      break;
    case PSSS_ASK:
      // IEEE Std 802.15.4-2006 section 6.7.3.3 (868 MHz)
      symbolRate = 12500;
      // IEEE Std 802.15.4-2006 section 6.7.2.2
      bitsPerSymbol = 20;
      // IEEE Std 802.15.4-2006 section 6.3.1 Table 19
      preambleDuration = 160;
      // IEEE Std 802.15.4-2006 section 6.3.2 Table 20 
      sfdSymbols = 1;
      break;
    default:
      // IEEE Std 802.15.4-2006 section 6.5.3.2
      symbolRate = 62500;
      // IEEE Std 802.15.4-2006 section 6.5.2.2
      bitsPerSymbol = 4;
      // IEEE Std 802.15.4-2006 section 6.3.1 Table 19
      preambleDuration = 128;
      // IEEE Std 802.15.4-2006 section 6.3.2 Table 20 
      sfdSymbols = 2;
  } 

  /* Synchronization Header + PHY Header + PHY Payload */
  duration = preambleDuration +
             lrint( ((ceil ((size * 8) / bitsPerSymbol)) + sfdSymbols) / 
             (symbolRate * 1e-6) );
  // Duration = Preamble plus 
  // number of symbols to transmit (ceil/size*8/bitsPerSymbol/sfdSymbols)
  // divided by duration of symbol (symbolRate * 1e-6)

  return MicroSeconds(duration);
}

void
SocketContikiPhy::StartReceivePacket (Ptr<Packet> packet, double rxPowerDbm) 
{ 
  NS_LOG_FUNCTION (this << packet << rxPowerDbm);
  //rxPowerDbm += m_rxGainDb;
  double rxPowerW = DbmToW (rxPowerDbm);
  Time rxDuration = CalculateTxDuration (packet->GetSize ());

  if (rxPowerW > m_edThresholdW)
  {
    NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
    Simulator::Schedule (rxDuration, &SocketContikiPhy::EndReceive, this, packet);
  }
  else
  {
    NS_LOG_DEBUG ("drop packet because signal power too Small (" <<
                  rxPowerW << "<" << m_edThresholdW << ")");
    //NotifyRxDrop (packet);
  }
}

void
SocketContikiPhy::EndReceive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  /*  If SNR and Packet Error Rate are acceptable */
  //if (m_random.GetValue (0, 1) > 0.1)
  if (1)
    { 
      /* Pass packet up the stack to the MAC Layer */
      m_rxOkCallback(packet);
    }
  else
    { 
      /*  failure. */
      //NotifyRxDrop (packet);
    }
}

void
SocketContikiPhy::RegisterListener (SocketNullMac *listener)
{
  m_listeners.push_back (listener);
}

void
SocketContikiPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_rxOkCallback = callback;
}

void
SocketContikiPhy::SendPacket (Ptr<const Packet> packet)
{
  Ptr<SocketContikiPhy> ptr = this;
  NS_LOG_FUNCTION (this << packet);
  m_channel->Send (ptr, packet, 65);
}

Ptr<SocketChannel>
SocketContikiPhy::GetChannel (void) const
{
  return m_channel;
}

void
SocketContikiPhy::SetChannel (Ptr<SocketChannel> channel)
{
  Ptr<SocketContikiPhy> ptr = this;
  m_channel = channel;
  channel->Add (ptr);
}

void
SocketContikiPhy::SetEdThreshold (double edThreshold)
{
  m_edThresholdW = edThreshold;
}

uint64_t
SocketContikiPhy::GetDataRate (void)
{
  return m_dataRate;
}

void
SocketContikiPhy::SetDataRate (uint64_t dataRate)
{
  m_dataRate = dataRate;
}

SocketContikiPhy::PhyMode
SocketContikiPhy::GetMode (void)
{
  return m_mode;
}

void
SocketContikiPhy::SetMode (PhyMode mode)
{
  m_mode = mode;
  switch (m_mode)
  {
    case DSSS_BPSK:
      // IEEE Std 802.15.4-2006 section 6.1.1 Table 1 (868 MHz)
      SetDataRate(20000);
      // IEEE Std 802.15.4-2006 section 6.6.3.4
      SetEdThreshold(-92);
      break;
    case DSSS_O_QPSK_MHz:
      // IEEE Std 802.15.4-2006 section 6.1.1 Table 1 (868 MHz)
      SetDataRate(100000);
      // IEEE Std 802.15.4-2006 section 6.8.3.4
      SetEdThreshold(-85);
      break;
    case PSSS_ASK:
      // IEEE Std 802.15.4-2006 section 6.1.1 Table 1 (868 MHz)
      SetDataRate(250000);
      // IEEE Std 802.15.4-2006 section 6.7.3.4
      SetEdThreshold(-85);
      break;
    default:
      // IEEE Std 802.15.4-2006 section 6.1.1 Table 1 (868 MHz)
      SetDataRate(250000);
      // IEEE Std 802.15.4-2006 section 6.5.3.3
      SetEdThreshold(-85);
  }
}

double
SocketContikiPhy::DbmToW (double dBm) const
{
  double mW = pow (10.0,dBm / 10.0);
  return mW / 1000.0;
}

} // namespace ns3
