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

#ifndef __INET_PACKETDISPATCHER_H
#define __INET_PACKETDISPATCHER_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This module connects multiple protocols to multiple other protocols.
 *
 * Dispatch mechanism:
 *  - protocols must register by sending a RegisterProtocolCommand
 *  - packets must have a proper control info attached that specifies the protocol
 */
class INET_API PacketDispatcher : public cSimpleModule
{
    protected:
        std::map<int, int> socketIdToUpperLayerGateIndex;
        std::map<int, int> interfaceIdToLowerLayerGateIndex;
        std::map<int, int> protocolIdToUpperLayerGateIndex;
        std::map<int, int> protocolIdToLowerLayerGateIndex;

    protected:
        virtual void handleMessage(cMessage *message) override;

        virtual int computeSocketId(cMessage *message);
        virtual int computeInterfaceId(cMessage *message);
        virtual int computeUpperLayerProtocolId(cMessage *message);
        virtual int computeLowerLayerProtocolId(cMessage *message);

    public:
        PacketDispatcher();
};

} // namespace inet

#endif // ifndef __INET_PACKETDISPATCHER_H

