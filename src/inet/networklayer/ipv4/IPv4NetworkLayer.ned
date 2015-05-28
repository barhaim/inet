//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.ipv4;

import inet.common.PacketDispatcher;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.contract.IARP;
import inet.networklayer.contract.INetworkLayer;


//
// Network layer of an IPv4 node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module IPv4NetworkLayer like INetworkLayer
{
    parameters:
        bool proxyARP = default(true);
        string igmpType = default("IGMPv2");
        string arpType = default("ARP");
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        arp.respondToProxyARP = proxyARP;
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);
        input transportIn @labels(IPv4ControlInfo/down);
        output transportOut @labels(IPv4ControlInfo/up);
        input igmpIn;
        output igmpOut;
        input arpIn;
        output arpOut;

    submodules:
        configurator: IPv4NodeConfigurator {
            parameters:
                @display("p=100,50");
        }
        igmp: <igmpType> like IIGMP {
            parameters:
                @display("p=200,100");
        }
        icmp: ICMP {
            parameters:
                @display("p=400,100");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=500,100");
        }
        dp: PacketDispatcher {
            parameters:
                @display("b=300,5,,,,1;p=300,200");
        }
        ip: IPv4 {
            parameters:
                useProxyARP = true; // as routes with unspecified next-hop addr are quite common
                @display("p=300,300;q=queue");
        }
        arp: <arpType> like IARP {
            parameters:
                @display("p=400,300;q=pendingQueue");
        }

    connections allowunconnected:
        dp.upperLayerOut++ --> { @display("m=n"); } --> transportOut;
        dp.upperLayerIn++ <-- { @display("m=n"); } <-- transportIn;

        dp.upperLayerOut++ --> icmp.ipIn;
        dp.upperLayerIn++ <-- icmp.ipOut;

        dp.upperLayerOut++ --> igmp.ipIn;
        dp.upperLayerIn++ <-- igmp.ipOut;

        icmp.errorOut --> errorHandling.in;

        ip.transportOut --> dp.lowerLayerIn++;
        ip.transportIn <-- dp.lowerLayerOut++;

        ip.arpOut --> arp.netwIn;
        ip.arpIn <-- arp.netwOut;

        arp.ifOut --> arpOut;
        arp.ifIn <-- arpIn;

        igmp.routerOut --> igmpOut;
        igmp.routerIn <-- igmpIn;

        ip.queueIn <-- { @display("m=s"); } <-- ifIn;
        ip.queueOut --> { @display("m=s"); } --> ifOut;
}

