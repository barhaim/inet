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

#ifndef __INET_PROTOCOL_H
#define __INET_PROTOCOL_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Protocol
{
  protected:
    static int nextId;
    static std::map<int, const Protocol *> idToProtocol;

  protected:
    const int id;
    const char *name;

  public:
    Protocol(const char *name);

    const int getId() const { return id; }
    const char *getName() const { return name; }

    static const Protocol *findProtocol(int id);
    static const Protocol *getProtocol(int id);

  public:
    // in alphanumeric order
    static const Protocol arp;
    static const Protocol aodv;
    static const Protocol bgp;
    static const Protocol dsr;
    static const Protocol dymo;
    static const Protocol ethernet;
    static const Protocol icmpv4;
    static const Protocol icmpv6;
    static const Protocol ieee80211;
    static const Protocol igmp;
    static const Protocol ipv4;
    static const Protocol ipv6;
    static const Protocol ospf;
    static const Protocol tcp;
    static const Protocol udp;
};

} // namespace inet

#endif // ifndef __INET_PROTOCOL_H

