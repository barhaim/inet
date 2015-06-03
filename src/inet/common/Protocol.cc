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

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {

int Protocol::nextId = 0;
std::map<int, const Protocol *> Protocol::idToProtocol;

Protocol::Protocol(const char *name) :
    id(nextId++),
    name(name)
{
    idToProtocol[id] = this;
}

const Protocol *Protocol::findProtocol(int id)
{
    auto it = idToProtocol.find(id);
    return it != idToProtocol.end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(int id)
{
    const Protocol *protocol = findProtocol(id);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol id: %d" , id);
}

const Protocol Protocol::arp("arp");
const Protocol Protocol::aodv("aodv");
const Protocol Protocol::bgp("bgp");
const Protocol Protocol::dsr("dsr");
const Protocol Protocol::dymo("dymo");
const Protocol Protocol::ethernet("ethernet");
const Protocol Protocol::icmpv4("icmpv4");
const Protocol Protocol::icmpv6("icmpv6");
const Protocol Protocol::ieee80211("ieee80211");
const Protocol Protocol::igmp("igmp");
const Protocol Protocol::ipv4("ipv4");
const Protocol Protocol::ipv6("ipv6");
const Protocol Protocol::manet("manet");
const Protocol Protocol::ospf("ospf");
const Protocol Protocol::tcp("tcp");
const Protocol Protocol::udp("udp");

} // namespace inet

