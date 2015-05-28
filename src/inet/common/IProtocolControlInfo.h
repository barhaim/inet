//
// Copyright (C) 2013 Opensim Ltd.
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

#ifndef __INET_IPROTOCOLCONTROLINFO_H
#define __INET_IPROTOCOLCONTROLINFO_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This control info specifies the communication protocol of the packet it is
 * attached to. Call Protocol::findProtocol() or Protocol::getProtocol() to
 * lookup the protocol based on the protocol id.
 */
class INET_API IPacketProtocol
{
  public:
    virtual ~IPacketProtocol() {}
    virtual int getPacketProtocolId() const = 0;
    virtual void setPacketProtocolId(int protocolId) = 0;
};

/**
 * This control info specifies the communication protocol of the control info
 * itself. Call Protocol::findProtocol() or Protocol::getProtocol() to lookup
 * the protocol based on the protocol id.
 */
class INET_API IControlInfoProtocol
{
  public:
    virtual ~IControlInfoProtocol() {}
    virtual int getControlInfoProtocolId() const = 0;
    virtual void setControlInfoProtocolId(int protocolId) = 0;
};

} // namespace inet

#endif // ifndef __INET_IPROTOCOLCONTROLINFO_H

