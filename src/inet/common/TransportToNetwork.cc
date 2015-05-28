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

#include "inet/common/TransportToNetwork.h"
#include "inet/common/ProtocolCommand.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/transportlayer/contract/ITransportProtocolControlInfo.h"

namespace inet {

Define_Module(TransportToNetwork);

TransportToNetwork::TransportToNetwork()
{
}

int TransportToNetwork::computeSocketId(cMessage *message)
{
    ISocketControlInfo *controlInfo = dynamic_cast<ISocketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getSocketId() : -1;
}

std::pair<int, int> TransportToNetwork::computeLowerProtocol(cMessage *message)
{
    INetworkLayerControlInfo *networkLayerControlInfo = dynamic_cast<INetworkLayerControlInfo *>(message->getControlInfo());
    if (networkLayerControlInfo != nullptr)
        return std::pair<int, int>(NETWORK_LAYER_PROTOCOL, networkLayerControlInfo->getNetworkProtocol());
    ILinkLayerControlInfo *linkLayerControlInfo = dynamic_cast<ILinkLayerControlInfo *>(message->getControlInfo());
    if (linkLayerControlInfo != nullptr)
        return std::pair<int, int>(LINK_LAYER_PROTOCOL, linkLayerControlInfo->getMACProtocol());
    return std::pair<int, int>(-1, -1);
}

std::pair<int, int> TransportToNetwork::computeUpperProtocol(cMessage *message)
{
    INetworkProtocolControlInfo *networkProtocolControlInfo = dynamic_cast<INetworkProtocolControlInfo *>(message->getControlInfo());
    if (networkProtocolControlInfo != nullptr)
        return std::pair<int, int>(TRANSPORT_LAYER_PROTOCOL, networkProtocolControlInfo->getTransportProtocol());
    return std::pair<int, int>(-1, -1);
}

void TransportToNetwork::handleMessage(cMessage *message)
{
    if (message->arrivedOn("transportIn")) {
        if (message->isPacket()) {
            auto protocol = computeLowerProtocol(message);
            auto it = protocolToLowerGateIndex.find(protocol);
            if (it != protocolToLowerGateIndex.end())
                send(message, "networkOut", it->second);
            else
                throw cRuntimeError("Unknown protocol");
        }
        else {
            RegisterProtocolCommand *command = dynamic_cast<RegisterProtocolCommand *>(message);
            if (command != nullptr) {
                protocolToUpperGateIndex[std::pair<int, int>(command->getLayer(), command->getProtocol())] = message->getArrivalGate()->getIndex();
                int size = gateSize("networkOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "networkOut", i);
                delete message;
            }
            else {
                int socketId = computeSocketId(message);
                if (socketId != -1)
                    socketIdToTransportGateIndex[socketId] = message->getArrivalGate()->getIndex();
                auto protocol = computeLowerProtocol(message);
                auto it = protocolToLowerGateIndex.find(protocol);
                if (it != protocolToLowerGateIndex.end())
                    send(message, "networkOut", it->second);
                else
                    throw cRuntimeError("Unknown protocol");
            }
        }
    }
    else if (message->arrivedOn("networkIn")) {
        if (message->isPacket()) {
            int socketId = computeSocketId(message);
            if (socketId != -1) {
                auto it = socketIdToTransportGateIndex.find(socketId);
                if (it != socketIdToTransportGateIndex.end())
                    send(message, "transportOut", it->second);
                else
                    throw cRuntimeError("Unknown socket");
            }
            else {
                auto protocol = computeUpperProtocol(message);
                auto it = protocolToUpperGateIndex.find(protocol);
                if (it != protocolToUpperGateIndex.end())
                    send(message, "transportOut", it->second);
                else
                    throw cRuntimeError("Unknown protocol");
            }
        }
        else {
            RegisterProtocolCommand *command = dynamic_cast<RegisterProtocolCommand *>(message);
            if (command != nullptr) {
                protocolToLowerGateIndex[std::pair<int, int>(command->getLayer(), command->getProtocol())] = message->getArrivalGate()->getIndex();
                int size = gateSize("transportOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "transportOut", i);
                delete message;
            }
            else if (dynamic_cast<RegisterInterfaceCommand *>(message)) {
                int size = gateSize("transportOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "transportOut", i);
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

