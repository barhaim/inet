//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

cplusplus {{
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/applications/contract/ISocketControlInfo.h"
}}

namespace inet;

class IPSocketCommand
{
    @implements(INetworkLayerControlInfo, ISocketControlInfo);
    int networkProtocol = ETHERTYPE_IPv4;
    int socketId;
}

class IPSocketBindCommand extends IPSocketCommand
{
    int protoclId;
}

class IPSocketCloseCommand extends IPSocketCommand
{
}
