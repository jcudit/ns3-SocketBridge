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

#ifndef SOCKET_BRIDGE_H
#define SOCKET_BRIDGE_H

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"
#include "ns3/mac64-address.h"
#include "ns3/unix-fd-reader.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/ipv6.h"
#include "ns3/simulator.h"
#include "ns3/realtime-simulator-impl.h"
#include "ns3/uinteger.h"

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "socket-null-mac.h"
#include "socket-contiki-phy.h"

namespace ns3 {

class SocketBridgeFdReader : public FdReader
{
private:
  FdReader::Data DoRead (void);
};

class Node;
class SocketNullMac;
class SocketContikiPhy;

/**
 * \ingroup socket-bridge
 * 
 * \brief A bridge to make it appear that a real host process is connected to 
 * an ns-3 net device via the OS sockets interface.
 *
 */
class SocketBridge : public NetDevice
{
public:
  static TypeId GetTypeId (void);

  /**
   * Enumeration of the operating modes supported in the class.
   *
   */
  enum Mode {
    ILLEGAL,          /**< mode not set */
    PHYOVERLAY,       /**< ns-3 medium emulation only */
    MACPHYOVERLAY,    /**< ns-3 MAC-layer stack participation with medium emulation */
  };

  SocketBridge ();
  virtual ~SocketBridge ();

  /**
   * \brief Get the bridged net device.
   *
   * The bridged net device is the ns-3 device to which this bridge is connected,
   *
   * \returns the bridged net device.
   */
  Ptr<NetDevice> GetBridgedNetDevice (void);

  /**
     * \brief Set the executable path used with the fork/exec call.
     *
     * This method specifies the location on the host OS where the external application running
     * the alternate stack and application layer is located.
     *
     * \param path The executable path 
     */
    void SetExecPath (std::string path);


  /**
     * \brief Set the MAC layer.
     *
     * This method tells the bridge which ns-3 MAC layer it should use to connect
     * through to the channel.
     *
     * \param mac MAC layer to set
     */
    void SetMac (Ptr<SocketNullMac> mac);

  /**
     * \brief Returns the attached MAC layer.
     *
     * This method returns the attached MAC layer which was previously set using SetMac.
     *
     * \returns the attached MAC layer
     */
    Ptr<SocketNullMac> GetMac (void);

  /**
     * \brief Set the PHY object attached to the attached MAC layer 
     *
     * This method tells the bridge which ns-3 PHY object is attached to the MAC layer
     * that has already been assigned to the bridge.
     *
     * \param phy PHY to set
     */
    void SetPhy (Ptr<SocketContikiPhy> phy);

  /**
     * \brief Returns the attached PHY object.
     *
     * This method returns the attached PHY object which was previously set using SetPhy.
     *
     * \returns the attached PHY object
     */
    Ptr<SocketContikiPhy> GetPhy (void);

  /**
   * \brief Set the ns-3 net device to bridge.
   *
   * This method tells the bridge which ns-3 net device it should use to connect
   * the simulation side of the bridge.
   *
   * \param bridgedDevice device to set
   */
  void SetBridgedNetDevice (Ptr<NetDevice> bridgedDevice);

  /**
   * \brief Pass packets from underlying layers into the bridge to be sent to the socket.
   *
   * This method receives the packet and ancilliary data and forwards this information to the socket.
   * Parameters follow the PromiscReceiveCallback type definition that belongs to a NetDevice.
   *
   * \param device Originating NetDevice
   * \param packet The packet extracted from the channel
   * \param protocol Protocol of the packet
   * \param src Source address of the packet
   * \param dst Destination address of the packet
   * \param packetType Type of the packet in terms of transmission (i.e. unicast, broadcast, etc.)
   */
  bool ReceiveFromBridgedDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &src, Address const &dst, PacketType packetType);

  /**
   * \brief Set a start time for the device.
   *
   * The socket bridge consumes a non-trivial amount of time to start.  It starts
   * up in the context of a scheduled event to ensure that all configuration
   * has been completed before extracting the configuration (IP addresses, etc.)
   * In order to allow a more reasonable start-up sequence than a thundering 
   * herd of devices, the time at which each device starts is also configurable
   * via the Attribute system and via this call.
   *
   * \param tStart the start time
   */
  void Start (Time tStart);

  /**
   * Set a stop time for the device.
   *
   * @param tStop the stop time
   *
   * \see SocketBridge::Start
   */
  void Stop (Time tStop);

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   */
  void SetMode (std::string mode);

  /**
   * Get the operating mode of this device.
   *
   * \returns The operating mode of this device.
   */
  SocketBridge::Mode  GetMode (void);

  //
  // The following methods are inherited from NetDevice base class and are
  // documented there.
  //
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom () const;
  virtual Address GetMulticast (Ipv6Address addr) const;

protected:
  virtual void DoDispose (void);


  bool DiscardFromBridgedDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &src);

private:

  /**
   * \internal
   *
   * Create a pair of sockets and spawn a new process passing one socket to it for IPC.
   * If this method returns, we'll have a socket waiting for us in m_sock that we can use
   * to talk to the process on the other end of the socket.
   */
  void CreateSocket (void);

  /**
   * \internal
   *
   * Spin up the device
   */
  void StartSocketDevice (void);

  /**
   * \internal
   *
   * Tear down the device
   */
  void StopSocketDevice (void);

  /**
   * \internal
   *
   * Callback to process packets that are read
   */
  void ReadCallback (uint8_t *buf, ssize_t len);

  /*
   * \internal
   *
   * Forward a packet received from the socket to the bridged ns-3 
   * device
   *
   * \param buf A character buffer containing the actual packet bits that were
   *            received from the host.
   * \param buf The length of the buffer.
   */
  void ForwardToBridgedDevice (uint8_t *buf, ssize_t len);

  /**
   * \internal
   *
   * Sanity checking and information extraction from the packet to aid propogation
   * through the ns-3 medium.
   *
   * \param packet The packet we received from the host, and which we need 
   *               to check.
   * \param src    A pointer to the data structure that will get the source
   *               address of the packet (extracted from the packet Layer 2 
   *               header).
   * \param dst    A pointer to the data structure that will get the destination
   *               address of the packet (extracted from the packet Layer 2 
   *               header).
   * \param type   A pointer to the variable that will get the packet type from 
   *               the Layer 2 header.
   */
  Ptr<Packet> Filter (Ptr<Packet> packet, Address *src, Address *dst, uint16_t *type);

  /**
   * \internal
   *
   * Callback used to hook the standard packet receive callback of the SocketBridge
   * ns-3 net device.  This is never called, and therefore no packets will ever
   * be received forwarded up the IP stack on the ghost node through this device.
   */
  NetDevice::ReceiveCallback m_rxCallback;

  /**
   * \internal
   *
   * Callback used to hook the promiscuous packet receive callback of the SocketBridge
   * ns-3 net device.  This is never called, and therefore no packets will ever
   * be received forwarded up the IP stack on the ghost node through this device.
   *
   * Note that we intercept the similar callback in the bridged device in order to
   * do the actual bridging between the bridged ns-3 net device and the Socket device
   * on the host.
   */
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  /**
   * \internal
   *
   * Pointer to the (ghost) Node to which we are connected.
   */
  Ptr<Node> m_node;


  /**
   * \internal
   *
   * The ns-3 interface index of this SocketBridge net device.
   */
  uint32_t m_ifIndex;

  /**
   * \internal
   *
   * The common mtu to use for the net devices
   */
  uint16_t m_mtu;

  /**
   * \internal
   *
   * The socket (actually interpreted as fd) to use to talk to the process on the other
   * end of the socket.
   */
  int m_sock;

  /**
   * \internal
   *
   * The ID of the ns-3 event used to schedule the start up of the underlying
   * host Socket device and ns-3 read thread.
   */
  EventId m_startEvent;

  /**
   * \internal
   *
   * The ID of the ns-3 event used to schedule the tear down of the underlying
   * host Socket device and ns-3 read thread.
   */
  EventId m_stopEvent;

  /**
   * \internal
   *
   * Includes the ns-3 read thread used to do blocking reads on the fd
   * corresponding to the host device.
   */
  Ptr<SocketBridgeFdReader> m_fdReader;

  /**
   * \internal
   *
   * The operating mode of the bridge. Indicates overall network stack participation. 
   */
  Mode m_mode;

  /**
   * \internal
   *
   * The (unused) MAC address of the SocketBridge net device.  Since the SocketBridge
   * is implemented as a ns-3 net device, it is required to implement certain
   * functionality.  In this case, the SocketBridge is automatically assigned a
   * MAC address, but it is not used.
   */
  Mac64Address m_address;

  /**
   * \internal
   *
   * Time to start spinning up the device
   */
  Time m_tStart;

  /**
   * \internal
   *
   * Time to start tearing down the device
   */
  Time m_tStop;

  /**
   * \internal
   *
   * The IP address to use as the device IP on the host.
   */
  Ipv6Address m_socketIp;

  /**
   * \internal
   *
   * The MAC address to use as the hardware address for the IPC process.
   * This value comes from the MAC address assigned to the bridged ns-3
   * net device. 
   */
  Mac64Address m_socketMac;

  /**
   * \internal
   *
   * The ns-3 net device to which we are bridging.
   */
  Ptr<NetDevice> m_bridgedDevice;

  /**
   * \internal
   *
   * The underlying ns-3 mac layer.
   */
  Ptr<SocketNullMac> m_macLayer;

  /**
   * \internal
   *
   * The underlying ns-3 PHY object.
   */
  Ptr<SocketContikiPhy> m_phy;

  /**
   * The PID of the child/Contiki process attached via the SocketBridge.
   */
  pid_t child;
 
  /**
   * A 64K buffer to hold packet data while it is being sent.
   */
  uint8_t *m_packetBuffer;
  /*
   * a copy of the node id so the read thread doesn't have to GetNode() in
   * in order to find the node ID.  Thread unsafe reference counting in 
   * multithreaded apps is not a good thing.
   */
  uint32_t m_nodeId;
  /*
   * Path to the executable that will be spawned from the fork/exec call 
   */
  std::string m_execPath;
  /*
   * Flag to denote that the layer 2 address was copied from the socket device
   * to the overlying container address. 
   */
  bool m_ns3AddressRewritten;

};

} // namespace ns3

#endif /* SOCKET_BRIDGE_H */
