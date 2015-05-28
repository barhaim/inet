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

#include "inet/common/PacketDispatcher.h"
#include "inet/common/ProtocolCommand.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/transportlayer/contract/ITransportProtocolControlInfo.h"

namespace inet {

Define_Module(PacketDispatcher);

PacketDispatcher::PacketDispatcher()
{
}

int PacketDispatcher::computeSocketId(cMessage *message)
{
    ISocketControlInfo *controlInfo = dynamic_cast<ISocketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getSocketId() : -1;
}

int PacketDispatcher::computeInterfaceId(cMessage *message)
{
    IMACProtocolControlInfo *controlInfo = dynamic_cast<IMACProtocolControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getInterfaceId() : -1;
}

std::pair<int, int> PacketDispatcher::computeUpperLayerProtocolId(cMessage *message)
{
    IMACProtocolControlInfo *controlInfo = dynamic_cast<IMACProtocolControlInfo *>(message->getControlInfo());
    if (controlInfo != nullptr)
        return std::pair<int, int>(NETWORK_LAYER_PROTOCOL, controlInfo->getNetworkProtocol());
    INetworkProtocolControlInfo *networkProtocolControlInfo = dynamic_cast<INetworkProtocolControlInfo *>(message->getControlInfo());
    if (networkProtocolControlInfo != nullptr)
        return std::pair<int, int>(TRANSPORT_LAYER_PROTOCOL, networkProtocolControlInfo->getTransportProtocol());
    return std::pair<int, int>(-1, -1);
}

std::pair<int, int> PacketDispatcher::computeLowerLayerProtocolId(cMessage *message)
{
    ILinkLayerControlInfo *linkLayerControlInfo = dynamic_cast<ILinkLayerControlInfo *>(message->getControlInfo());
    if (linkLayerControlInfo != nullptr)
        return std::pair<int, int>(LINK_LAYER_PROTOCOL, linkLayerControlInfo->getMACProtocol());
    INetworkLayerControlInfo *networkLayerControlInfo = dynamic_cast<INetworkLayerControlInfo *>(message->getControlInfo());
    if (networkLayerControlInfo != nullptr)
        return std::pair<int, int>(NETWORK_LAYER_PROTOCOL, networkLayerControlInfo->getNetworkProtocol());
    ITransportLayerControlInfo *transportLayerControlInfo = dynamic_cast<ITransportLayerControlInfo *>(message->getControlInfo());
    if (transportLayerControlInfo != nullptr)
        return std::pair<int, int>(TRANSPORT_LAYER_PROTOCOL, transportLayerControlInfo->getTransportProtocol());
    return std::pair<int, int>(-1, -1);
}

void PacketDispatcher::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        if (message->isPacket()) {
            int interfaceId = computeInterfaceId(message);
            auto protocol = computeLowerLayerProtocolId(message);
            if (interfaceId != -1) {
                auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
                if (it != interfaceIdToLowerLayerGateIndex.end())
                    send(message, "lowerLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown interface: id = %d", interfaceId);
            }
            else if (protocol.first != -1 && protocol.second != -1) {
                auto it = protocolIdToLowerLayerGateIndex.find(protocol);
                if (it != protocolIdToLowerLayerGateIndex.end())
                    send(message, "lowerLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown upper layer protocol: OSI layer = %d, id = %d", protocol.first, protocol.second);
            }
            else
                throw cRuntimeError("Unknown packet: %s", message->getName());
        }
        else {
            RegisterProtocolCommand *registerProtocolCommand = dynamic_cast<RegisterProtocolCommand *>(message);
            if (registerProtocolCommand != nullptr) {
                protocolIdToUpperLayerGateIndex[std::pair<int, int>(registerProtocolCommand->getLayer(), registerProtocolCommand->getProtocol())] = message->getArrivalGate()->getIndex();
                int size = gateSize("lowerLayerOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "lowerLayerOut", i);
                delete message;
            }
            else {
                int socketId = computeSocketId(message);
                int interfaceId = computeInterfaceId(message);
                auto protocol = computeLowerLayerProtocolId(message);
                if (socketId != -1)
                    socketIdToUpperLayerGateIndex[socketId] = message->getArrivalGate()->getIndex();
                if (interfaceId != -1) {
                    auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
                    if (it != interfaceIdToLowerLayerGateIndex.end())
                        send(message, "lowerLayerOut", it->second);
                    else
                        throw cRuntimeError("Unknown interface: id = %d", interfaceId);
                }
                else if (protocol.first != -1 && protocol.second != -1) {
                    auto it = protocolIdToLowerLayerGateIndex.find(protocol);
                    if (it != protocolIdToLowerLayerGateIndex.end())
                        send(message, "lowerLayerOut", it->second);
                    else
                        throw cRuntimeError("Unknown lower layer protocol: OSI layer = %d, id = %d", protocol.first, protocol.second);
                }
                else
                    throw cRuntimeError("Unknown message: %s", message->getName());
            }
        }
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        if (message->isPacket()) {
            int socketId = computeSocketId(message);
            auto protocol = computeUpperLayerProtocolId(message);
            if (socketId != -1) {
                auto it = socketIdToUpperLayerGateIndex.find(socketId);
                if (it != socketIdToUpperLayerGateIndex.end())
                    send(message, "upperLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown socket, id = %d", socketId);
            }
            else if (protocol.first != -1 && protocol.second != -1) {
                auto it = protocolIdToUpperLayerGateIndex.find(protocol);
                if (it != protocolIdToUpperLayerGateIndex.end())
                    send(message, "upperLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown upper layer protocol: OSI layer = %d, id = %d", protocol.first, protocol.second);
            }
            else
                throw cRuntimeError("Unknown packet: %s", message->getName());
        }
        else {
            RegisterProtocolCommand *registerProtocolCommand = dynamic_cast<RegisterProtocolCommand *>(message);
            RegisterInterfaceCommand *registerInterfaceCommand = dynamic_cast<RegisterInterfaceCommand *>(message);
            if (registerProtocolCommand != nullptr) {
                protocolIdToLowerLayerGateIndex[std::pair<int, int>(registerProtocolCommand->getLayer(), registerProtocolCommand->getProtocol())] = message->getArrivalGate()->getIndex();
                int size = gateSize("upperLayerOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "upperLayerOut", i);
                delete message;
            }
            else if (registerInterfaceCommand != nullptr) {
                interfaceIdToLowerLayerGateIndex[registerInterfaceCommand->getInterfaceId()] = message->getArrivalGate()->getIndex();
                int size = gateSize("upperLayerOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "upperLayerOut", i);
                delete message;
            }
            else
                throw cRuntimeError("Unknown message: %s", message->getName());
        }
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getName());
}

} // namespace inet

