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

#include "inet/common/ApplicationToTransport.h"
#include "inet/common/ProtocolCommand.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/transportlayer/contract/ITransportProtocolControlInfo.h"

namespace inet {

Define_Module(ApplicationToTransport);

ApplicationToTransport::ApplicationToTransport()
{
}

int ApplicationToTransport::computeSocketId(cMessage *message)
{
    ISocketControlInfo *controlInfo = dynamic_cast<ISocketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getSocketId() : -1;
}

std::pair<int, int> ApplicationToTransport::computeLowerProtocol(cMessage *message)
{
    ITransportLayerControlInfo *transportLayerControlInfo = dynamic_cast<ITransportLayerControlInfo *>(message->getControlInfo());
    if (transportLayerControlInfo != nullptr)
        return std::pair<int, int>(TRANSPORT_LAYER_PROTOCOL, transportLayerControlInfo->getTransportProtocol());
    INetworkLayerControlInfo *networkLayerControlInfo = dynamic_cast<INetworkLayerControlInfo *>(message->getControlInfo());
    if (networkLayerControlInfo != nullptr)
        return std::pair<int, int>(NETWORK_LAYER_PROTOCOL, networkLayerControlInfo->getNetworkProtocol());
    ILinkLayerControlInfo *linkLayerControlInfo = dynamic_cast<ILinkLayerControlInfo *>(message->getControlInfo());
    if (linkLayerControlInfo != nullptr)
        return std::pair<int, int>(LINK_LAYER_PROTOCOL, linkLayerControlInfo->getMACProtocol());
    return std::pair<int, int>(-1, -1);
}

void ApplicationToTransport::handleMessage(cMessage *message)
{
    if (message->arrivedOn("applicationIn")) {
        if (message->isPacket()) {
            auto protocol = computeLowerProtocol(message);
            auto it = protocolToTransportGateIndex.find(protocol);
            if (it != protocolToTransportGateIndex.end())
                send(message, "transportOut", it->second);
            else
                throw cRuntimeError("Unknown protocol");
        }
        else {
            int socketId = computeSocketId(message);
            if (socketId != -1)
                socketIdToApplicationGateIndex[socketId] = message->getArrivalGate()->getIndex();
            auto protocol = computeLowerProtocol(message);
            auto it = protocolToTransportGateIndex.find(protocol);
            if (it != protocolToTransportGateIndex.end())
                send(message, "transportOut", it->second);
            else
                throw cRuntimeError("Unknown protocol");
        }
    }
    else if (message->arrivedOn("transportIn")) {
        if (message->isPacket()) {
            int socketId = computeSocketId(message);
            auto it = socketIdToApplicationGateIndex.find(socketId);
            if (it != socketIdToApplicationGateIndex.end())
                send(message, "applicationOut", it->second);
            else
                throw cRuntimeError("Unknown socket");
        }
        else {
            RegisterProtocolCommand *command = dynamic_cast<RegisterProtocolCommand *>(message);
            if (command != nullptr) {
                protocolToTransportGateIndex[std::pair<int, int>(command->getLayer(), command->getProtocol())] = message->getArrivalGate()->getIndex();
                delete message;
            }
            else if (dynamic_cast<RegisterInterfaceCommand *>(message))
                delete message;
            else
                throw cRuntimeError("Unknown message");
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

