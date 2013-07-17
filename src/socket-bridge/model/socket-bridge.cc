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

#include "socket-bridge.h"

NS_LOG_COMPONENT_DEFINE ("SocketBridge");

namespace ns3 {

FdReader::Data SocketBridgeFdReader::DoRead (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  uint32_t bufferSize = 65536;
  uint8_t *buf = (uint8_t *)malloc (bufferSize);
  NS_ABORT_MSG_IF (buf == 0, "malloc() failed");

  NS_LOG_LOGIC ("Calling read on IPC socket fd " << m_fd);
  ssize_t len = read (m_fd, buf, bufferSize);
//NS_LOG_UNCOND ("Contiki -> NS3: " << len << " Bytes Read on IPC socket fd " << m_fd);
  if (len <= 0)
    {
      NS_LOG_INFO ("SocketBridgeFdReader::DoRead(): done");
      free (buf);
      buf = 0;
      len = 0;
    }

  return FdReader::Data (buf, len);
}

NS_OBJECT_ENSURE_REGISTERED (SocketBridge);

TypeId
SocketBridge::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SocketBridge")
    .SetParent<NetDevice> ()
    .AddConstructor<SocketBridge> ()
    .AddAttribute ("Start", 
                   "The simulation time at which to spin up the socket device read thread.",
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&SocketBridge::m_tStart),
                   MakeTimeChecker ())
    .AddAttribute ("Stop", 
                   "The simulation time at which to tear down the socket device read thread.",
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&SocketBridge::m_tStop),
                   MakeTimeChecker ())
  ;
  return tid;
}

SocketBridge::SocketBridge ()
  : m_node (0),
    m_ifIndex (0),
    m_sock (-1),
    m_startEvent (),
    m_stopEvent (),
    m_fdReader (0)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_packetBuffer = new uint8_t[65536];
  Start (m_tStart);
}

SocketBridge::~SocketBridge()
{
  NS_LOG_FUNCTION_NOARGS ();

  StopSocketDevice ();

  delete [] m_packetBuffer;
  m_packetBuffer = 0;
  m_bridgedDevice = 0;
}

void
SocketBridge::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NetDevice::DoDispose ();
}

void
SocketBridge::Start (Time tStart)
{
  NS_LOG_FUNCTION (tStart);

  //
  // Cancel any pending start event and schedule a new one at some relative time in the future.
  //
  Simulator::Cancel (m_startEvent);
  m_startEvent = Simulator::Schedule (tStart, &SocketBridge::StartSocketDevice, this);
}

void
SocketBridge::Stop (Time tStop)
{
  NS_LOG_FUNCTION (tStop);
  //
  // Cancel any pending stop event and schedule a new one at some relative time in the future.
  //
  Simulator::Cancel (m_stopEvent);
  m_stopEvent = Simulator::Schedule (tStop, &SocketBridge::StopSocketDevice, this);
}

void
SocketBridge::StartSocketDevice (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ABORT_MSG_IF (m_sock != -1, "SocketBridge::StartSocketDevice(): IPC socket already created");

  m_nodeId = GetNode ()->GetId ();

  //
  // Spin up the Socket bridge and start receiving packets.
  //
  NS_LOG_LOGIC ("Creating IPC Socket");

  CreateSocket ();

  //
  // Now spin up a read thread to read packets from the tap device.
  //
  NS_ABORT_MSG_IF (m_fdReader != 0,"SocketBridge::StartSocketDevice(): Receive thread is already running");
  NS_LOG_LOGIC ("Spinning up read thread");

  m_fdReader = Create<SocketBridgeFdReader> ();
  m_fdReader->Start (m_sock, MakeCallback (&SocketBridge::ReadCallback, this));
}

void
SocketBridge::StopSocketDevice (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_fdReader != 0)
    {
      m_fdReader->Stop ();
      m_fdReader = 0;
    }

  if (m_sock != -1)
    {
      close (m_sock);
      m_sock = -1;
    }
NS_LOG_UNCOND("Killing Child");
  kill(child,SIGKILL);
}

void
SocketBridge::CreateSocket (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
    NS_ABORT_MSG ("SocketBridge::CreateSocket(): Unix socket creation error, errno = " << strerror (errno));

  Ptr<NetDevice> nd = GetBridgedNetDevice ();
  Ptr<Node> n = nd->GetNode ();

  /* Generate MAC address, assign to Node */ 
  Mac64Address mac64Address = Mac64Address::Allocate();

  Address ndAddress = Address(mac64Address);
  nd->SetAddress(ndAddress); 
  
  if ((child = fork()) == -1)
    NS_ABORT_MSG ("SocketBridge::CreateSockete(): Unix fork error, errno = " << strerror (errno));
  else if (child) {   /*  This is the parent. */
    NS_LOG_DEBUG ("Parent process");
    close(sockets[1]);
    NS_LOG_INFO ("Got the socket from the socket creator = " << sockets[0]);
    m_sock = sockets[0];
//NS_LOG_UNCOND("Child PID: " << child);
  } else {            /*  This is the child. */
    close(sockets[0]);
   
    /* Pass address to child application */
    std::ostringstream ossMac;
    ossMac << "-a" << mac64Address;

    /* Exec Contiki Node with File Descriptor of socket */
    dup2(sockets[1], STDIN_FILENO);

    char * path;
    path = (char *)m_execPath.c_str();
    ::execlp (path, 
              path,                       // argv[0] (filename)
              ossMac.str ().c_str (),     // argv[1] (-m<MAC address>)
              (char *)NULL);
    //
    // If the execlp successfully completes, it never returns.  If it returns it failed or the OS is
    // broken.  In either case, we bail.
    //
    NS_FATAL_ERROR ("SocketBridge::CreateSocket(): Back from execlp(), errno = " << ::strerror (errno));
  }
}

void
SocketBridge::ReadCallback (uint8_t *buf, ssize_t len)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT_MSG (buf != 0, "invalid buf argument");
  NS_ASSERT_MSG (len > 0, "invalid len argument");

  NS_LOG_INFO ("SocketBridge::ReadCallback(): Received packet on node " << m_nodeId);
  NS_LOG_INFO ("SocketBridge::ReadCallback(): Scheduling handler");
  Simulator::ScheduleWithContext (m_nodeId, Seconds (0.0), MakeEvent (&SocketBridge::ForwardToBridgedDevice, this, buf, len));
}

void
SocketBridge::ForwardToBridgedDevice (uint8_t *buf, ssize_t len)
{
  NS_LOG_FUNCTION (buf << len);

  //
  // First, create a packet out of the byte buffer we received and free that
  // buffer.
  //
  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t *> (buf), len);
  free (buf);
  buf = 0;

  Address src, dst;
  uint16_t type;

  NS_LOG_LOGIC ("Received packet from socket");
  
  // Pull source, destination and type information from packet
  Ptr<Packet> p = Filter (packet, &src, &dst, &type);

  if (p == 0)
    {
      NS_LOG_LOGIC ("SocketBridge::ForwardToBridgedDevice:  Discarding packet as unfit for ns-3 consumption");
      return;
    }

  NS_LOG_LOGIC ("Pkt source is " << src);
  NS_LOG_LOGIC ("Pkt destination is " << dst);
  NS_LOG_LOGIC ("Pkt LengthType is " << type);
  NS_LOG_LOGIC ("Forwarding packet from external socket to simulated network");

  if (m_mode == MACPHYOVERLAY)
    {
      if (m_ns3AddressRewritten == false)
        {
          //
          // Set the ns-3 device's mac address to the overlying container's
          // mac address
          //
          Mac48Address learnedMac = Mac48Address::ConvertFrom (src);
          NS_LOG_LOGIC ("Learned MacAddr is " << learnedMac << ": setting ns-3 device to use this address");
          m_bridgedDevice->SetAddress (Mac48Address::ConvertFrom (learnedMac));
          m_ns3AddressRewritten = true;
        }

      NS_LOG_LOGIC ("Forwarding packet to ns-3 device via Send()");
      m_bridgedDevice->Send (packet, dst, type);
      //m_bridgedDevice->SendFrom (packet, src, dst, type);
      return;
    }
  else {
	Address nullAddr = Address();
	m_bridgedDevice->Send(packet, nullAddr, uint16_t(0));
  }
}

Ptr<Packet>
SocketBridge::Filter (Ptr<Packet> p, Address *src, Address *dst, uint16_t *type)
{
  NS_LOG_FUNCTION (p);
  /* Fill out src, dst and maybe type for the Send() function 
 *   This needs to be completed for MACOVERLAY mode to function - currently crashes because improper src/dst assigned */
  return p;
}

Ptr<NetDevice>
SocketBridge::GetBridgedNetDevice (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_bridgedDevice;
}

void
SocketBridge::SetExecPath (std::string path)
{
  m_execPath = path;
}


void
SocketBridge::SetMac (Ptr<SocketNullMac> mac)
{
  m_macLayer = mac;
}

Ptr<SocketNullMac>
SocketBridge::GetMac (void)
{
  return m_macLayer;
}

void
SocketBridge::SetPhy (Ptr<SocketContikiPhy> phy)
{
  m_phy = phy;
}

Ptr<SocketContikiPhy>
SocketBridge::GetPhy (void)
{
  return m_phy;
}

void 
SocketBridge::SetBridgedNetDevice (Ptr<NetDevice> bridgedDevice)
{
  NS_LOG_FUNCTION (bridgedDevice);

  NS_ASSERT_MSG (m_node != 0, "SocketBridge::SetBridgedDevice:  Bridge not installed in a node");
  //NS_ASSERT_MSG (bridgedDevice != this, "SocketBridge::SetBridgedDevice:  Cannot bridge to self");
  NS_ASSERT_MSG (m_bridgedDevice == 0, "SocketBridge::SetBridgedDevice:  Already bridged");

  /* Disconnect the bridged device from the native ns-3 stack 
  *  and branch to network stack on the other side of the socket. */
  bridgedDevice->SetReceiveCallback (MakeCallback (&SocketBridge::DiscardFromBridgedDevice, this));
  bridgedDevice->SetPromiscReceiveCallback (MakeCallback (&SocketBridge::ReceiveFromBridgedDevice, this));
  m_bridgedDevice = bridgedDevice;
}

bool
SocketBridge::DiscardFromBridgedDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &src)
{
  NS_LOG_FUNCTION (device << packet << protocol << src);
  NS_LOG_LOGIC ("Discarding packet stolen from bridged device " << device);
  return true;
}

bool
SocketBridge::ReceiveFromBridgedDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &src, Address const &dst, PacketType packetType)
{
  NS_LOG_DEBUG ("Packet UID is " << packet->GetUid ());
  /* Forward packet to socket */
  Ptr<Packet> p = packet->Copy ();
  NS_LOG_LOGIC ("Writing packet to socket");
  p->CopyData (m_packetBuffer, p->GetSize ());

  uint32_t bytesWritten = write (m_sock, m_packetBuffer, p->GetSize ());
  NS_ABORT_MSG_IF (bytesWritten != p->GetSize (), "SocketBridge::ReceiveFromBridgedDevice(): Write error.");
//NS_LOG_UNCOND("NS3 -> Contiki: Wrote " << bytesWritten << " bytes to socket " << m_sock);
  NS_LOG_LOGIC ("End of receive packet handling on node " << m_node->GetId ());
  return true;
}

void 
SocketBridge::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ifIndex = index;
}

uint32_t 
SocketBridge::GetIfIndex (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifIndex;
}

Ptr<Channel> 
SocketBridge::GetChannel (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return 0;
}

void
SocketBridge::SetAddress (Address address)
{
  NS_LOG_FUNCTION (address);
  m_address = Mac64Address::ConvertFrom (address);
}

Address 
SocketBridge::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

void
SocketBridge::SetMode (std::string mode)
{
  NS_LOG_FUNCTION (mode);
  if (mode.compare("MACPHYOVERLAY") == 0) {
    m_mode = (SocketBridge::Mode) 2;
  }
  else if (mode.compare("PHYOVERLAY") == 0) {
    m_mode = (SocketBridge::Mode) 1;
  }
  else {
    m_mode = (SocketBridge::Mode) 0;
  }
}

SocketBridge::Mode
SocketBridge::GetMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mode;
}

bool 
SocketBridge::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_mtu = mtu;
  return true;
}

uint16_t 
SocketBridge::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}

bool 
SocketBridge::IsLinkUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void 
SocketBridge::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool 
SocketBridge::IsBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
SocketBridge::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac64Address ("ff:ff:ff:ff:ff:ff:ff:ff");
}

bool
SocketBridge::IsMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
SocketBridge::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast (multicastGroup);
  return multicast;
}

bool 
SocketBridge::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool 
SocketBridge::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  //
  // Returning false from IsBridge in a device called SocketBridge may seem odd
  // at first glance, but this test is for a device that bridges ns-3 devices
  // together.  The Tap bridge doesn't do that -- it bridges an ns-3 device to
  // a Linux device.  This is a completely different story.
  // 
  return false;
}

bool 
SocketBridge::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet);
  /* Send to MAC Layer */
  m_macLayer->Enqueue(packet);
  return true;
}

bool
SocketBridge::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dst, uint16_t protocol)
{
  NS_LOG_FUNCTION (packet << src << dst << protocol);
  NS_FATAL_ERROR ("SocketBridge::Send: You may not call SendFrom on a SocketBridge directly");
  return true;
}

Ptr<Node> 
SocketBridge::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

void 
SocketBridge::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = node;
}

bool 
SocketBridge::NeedsArp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void 
SocketBridge::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_rxCallback = cb;
}

void 
SocketBridge::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_promiscRxCallback = cb;
}

bool
SocketBridge::SupportsSendFrom () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address SocketBridge::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

} // namespace ns3
