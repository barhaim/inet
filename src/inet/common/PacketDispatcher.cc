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

#include "inet/common/IProtocolControlInfo.h"
#include "inet/common/PacketDispatcher.h"
#include "inet/common/ProtocolCommand.h"
#include "inet/applications/contract/ISocketControlInfo.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"

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

int PacketDispatcher::computeUpperLayerProtocolId(cMessage *message)
{
    IPacketProtocol *controlInfo = dynamic_cast<IPacketProtocol *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getPacketProtocolId() : -1;
}

int PacketDispatcher::computeLowerLayerProtocolId(cMessage *message)
{
    IControlInfoProtocol *controlInfo = dynamic_cast<IControlInfoProtocol *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getControlInfoProtocolId() : -1;
}

void PacketDispatcher::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        if (message->isPacket()) {
            int interfaceId = computeInterfaceId(message);
            int protocolId = computeLowerLayerProtocolId(message);
            if (interfaceId != -1) {
                auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
                if (it != interfaceIdToLowerLayerGateIndex.end())
                    send(message, "lowerLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown interface: id = %d", interfaceId);
            }
            else if (protocolId != -1) {
                auto it = protocolIdToLowerLayerGateIndex.find(protocolId);
                if (it != protocolIdToLowerLayerGateIndex.end())
                    send(message, "lowerLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown upper layer protocol: %d", protocolId);
            }
            else
                throw cRuntimeError("Unknown packet: %s", message->getName());
        }
        else {
            RegisterProtocolCommand *registerProtocolCommand = dynamic_cast<RegisterProtocolCommand *>(message);
            if (registerProtocolCommand != nullptr) {
                protocolIdToUpperLayerGateIndex[registerProtocolCommand->getProtocol()] = message->getArrivalGate()->getIndex();
                int size = gateSize("lowerLayerOut");
                for (int i = 0; i < size; i++)
                    send(message->dup(), "lowerLayerOut", i);
                delete message;
            }
            else {
                int socketId = computeSocketId(message);
                int interfaceId = computeInterfaceId(message);
                int protocolId = computeLowerLayerProtocolId(message);
                if (socketId != -1)
                    socketIdToUpperLayerGateIndex[socketId] = message->getArrivalGate()->getIndex();
                if (interfaceId != -1) {
                    auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
                    if (it != interfaceIdToLowerLayerGateIndex.end())
                        send(message, "lowerLayerOut", it->second);
                    else
                        throw cRuntimeError("Unknown interface: id = %d", interfaceId);
                }
                else if (protocolId != -1) {
                    auto it = protocolIdToLowerLayerGateIndex.find(protocolId);
                    if (it != protocolIdToLowerLayerGateIndex.end())
                        send(message, "lowerLayerOut", it->second);
                    else
                        throw cRuntimeError("Unknown lower layer protocol: %d", protocolId);
                }
                else
                    throw cRuntimeError("Unknown message: %s", message->getName());
            }
        }
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        if (message->isPacket()) {
            int socketId = computeSocketId(message);
            int protocolId = computeUpperLayerProtocolId(message);
            if (socketId != -1) {
                auto it = socketIdToUpperLayerGateIndex.find(socketId);
                if (it != socketIdToUpperLayerGateIndex.end())
                    send(message, "upperLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown socket, id = %d", socketId);
            }
            else if (protocolId != -1) {
                auto it = protocolIdToUpperLayerGateIndex.find(protocolId);
                if (it != protocolIdToUpperLayerGateIndex.end())
                    send(message, "upperLayerOut", it->second);
                else
                    throw cRuntimeError("Unknown upper layer protocol: %d", protocolId);
            }
            else
                throw cRuntimeError("Unknown packet: %s", message->getName());
        }
        else {
            RegisterProtocolCommand *registerProtocolCommand = dynamic_cast<RegisterProtocolCommand *>(message);
            RegisterInterfaceCommand *registerInterfaceCommand = dynamic_cast<RegisterInterfaceCommand *>(message);
            if (registerProtocolCommand != nullptr) {
                protocolIdToLowerLayerGateIndex[registerProtocolCommand->getProtocol()] = message->getArrivalGate()->getIndex();
                int size = gateSize("upperLayerOut");
                for (int i = 0; i < size; i++)
                    if (gate("upperLayerOut", i)->isPathOK())
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

