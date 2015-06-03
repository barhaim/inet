//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.dymo;

import inet.node.inet.WirelessHost;
import inet.routing.dymo.IDYMO;


//
// TODO
//
module DYMORouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        string dymoType = default("DYMO");
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        dymo.routingTableModule = default(absPath(".ipv4RoutingTable"));
        dymo.networkProtocolModule = default(absPath(".ipv4.ip"));
    submodules:
        dymo: <dymoType> like IDYMO {
            @display("p=700,300");
        }
    connections:
        dymo.ipOut --> tn.upperLayerIn++;
        dymo.ipIn <-- tn.upperLayerOut++;
}
