//
// Copyright (C) 2012 Opensim Ltd
// Author: Levente Meszaros
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

#ifndef __INET_ITRANSPORTPROTOCOLCONTROLINFO_H
#define __INET_ITRANSPORTPROTOCOLCONTROLINFO_H

#include "inet/applications/contract/ISocketControlInfo.h"

namespace inet {

class INET_API ITransportLayerControlInfo
{
  public:
    virtual ~ITransportLayerControlInfo() {}
    virtual int getTransportProtocol() const = 0;
};

class INET_API ITransportProtocolControlInfo : public ITransportLayerControlInfo, public ISocketControlInfo
{
  public:
    virtual ~ITransportProtocolControlInfo() {}
};

} // namespace inet

#endif // ifndef __INET_ITRANSPORTPROTOCOLCONTROLINFO_H

