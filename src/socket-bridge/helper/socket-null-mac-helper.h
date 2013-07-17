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

#ifndef SOCKET_NULL_MAC_HELPER_H
#define SOCKET_NULL_MAC_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/socket-bridge.h"
#include <string>

namespace ns3 {

class Node;
class AttributeValue;

/**
 * \brief build SocketNullMac to allow ns-3 simulations to interact with Linux 
 * tap devices and processes on the Linux host.
 */
class SocketNullMacHelper
{
public:
  /**
   * Construct a SocketNullMacHelper to make life easier for people wanting to 
   * have their simulations interact with Linux tap devices and processes
   * on the Linux host.
   */
  SocketNullMacHelper ();

  /*
   * Set an attribute in the underlying SocketNullMac net device when these
   * devices are automatically created.
   *
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   */
  void SetAttribute (std::string n1, const AttributeValue &v1);

  /**
   * This method installs a SocketNullMac on the specified SocketBridge
   *
   * \param socketBridge The Ptr<SocketBridge> to install the SocketNullMac in
   * \returns A pointer to the new SocketNullMac layer.
   */
  Ptr<SocketNullMac> Install (Ptr<SocketBridge> socketBridge);

private:
  ObjectFactory m_deviceFactory;
};

} // namespace ns3


#endif /* SOCKET_NULL_MAC_HELPER_H */
