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
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        string dymoType = default("DYMO");
    submodules:
        dymo: <dymoType> like IDYMO {
            @display("p=700,300");
        }
    connections:
        dymo.ipOut --> tn.upperLayerIn++;
        dymo.ipIn <-- tn.upperLayerOut++;
}
