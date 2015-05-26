//
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

package inet.node.inet;

import inet.common.NetworkToLink;
import inet.common.lifecycle.NodeStatus;
import inet.common.packet.PcapRecorder;
import inet.linklayer.contract.IExternalNic;
import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.generic.GenericNetworkLayer;
import inet.networklayer.generic.GenericRoutingTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.ipv6.IPv6NetworkLayer;
import inet.networklayer.ipv6.IPv6RoutingTable;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyStorage;


//
// Contains the common lower layers (linklayer and networklayer)
// of Router, StandardHost, WirelessHost etc.
//
// NOTE:
//    - forwarding is turned on by default.
//    - use this model only as a base type. In simulations use ~Router,
//      ~StandardHost, ~WirelessHost and ~AdhocHost
//
module NodeBase like INode
{
    parameters:
        @node;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        bool hasPcapRecorder = default(false);
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);
        bool hasLo = default(true);
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numTunInterfaces = default(0);
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string routingTableType = default("IPv4RoutingTable");
        bool forwarding = default(true);
        bool multicastForwarding = default(false);
        string energyStorageType = default("");
        string energyGeneratorType = default("");
        ipv4RoutingTable.forwarding = forwarding;
        ipv6RoutingTable.forwarding = forwarding;
        gnRoutingTable.forwarding = forwarding;
        ipv4RoutingTable.multicastForwarding = multicastForwarding;
        ipv6RoutingTable.multicastForwarding = multicastForwarding;
        gnRoutingTable.multicastForwarding = multicastForwarding;
        ipv4.routingTableModule = default(absPath(".ipv4RoutingTable"));
        ipv6.routingTableModule = default(absPath(".ipv6RoutingTable"));
        gn.routingTableModule = default(absPath(".gnRoutingTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");
        @figure[networkLayer](type=rectangle; pos=250,400; size=600,200; fillColor=#00ff00; cornerRadius=12; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=740,410; text="network layer");
        @figure[linkLayer](type=rectangle; pos=250,600; size=600,200; fillColor=#0000ff; cornerRadius=12; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=740,610; text="link layer");
        @figure[submodules];
    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            parameters:
                @display("p=100,50;is=s");
        }
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=100,150;is=s");
        }
        pcapRecorder: PcapRecorder if hasPcapRecorder {
            parameters:
                @display("p=100,250,r,10;is=s");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=100,350;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=100,450;i=block/plug;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,550;is=s");
        }
        ipv4RoutingTable: IPv4RoutingTable if hasIpv4 {
            parameters:
                @display("p=100,650;is=s");
        }
        ipv6RoutingTable: IPv6RoutingTable if hasIpv6 {
            parameters:
                @display("p=100,750;is=s");
        }
        gnRoutingTable: GenericRoutingTable if hasGn {
            parameters:
                @display("p=100,850;is=s");
        }
        ipv4: IPv4NetworkLayer if hasIpv4 {
            parameters:
                @display("p=400,500;q=queue");
        }
        ipv6: IPv6NetworkLayer if hasIpv6 {
            parameters:
                @display("p=500,500;q=queue");
        }
        gn: GenericNetworkLayer if hasGn {
            parameters:
                @display("p=600,500;q=queue");
        }
        nl: NetworkToLink {
            parameters:
                //                @display("i=misc/node;is=vs;p=550,600");
                @display("b=580,5,,,,1;p=550,600");
        }
        lo0: LoopbackInterface if hasLo {
            parameters:
                @display("p=300,700");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=400,700,row,60;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=500,700,row,60;q=txQueue");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=600,700,row,60;q=queue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=700,700,row,10;q=txQueue;i=block/ifcard");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=800,700,row,10;q=txQueue;i=block/ifcard");
        }

    connections allowunconnected:
        nl.ifOut++ --> lo0.upperLayerIn if hasLo;
        lo0.upperLayerOut --> nl.ifIn++ if hasLo;

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> nl.ifIn++;
            wlan[i].upperLayerIn <-- nl.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
            eth[i].upperLayerOut --> nl.ifIn++;
            eth[i].upperLayerIn <-- nl.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> nl.ifIn++;
            ppp[i].upperLayerIn <-- nl.ifOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> nl.ifIn++;
            ext[i].upperLayerIn <-- nl.ifOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> nl.ifIn++;
            tun[i].upperLayerIn <-- nl.ifOut++;
        }

        ipv4.ifIn++ <-- nl.networkOut++ if hasIpv4;
        ipv4.ifOut++ --> nl.networkIn++ if hasIpv4;

        ipv4.arpIn <-- nl.networkOut++ if hasIpv4;
        ipv4.arpOut --> nl.networkIn++ if hasIpv4;

        ipv6.ifIn++ <-- nl.networkOut++ if hasIpv6;
        ipv6.ifOut++ --> nl.networkIn++ if hasIpv6;

        gn.ifIn++ <-- nl.networkOut++ if hasGn;
        gn.ifOut++ --> nl.networkIn++ if hasGn;
}

