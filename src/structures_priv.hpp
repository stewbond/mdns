#ifndef _0684eb7668b8e333a0e6f8452a6c5355
#define _0684eb7668b8e333a0e6f8452a6c5355

// This header contains avahi-specific objects.
// Anything that needs <avahi-*/*.h> should not be exposed in public headers
// and can go here.

#include <mdns/structures.hpp>
#include <mdns/address.hpp>
#include <avahi-common/address.h> // AvahiProtocol, AvahiIfIndex
#include <avahi-common/defs.h> // AvahiLookupFlags
#include <avahi-client/client.h> // AvahiClientState

namespace mdns {
AvahiIfIndex to_avahi(IfIndex i);
// IfIndex from_avahi(AvahiIfIndex i); <-- AvahiIfIndex is not unique enough for this overload.  Use static_cast<IfIndex>() instead.


AvahiProtocol to_avahi(Protocol p);
Protocol from_avahi(AvahiProtocol p);

AvahiLookupFlags to_avahi(LookupFlags f);
LookupFlags from_avahi(AvahiLookupFlags f);

AvahiLookupResultFlags to_avahi(ResultFlags f);
ResultFlags from_avahi(AvahiLookupResultFlags f);

AvahiClientFlags to_avahi(ClientFlags f);
ClientFlags from_avahi(AvahiClientFlags f);

AvahiPublishFlags to_avahi(PublishFlags f);
PublishFlags from_avahi(AvahiPublishFlags f);

AvahiClientState to_avahi(ClientState s);
ClientState from_avahi(AvahiClientState s);

AvahiBrowserEvent to_avahi(BrowserEvent e);
BrowserEvent from_avahi(AvahiBrowserEvent e);

AvahiResolverEvent to_avahi(ResolverEvent e);
ResolverEvent from_avahi(AvahiResolverEvent e);

AvahiDomainBrowserType to_avahi(DomainBrowserType t);
DomainBrowserType from_avahi(AvahiDomainBrowserType t);

uint16_t to_avahi(DnsType t);
//DnsType from_avai(int); // Not unique enough for this overload.  Use static_cast<DnsType>() instead.

AvahiEntryGroupState to_avahi(GroupState s);
GroupState from_avahi(AvahiEntryGroupState s);

const char* to_avahi(const std::string& s);
std::string from_avahi(const char*);

AvahiStringList* to_avahi(const StrList&);
StrList from_avahi(AvahiStringList*);

AvahiIPv6Address to_avahi(const IPv6Address&);
IPv6Address from_avahi(const AvahiIPv6Address&);

AvahiIPv4Address to_avahi(const IPv4Address&);
IPv4Address from_avahi(const AvahiIPv4Address&);

AvahiAddress to_avahi(const Address&);
Address from_avahi(const AvahiAddress&);
Address from_avahi(const AvahiAddress*);
}
#endif
