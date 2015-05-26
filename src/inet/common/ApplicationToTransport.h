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

#ifndef __INET_APPLICATIONTOTRANSPORT_H
#define __INET_APPLICATIONTOTRANSPORT_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This module connects multiple applications to multiple transport protocols.
 *
 * Dispatch mechanism from applications to transport protocols:
 *  - applications must register sockets by sending an open socket command
 *  - packets must have an ITransportControlInfo attached that specifies the transport protocol
 *
 * Dispatch mechanism from transport protocols to applications:
 *  - transport protocols must register by sending a RegisterProtocolCommand
 *  - packets must have an ITransportControlInfor attached that specifies the application socket
 */
class INET_API ApplicationToTransport : public cSimpleModule
{
    protected:
        std::map<int, int> socketIdToApplicationGateIndex;
        std::map<std::pair<int, int>, int> protocolToTransportGateIndex;

    protected:
        virtual void handleMessage(cMessage *message) override;

        virtual int computeSocketId(cMessage *message);
        virtual std::pair<int, int> computeLowerProtocol(cMessage *message);

    public:
        ApplicationToTransport();
};

} // namespace inet

#endif // ifndef __INET_APPLICATIONTOTRANSPORT_H

