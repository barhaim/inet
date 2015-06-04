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

#ifndef __INET_PROTOCOLCOMMAND_H
#define __INET_PROTOCOLCOMMAND_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolCommand_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

// TODO: base class?
INET_API void registerProtocol(const Protocol& protocol, cGate *gate);

// TODO: base class?
INET_API void registerInterface(const InterfaceEntry& interface, cGate *gate);

// TODO: rename?
class INET_API IProtocolRegistration
{
  public:
    virtual void registerProtocol(const Protocol& protocol, cGate *gate) = 0;
};

// TODO: rename?
class INET_API IInterfaceRegistration
{
  public:
    virtual void registerInterface(const InterfaceEntry &interface, cGate *gate) = 0;
};

class INET_API RegisterProtocolCommand : public RegisterProtocolCommand_Base
{
  private:
    void copy(const RegisterProtocolCommand& other) { }

  public:
    RegisterProtocolCommand(const char *name=NULL, int kind=0) : RegisterProtocolCommand_Base(name, kind) { }
    RegisterProtocolCommand(const RegisterProtocolCommand& other) : RegisterProtocolCommand_Base(other) { copy(other); }
    RegisterProtocolCommand(int layer, int protocol) : RegisterProtocolCommand_Base(NULL, 0) { layer_var = layer; protocol_var = protocol; }
    RegisterProtocolCommand& operator=(const RegisterProtocolCommand& other) { if (this==&other) return *this; RegisterProtocolCommand_Base::operator=(other); copy(other); return *this; }
    virtual RegisterProtocolCommand *dup() const { return new RegisterProtocolCommand(*this); }
};

class INET_API RegisterInterfaceCommand : public RegisterInterfaceCommand_Base
{
  private:
    void copy(const RegisterInterfaceCommand& other) { }

  public:
    RegisterInterfaceCommand(const char *name=NULL, int kind=0) : RegisterInterfaceCommand_Base(name,kind) {}
    RegisterInterfaceCommand(const RegisterInterfaceCommand& other) : RegisterInterfaceCommand_Base(other) {copy(other);}
    RegisterInterfaceCommand(int interfaceId) : RegisterInterfaceCommand_Base(NULL, 0) { interfaceId_var = interfaceId; }
    RegisterInterfaceCommand& operator=(const RegisterInterfaceCommand& other) {if (this==&other) return *this; RegisterInterfaceCommand_Base::operator=(other); copy(other); return *this;}
    virtual RegisterInterfaceCommand *dup() const {return new RegisterInterfaceCommand(*this);}
};

} // namespace inet

#endif // ifndef __INET_PROTOCOLCOMMAND_H

