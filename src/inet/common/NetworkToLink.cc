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

#include "inet/common/NetworkToLink.h"
#include "inet/common/ProtocolCommand.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"

namespace inet {

Define_Module(NetworkToLink);

NetworkToLink::NetworkToLink()
{
}

int NetworkToLink::computeSocketId(cMessage *message)
{
    ISocketControlInfo *controlInfo = dynamic_cast<ISocketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getSocketId() : -1;
}

int NetworkToLink::computeInterfaceId(cMessage *message)
{
    IMACProtocolControlInfo *controlInfo = dynamic_cast<IMACProtocolControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getInterfaceId() : -1;
}

std::pair<int, int> NetworkToLink::computeUpperProtocol(cMessage *message)
{
    IMACProtocolControlInfo *controlInfo = dynamic_cast<IMACProtocolControlInfo *>(message->getControlInfo());
    if (controlInfo != nullptr)
        return std::pair<int, int>(NETWORK_LAYER_PROTOCOL, controlInfo->getNetworkProtocol());
    return std::pair<int, int>(-1, -1);
}

void NetworkToLink::handleMessage(cMessage *message)
{
    if (message->arrivedOn("networkIn")) {
        if (message->isPacket()) {
            int interfaceId = computeInterfaceId(message);
            auto it = interfaceIdToInterfaceGateIndex.find(interfaceId);
            if (it != interfaceIdToInterfaceGateIndex.end())
                send(message, "ifOut", it->second);
            else
                throw cRuntimeError("Unknown interface");
        }
        else {
            RegisterProtocolCommand *command = dynamic_cast<RegisterProtocolCommand *>(message);
            if (command != nullptr) {
                protocolToNetworkGateIndex[std::pair<int, int>(command->getLayer(), command->getProtocol())] = message->getArrivalGate()->getIndex();
                int size = gateSize("ifOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "ifOut", i);
                delete message;
            }
            else {
                int socketId = computeSocketId(message);
                if (socketId != -1)
                    socketIdToNetworkGateIndex[socketId] = message->getArrivalGate()->getIndex();
                int interfaceId = computeInterfaceId(message);
                auto it = interfaceIdToInterfaceGateIndex.find(interfaceId);
                if (it != interfaceIdToInterfaceGateIndex.end())
                    send(message, "ifOut", it->second);
                else
                    throw cRuntimeError("Unknown interface");
            }
        }
    }
    else if (message->arrivedOn("ifIn")) {
        if (message->isPacket()) {
            int socketId = computeSocketId(message);
            if (socketId != -1) {
                auto it = socketIdToNetworkGateIndex.find(socketId);
                if (it != socketIdToNetworkGateIndex.end())
                    send(message, "networkOut", it->second);
                else
                    throw cRuntimeError("Unknown socket");
            }
            else {
                auto protocol = computeUpperProtocol(message);
                auto it = protocolToNetworkGateIndex.find(protocol);
                if (it != protocolToNetworkGateIndex.end())
                    send(message, "networkOut", it->second);
                else
                    throw cRuntimeError("Unknown protocol");
            }
        }
        else {
            RegisterInterfaceCommand *command = dynamic_cast<RegisterInterfaceCommand *>(message);
            if (command != nullptr) {
                interfaceIdToInterfaceGateIndex[command->getInterfaceId()] = message->getArrivalGate()->getIndex();
                int size = gateSize("networkOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "networkOut", i);
                delete message;
            }
            else
                throw cRuntimeError("Unknown message");
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

