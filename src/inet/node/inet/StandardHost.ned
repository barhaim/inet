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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.inet;

import inet.applications.contract.IApp;
import inet.common.PacketDispatcher;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;

//
// IPv4 host with SCTP, TCP, UDP layers and applications.
// IP forwarding is disabled by default (see forwarding).
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full-duplex connections are supported
//   only (twisted pair). Set **.eth[*].typename="EthernetInterface" for
//   a full/half-duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan[*].typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessNic
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext[*].typename in the INI file. see: ~ExtInterface and ~IExternalNic
// - If wireless card is present, node mobility can be set using **.mobilityType
//   see: inet.mobility and ~IMobility
//
module StandardHost extends NodeBase
{
    parameters:
        int numApps = default(0);
        bool hasTcp = default(true);
        bool hasUdp = default(true);
        bool hasSctp = default(false);
        bool hasTun = default(false);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailableOrEmpty("UDP"));
        string sctpType = default(firstAvailableOrEmpty("SCTP"));
        forwarding = default(false);  // disable routing by default
        networkLayer.proxyARP = default(false);
        ipv4.routingTableModule = default(absPath(".ipv4RoutingTable"));
        ipv6.routingTableModule = default(absPath(".ipv6RoutingTable"));
        gn.routingTableModule = default(absPath(".gnRoutingTable"));
        sctp.routingTableModule = default(absPath(".ipv4RoutingTable"));
        @display("i=device/pc2");
        @figure[applicationLayer](type=rectangle; pos=250,0; size=600,200; fillColor=#ffff00; cornerRadius=12; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=740,10; text="application layer");
        @figure[transportLayer](type=rectangle; pos=250,200; size=600,200; fillColor=#ff0000; cornerRadius=12; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=740,210; text="transport layer");
        @figure[submodules];

    submodules:
        app[numApps]: <> like IApp {
            parameters:
                @display("p=550,100,row,100");
        }
        at: PacketDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,200");
        }
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=400,300");
        }
        tcp: <tcpType> like ITCP if hasTcp {
            parameters:
                @display("p=500,300");
        }
        sctp: <sctpType> like ISCTP if hasSctp {
            parameters:
                @display("p=600,300");
        }
        tn: PacketDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,400");
        }
    connections allowunconnected:
        at.lowerLayerOut++ --> udp.appIn++ if hasUdp;
        at.lowerLayerIn++ <-- udp.appOut++ if hasUdp;

        at.lowerLayerOut++ --> tcp.appIn++ if hasTcp;
        at.lowerLayerIn++ <-- tcp.appOut++ if hasTcp;

        at.lowerLayerOut++ --> sctp.from_appl++ if hasSctp;
        at.lowerLayerIn++ <-- sctp.to_appl++ if hasSctp;

        at.lowerLayerOut++ --> tn.upperLayerIn++;
        at.lowerLayerIn++ <-- tn.upperLayerOut++;

        tn.lowerLayerOut++ --> ipv4.transportIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.transportOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv4.igmpIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.igmpOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv6.transportIn if hasIpv6;
        tn.lowerLayerIn++ <-- ipv6.transportOut if hasIpv6;

        tn.lowerLayerOut++ --> gn.transportIn if hasGn;
        tn.lowerLayerIn++ <-- gn.transportOut if hasGn;

        tn.lowerLayerOut++ --> nl.upperLayerIn++;
        tn.lowerLayerIn++ <-- nl.upperLayerOut++;

        for i=0..numApps-1 {
            app[i].socketOut --> at.upperLayerIn++;
            app[i].socketIn <-- at.upperLayerOut++;
        }

        udp.ipOut --> tn.upperLayerIn++ if hasUdp;
        udp.ipIn <-- tn.upperLayerOut++ if hasUdp;

        tcp.ipOut --> tn.upperLayerIn++ if hasTcp;
        tcp.ipIn <-- tn.upperLayerOut++ if hasTcp;

        sctp.to_ip --> tn.upperLayerIn++ if hasSctp;
        tn.upperLayerOut++ --> sctp.from_ip if hasSctp;
}
