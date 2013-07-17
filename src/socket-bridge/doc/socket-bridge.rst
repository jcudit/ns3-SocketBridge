SocketBridge Module Documentation
----------------------------

This model provides an alternative to external process communication with the ns-3 network simulator.  Much of the code within the tap-bridge model was reused to enable IPC between the ns-3 executable and an external process.

Model Description
*****************

The source code for the new model lives in the directory ``src/socket-bridge``.

Core Files
##########

The core source files are as follows:


``model/socket-bridge.cc``
The SocketBridge class is defined here.  The function of this class is to facilitate IPC between ns-3 and an external process.  This class performs a fork/exec for each configured instatiation, generates, assigns and passes a MAC address, and should perform optional translation between ns-3 layer 2 addresses and external process layer 2 addresses as traffic crosses the bridge.

``model/socket-phy.cc``
The SocketPhy class is defined here.  The function of this class is to comply with the ns-3 network stack by providing a PHY-layer.  This class defines inheritable virtual functions for application specific classes to define functionality for (see socket-contiki-phy.cc).  The design goal here is to have a specific PHY layer for each external process that communicates with ns-3 over IPC using this model.  This can come in handy for applications that implement different PHY protocols.

``model/socket-channel.cc``
The SocketChannel class is defined here.  The function of this class is to comply with the ns-3 network stack by providing a simulated channel.  This class is a bare bones version of the TapBridge implementation of a channel.  Required functions for sending and receiving to and from the channel are present as well as functions that support PropogationLossModel objects and PropogationDelayModel objects.

Supplementary Files
###################

Supplementary source files are as follows:

``model/interference-helper.cc``
The InterferenceHelper class is defined here.  The function of this class is to support the Receive() function within the SocketChannel.  This class is a derivative of the Wifi implementation of the InterferenceHelper.  As of this writing, interference does not play into the Tx or Rx processes and does not require compilation.

``model/socket-contiki-phy.cc``
The SocketContikiPhy class is defined here which is an application specific PHY-layer implementation.  The Contiki OS can be built as a standalone executable that occupies an IPC-enabled process.  This means that it may exist on the other side of the ns-3 IPC socket.  Contiki uses various PHY-layer protocols and thus requires a tailored ns-3 PHY implementation to align with the messages passed down from the external process towards the ns-3 channel.  Again, the aim here is to fulfill virtual functions defined in socket-phy.cc with specific implementations matching the supported protocols in the external process.

``model/socket-null-mac.cc``
The SocketNullMac class is defined here.  The function of this calss is to comply with the ns-3 network stack by providing a null-processing MAC layer so as to allow external processing of Layer 2 protocols.  The aim of this design is to allow Application Layer, Network Layer and MAC Layer data to be passed into ns-3 at the MAC Layer (NetDevice -> SocketBridge -> SocketNullMac -> SocketPHY -> SocketChannel).  Due to the configured operation mode (MACPHYOVERLAY vs. PHYOVERLAY), MAC-layer processing would have been performed in the external process.  However, to maintain a consistent network stack traversal, the SocketNullMac performs zero data manipulation and passes incoming data down the stack unmodified.

Design
======

The design of the model is similar to that of the tap-bridge model:

    +--------+
    |        |
    |        |                    +----------+
    |        |                    |   ghost  |
    |  apps  |                    |   node   |
    |        |                    | -------- |
    |        |                    |    IP    |     +----------+
    | ------ |                    |   stack  |     |   node   |
    | Linux  |                    |==========|     | -------- |
    |  Host  | <----- IPC ------> |  socket  |     |    IP    |
    +--------+                    |  bridge  |     |   stack  |
                                  | -------- |     | -------- |
                                  |   ns-3   |     |   ns-3   |
                                  |   net    |     |   net    |
                                  |  device  |     |  device  |
                                  +----------+     +----------+
                                       ||               ||
                                  +---------------------------+
                                  |        ns-3 channel       |
                                  +---------------------------+

Note that underneath the ns-3 NetDevice exists MAC, PHY and Channel objects to accomodate the ns-3 network stack framework.


Scope and Lvimitations
=====================

What can the model do?  What can it not do?  Please use this section to
describe the scope and limitations of the model.

At the time of this writing, the model can support external application to external application communication using ns-3 3.14.  Other versions of ns-3 have not been tested.  Applications other than the Contiki OS have not been tested.  Other limitations include:

- SNR and Packet Error Rate emulate ideal conditions (see SocketContikiPhy::EndReceive in socket-contiki-phy.cc)
- Interference is not part of the reception process for the PHY object; interference-helper.cc was not finished.
- External process to native ns-3 node communication does not work as the construction of an inbound packet does not occur (src, dst, type) in SocketBridge::Filter in socket-bridge.cc
- Incomplete documentation/coding style (i.e. license headers for source code, incomplete or copied doxygen tags, commented debug commands)
- There are no tests/validation

References
==========

See jeremyudit.blogspot.com for a complete explanation of the code and the project completed with the SocketBridge model.

Usage
*****

This model is used similarly to the TapBridge model whereby nodes are setup and configured normally and then branched from the NetDevice level into an external process.  This is performed by attaching a NetDevice and a SocketBridge, specifying the location of the external executable, and specifying the operation mode (MACPHYOVERLAY or PHYOVERLAY).  There are Helper classes available to automate the chaining of NetDevice, SocketBridge, SocketMAC, SocketPHY and SocketChannel objects.

Helpers
=======

There are several Helper classes provided to automate the chaining of the various components that correspond to different levels of the ns-3 stack.  They are outlined below:

``socket-bridge-helper.cc``
At the highest abstracted layer in terms of layer chaining is the SocketBridgeHelper class.  This class accepts a NodeContainer, the path to the external application, and the operating mode (MACPHYOVERLAY: ns-3 MAC and PHY protocols are active; PHYOVERLAY: ns-3 PHY protocols are active and the external program sends Layer 2 encapsulated data).  This Helper creates SocketBridge objects according to the number of nodes assigned to the NodeContainer.  For each SocketBridge created, an instance of the external process at the path specified is fork/exec'd.  Each SocketBridge object is connected to a NetDevice and each NetDevice is connected to a MAC object which is further connected to a PHY object.  All PHY objects are connected to a single channel.  Default PropogationDelayModel and PropogationLossModel values are chosen here where any ns-3 supported object could be used. 

``socket-channel-helper.cc``
This Helper simply connects existing PHY objects passed in as arguments to the Install function to an existing channel.  It is driven by the SocketBridgeHelper.

``socket-contiki-phy-helper.cc``
This Helper simply creates a new PHY object and connects it to an overlying MAC object. It is driven by the SocketBridgeHelper.  Note that this Helper constructs an object for a specific application but other general classes can be created using the same design.

``socket-null-mac-helper.cc``
This Helper simply creates a new MAC object and connects it to an overlying NetDevice object. It is driven by the SocketBridgeHelper.  Note that this Helper constructs an object for a specific application but other general classes can be created using the same design.

Advanced Usage
==============

Go into further details (such as using the API outside of the helpers)
in additional sections, as needed.

Examples
========

There are two examples found in the ``examples`` directory that illustrate external process to external process communication.

PING6 Example
#############
A simple demonstration of external process to external process communication via the ns-3 network stack and channel is accomplished with ``examples/socket-bridge-example.cc``.

Looking at the source file, the following lines of code tell ns-3 to operrate in real-time mode.  This mode effectively restricts the ns-3 scheduler from advancing faster than the hardware clock such that a simulated second is nearly equivalent to a real-time second.   

  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

Creating a pair of nodes to participate in the example is done similarly to other ns-3 Modules where the NodeContainer is used:

  NodeContainer nodes;
  nodes.Create(2);

Once the number of nodes have been specified, shaping the ns-3 stack is performed with the aid of the SocketBridgeHelper:

  SocketBridgeHelper socketBridgeHelper;
  socketBridgeHelper.Install(nodes, "/cn8801/contiki/examples/ns3-ping6/example-ping6.ns3", "PHYOVERLAY");

Finally, the simulator runtime is specified, executed and stopped with the final few lines of the example source file:

  Simulator::Stop (Seconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();

It is important to note that the Ping6 functionality actually lives in the Contiki application layer and ns-3 simply provides channel emulation as shown in the diagram below.  Changing the functionality of the nodes is simply a matter of changing the referenced path that is fed into the SocketBridgeHelper::Install function.

    +---------+                                                                     +---------+
    | Contiki |                                                                     | Contiki |
    |  Apps   |                    +----------+     +----------+                    |  Apps   |
    | ------- |                    |   ghost  |     |   ghos   |                    | ------- |
    | Contiki |                    |   node   |     |   node   |                    | Contiki |
    |   OS    |                    | -------- |     |----------|                    |   OS    |
    |         |                    |    IP    |     |    IP    |                    |         |
    | ------- |                    |   stack  |     |   stack  |                    | ------- |
    |  Linux  |                    |==========|     |==========|                    |  Linux  |
    |   Host  | <----- IPC ------> |  socket  |     |  socket  | <----- IPC ------> |  Host   |
    +---------+                    |  bridge  |     |  bridge  |                    +---------+ 
                                   | -------- |     | -------- |
                                   |   ns-3   |     |   ns-3   |
                                   |   net    |     |   net    |
                                   |  device  |     |  device  |
                                   +----------+     +----------+
                                        ||               ||
                                   +---------------------------+
                                   |        ns-3 channel       |
                                   +---------------------------+

Running the example is performed by navigating to the ns3 root directory (containing the ns3/ src/ and doc/ subdirectories among others) and running:

  ./waf --run socket-bridge-example 

Distributed ANN Example
#######################

Another example is found in the ``examples/socket-bridge-ann-example`` source file. The distributed ANN example differs from the Ping6 example in terms of scale alone.  Configuration for real-time mode and the simulator start/stop is the same.  The number of nodes however is a variable value:

  NodeContainer nodes;
  nodes.Create(70);

Again, the number of nodes used is configured using the NodeContainer class.  Because the nodes in the Contiki application will be performing a different application layer function, a different executable is specified in the SocketBridgeHelper::Install function:

  SocketBridgeHelper socketBridgeHelper;
  socketBridgeHelper.Install(nodes, "/cn8801/contiki/examples/ns3-ann/ns3-ann.ns3");

Running the example is performed by navigating to the ns3 root directory (containing the ns3/ src/ and doc/ subdirectories among others) and running:

  ./waf --run socket-bridge-ann-example 

Troubleshooting
===============

Add any tips for avoiding pitfalls, etc.
