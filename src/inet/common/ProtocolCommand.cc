//
// Copyright (C) 2012 Opensim Ltd
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

#include "inet/common/ProtocolCommand.h"

namespace inet {

void registerProtocol(const Protocol& protocol, cGate *gate)
{
    //RegisterProtocolCommand *command = new RegisterProtocolCommand(TRANSPORT_LAYER_PROTOCOL, protocol.getId());
    //check_and_cast<cSimpleModule *>(gate->getOwner())->send(command, gate);
    cGate* pathEndGate = gate->getPathEndGate();
    IProtocolRegistration *protocolRegistration = dynamic_cast<IProtocolRegistration *>(pathEndGate->getOwner());
    if (protocolRegistration != nullptr)
        protocolRegistration->registerProtocol(protocol, pathEndGate);
}

void registerInterface(const InterfaceEntry& interface, cGate *gate)
{
    //RegisterInterfaceCommand *command = new RegisterInterfaceCommand(interface.getInterfaceId());
    //check_and_cast<cSimpleModule *>(gate->getOwner())->send(command, gate);
    cGate* pathEndGate = gate->getPathEndGate();
    IInterfaceRegistration *interfaceRegistration = dynamic_cast<IInterfaceRegistration *>(pathEndGate->getOwner());
    if (interfaceRegistration != nullptr)
        interfaceRegistration->registerInterface(interface, pathEndGate);
}

} // namespace inet

