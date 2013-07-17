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

#ifndef SOCKET_CONTIKI_PHY_HELPER_H
#define SOCKET_CONTIKI_PHY_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/socket-bridge.h"
#include <string>

namespace ns3 {

class Node;
class AttributeValue;

/**
 * \brief build SocketContikiPhy to allow ns-3 simulations to interact with Linux 
 * tap devices and processes on the Linux host.
 */
class SocketContikiPhyHelper
{
public:
  /**
   * Construct a SocketContikiPhyHelper to make life easier for people wanting to 
   * have their simulations interact with Linux tap devices and processes
   * on the Linux host.
   */
  SocketContikiPhyHelper ();

  /*
   * Set an attribute in the underlying SocketContikiPhy net device when these
   * devices are automatically created.
   *
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   */
  void SetAttribute (std::string n1, const AttributeValue &v1);

  /**
   * This method installs a SocketContikiPhy on the specified Node/MAC layer 
   *
   * \param node The Ptr<SocketBridge> to install the SocketContikiPhy in
   * \param mac The Ptr<SocketNullMac> to install the SocketContikiPhy in
   * \param mode The mode the PHY will operate under 
   * \returns A pointer to the new SocketContikiPhy NetDevice.
   */
  Ptr<SocketContikiPhy> Install (Ptr<SocketBridge> bridge, Ptr<SocketNullMac> mac, SocketContikiPhy::PhyMode mode);

private:
  ObjectFactory m_deviceFactory;
};

} // namespace ns3


#endif /* SOCKET_CONTIKI_PHY_HELPER_H */
