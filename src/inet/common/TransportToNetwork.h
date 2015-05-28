//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_TRANSPORTTONETWORK_H
#define __INET_TRANSPORTTONETWORK_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This module connects multiple transport protocols to multiple network protocols.
 *
 * Dispatch mechanism from transport protocols to network protocols:
 *  - network protocols must register by sending a RegisterProtocolCommand
 *  - packets must have an INetworkProtocolControlInfo attached that specifies the network protocol
 *
 * Dispatch mechanism from network protocols to transport protocols:
 *  - transport protocols must register by sending a RegisterProtocolCommand
 *  - packets must have an INetworkProtocolControlInfo attached that specifies the transport protocol
 */
class INET_API TransportToNetwork : public cSimpleModule
{
    protected:
        std::map<int, int> socketIdToTransportGateIndex;
        std::map<std::pair<int, int>, int> protocolToUpperGateIndex;
        std::map<std::pair<int, int>, int> protocolToLowerGateIndex;

    protected:
        virtual void handleMessage(cMessage *message) override;

        virtual int computeSocketId(cMessage *message);
        virtual std::pair<int, int> computeLowerProtocol(cMessage *message);
        virtual std::pair<int, int> computeUpperProtocol(cMessage *message);

    public:
        TransportToNetwork();
};

} // namespace inet

#endif // ifndef __INET_TRANSPORTTONETWORK_H

