#include <mdns/structures.hpp>
#include "structures_priv.hpp"
#include <mdns/exception.hpp>

#include <avahi-common/address.h> // AvahiAddress, AvahiIfIndex, AvahiProtocol
#include <avahi-client/client.h>
#include <arpa/inet.h>
#include <ostream>
namespace mdns {

// We're trying to avoid including <avahi*> in our public interface.
// So we're making assumptions about Avahi's underlying types in public,
// and then static_asserting those assumptions privately
static_assert(
    std::is_same_v<IfIndex::Type,AvahiIfIndex>,
    "Type must be equivalent to AvahiIfIndex"
);

static_assert(
    IfIndex::any().value() == AVAHI_IF_UNSPEC,
    "If this breaks, then we need to change how 'any' gets casted"
);

AvahiIfIndex to_avahi(IfIndex i)
{
    return i.value();
}

std::ostream& operator<<(std::ostream& o, const IfIndex& i)
{
    if (i == IfIndex::any())
        return o << "any";
    return o << i.value();
}

AvahiProtocol to_avahi(Protocol p)
{
    switch(p)
    {
    case Protocol::ipv4  : return AVAHI_PROTO_INET  ;
    case Protocol::ipv6  : return AVAHI_PROTO_INET6 ;
    case Protocol::unspec: return AVAHI_PROTO_UNSPEC;
    default              : return AVAHI_PROTO_UNSPEC;
    }

}
Protocol from_avahi(AvahiProtocol p)
{
    switch(p)
    {
    case AVAHI_PROTO_INET  : return Protocol::ipv4  ;
    case AVAHI_PROTO_INET6 : return Protocol::ipv6  ;
    case AVAHI_PROTO_UNSPEC: return Protocol::unspec;
    default: throw Exception(Error::bad_protocol);
    }
}

std::ostream& operator<<(std::ostream& o, const Protocol& p)
{
    // o << avahi_proto_to_string(to_avahi(p))
    switch(p)
    {
    case Protocol::ipv4  : return o << "IPv4"  ;
    case Protocol::ipv6  : return o << "IPv6"  ;
    case Protocol::unspec: return o << "Unspec";
    }
    return o;
}


LookupFlags operator|(LookupFlags lhs, LookupFlags rhs)
{
    return static_cast<LookupFlags>(
        static_cast<int>(lhs) | static_cast<int>(rhs)
    );
}

LookupFlags operator&(LookupFlags lhs, LookupFlags rhs)
{
    return static_cast<LookupFlags>(
        static_cast<int>(lhs) & static_cast<int>(rhs)
    );
}

LookupFlags operator^(LookupFlags lhs, LookupFlags rhs)
{
    return static_cast<LookupFlags>(
        static_cast<int>(lhs) ^ static_cast<int>(rhs)
    );
}

LookupFlags operator~(LookupFlags f)
{
    return static_cast<LookupFlags>( ~static_cast<int>(f) );
}

LookupFlags& operator|=(LookupFlags& lhs, LookupFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

LookupFlags& operator&=(LookupFlags& lhs, LookupFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

LookupFlags& operator^=(LookupFlags& lhs, LookupFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& os, LookupFlags f)
{
    if (f == LookupFlags::none)
        return os << "none";

    bool first = true;
    auto print_flag = [&](LookupFlags flag, const char* name)
    {
        if ((f & flag) != LookupFlags::none)
        {
            if (!first) os << " | ";
            os << name;
            first = false;
        }
    };

    print_flag(LookupFlags::use_wide_area, "use_wide_area");
    print_flag(LookupFlags::use_multicast, "use_multicast");
    print_flag(LookupFlags::no_txt,        "no_txt"       );
    print_flag(LookupFlags::no_address,    "no_address"   );

    return os;
}
AvahiLookupFlags to_avahi(LookupFlags f)
{
    return static_cast<AvahiLookupFlags>(static_cast<int>(f));
}
LookupFlags from_avahi(AvahiLookupFlags f)
{
    return static_cast<LookupFlags>(static_cast<int>(f));
}

static_assert(
    static_cast<int>(LookupFlags::use_wide_area) ==
    static_cast<int>(AVAHI_LOOKUP_USE_WIDE_AREA) &&
    static_cast<int>(LookupFlags::use_multicast) ==
    static_cast<int>(AVAHI_LOOKUP_USE_MULTICAST) &&
    static_cast<int>(LookupFlags::no_txt       ) ==
    static_cast<int>(AVAHI_LOOKUP_NO_TXT       ) &&
    static_cast<int>(LookupFlags::no_address   ) ==
    static_cast<int>(AVAHI_LOOKUP_NO_ADDRESS   )
);

ResultFlags operator|(ResultFlags lhs, ResultFlags rhs)
{
    return static_cast<ResultFlags>(
        static_cast<int>(lhs) | static_cast<int>(rhs)
    );
}

ResultFlags operator&(ResultFlags lhs, ResultFlags rhs)
{
    return static_cast<ResultFlags>(
        static_cast<int>(lhs) & static_cast<int>(rhs)
    );
}

ResultFlags operator^(ResultFlags lhs, ResultFlags rhs)
{
    return static_cast<ResultFlags>(
        static_cast<int>(lhs) ^ static_cast<int>(rhs)
    );
}

ResultFlags operator~(ResultFlags f)
{
    return static_cast<ResultFlags>( ~static_cast<int>(f) );
}

ResultFlags& operator|=(ResultFlags& lhs, ResultFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

ResultFlags& operator&=(ResultFlags& lhs, ResultFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

ResultFlags& operator^=(ResultFlags& lhs, ResultFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& os, ResultFlags f)
{
    if (f == ResultFlags::none)
        return os << "none";

    bool first = true;
    auto print_flag = [&](ResultFlags flag, const char* name)
    {
        if ((f & flag) != ResultFlags::none)
        {
            if (!first) os << " | ";
            os << name;
            first = false;
        }
    };

    print_flag(ResultFlags::cached,    "cached"   );
    print_flag(ResultFlags::wide_area, "wide_area");
    print_flag(ResultFlags::multicast, "multicast");
    print_flag(ResultFlags::local,     "local"    );
    print_flag(ResultFlags::our_own,   "our_own"  );
    print_flag(ResultFlags::static_,   "static"   );

    return os;
}
AvahiLookupResultFlags to_avahi(ResultFlags f)
{
    return static_cast<AvahiLookupResultFlags>(static_cast<int>(f));
}
ResultFlags from_avahi(AvahiLookupResultFlags f)
{
    return static_cast<ResultFlags>(static_cast<int>(f));
}

static_assert(
    static_cast<int>(ResultFlags::cached          ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_CACHED   ) &&
    static_cast<int>(ResultFlags::wide_area       ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_WIDE_AREA) &&
    static_cast<int>(ResultFlags::multicast       ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_MULTICAST) &&
    static_cast<int>(ResultFlags::local           ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_LOCAL    ) &&
    static_cast<int>(ResultFlags::our_own         ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_OUR_OWN  ) &&
    static_cast<int>(ResultFlags::static_         ) ==
    static_cast<int>(AVAHI_LOOKUP_RESULT_STATIC   )
);


ClientFlags operator|(ClientFlags lhs, ClientFlags rhs)
{
    return static_cast<ClientFlags>(
        static_cast<int>(lhs) | static_cast<int>(rhs)
    );
}

ClientFlags operator&(ClientFlags lhs, ClientFlags rhs)
{
    return static_cast<ClientFlags>(
        static_cast<int>(lhs) & static_cast<int>(rhs)
    );
}

ClientFlags operator^(ClientFlags lhs, ClientFlags rhs)
{
    return static_cast<ClientFlags>(
        static_cast<int>(lhs) ^ static_cast<int>(rhs)
    );
}

ClientFlags operator~(ClientFlags f)
{
    return static_cast<ClientFlags>( ~static_cast<int>(f) );
}

ClientFlags& operator|=(ClientFlags& lhs, ClientFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

ClientFlags& operator&=(ClientFlags& lhs, ClientFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

ClientFlags& operator^=(ClientFlags& lhs, ClientFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& os, ClientFlags f)
{
    if (f == ClientFlags::none)
        return os << "none";

    bool first = true;
    auto print_flag = [&](ClientFlags flag, const char* name)
    {
        if ((f & flag) != ClientFlags::none)
        {
            if (!first) os << " | ";
            os << name;
            first = false;
        }
    };

    print_flag(ClientFlags::ignore_user_config, "ignore_user_config" );
    print_flag(ClientFlags::no_fail           , "no_fail"            );

    return os;
}

AvahiClientFlags to_avahi(ClientFlags f)
{
    return static_cast<AvahiClientFlags>(static_cast<int>(f));
}
ClientFlags from_avahi(AvahiClientFlags f)
{
    return static_cast<ClientFlags>(static_cast<int>(f));
}

static_assert(
    static_cast<int>(ClientFlags::ignore_user_config) ==
    static_cast<int>(AVAHI_CLIENT_IGNORE_USER_CONFIG) &&
    static_cast<int>(ClientFlags::no_fail           ) ==
    static_cast<int>(AVAHI_CLIENT_NO_FAIL           )
);

PublishFlags operator|(PublishFlags lhs, PublishFlags rhs)
{
    return static_cast<PublishFlags>(
        static_cast<int>(lhs) | static_cast<int>(rhs)
        );
}

PublishFlags operator&(PublishFlags lhs, PublishFlags rhs)
{
    return static_cast<PublishFlags>(
        static_cast<int>(lhs) & static_cast<int>(rhs)
        );
}

PublishFlags operator^(PublishFlags lhs, PublishFlags rhs)
{
    return static_cast<PublishFlags>(
        static_cast<int>(lhs) ^ static_cast<int>(rhs)
        );
}

PublishFlags operator~(PublishFlags f)
{
    return static_cast<PublishFlags>( ~static_cast<int>(f) );
}

PublishFlags& operator|=(PublishFlags& lhs, PublishFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

PublishFlags& operator&=(PublishFlags& lhs, PublishFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

PublishFlags& operator^=(PublishFlags& lhs, PublishFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& os, PublishFlags f)
{
    if (f == PublishFlags::none)
        return os << "none";

    bool first = true;
    auto print_flag = [&](PublishFlags flag, const char* name)
    {
        if ((f & flag) != PublishFlags::none)
        {
            if (!first) os << " | ";
            os << name;
            first = false;
        }
    };

    print_flag(PublishFlags::unique        , "unique"        );
    print_flag(PublishFlags::no_probe      , "no_probe"      );
    print_flag(PublishFlags::no_announce   , "no_announce"   );
    print_flag(PublishFlags::allow_multiple, "allow_multiple");
    print_flag(PublishFlags::no_reverse    , "no_reverse"    );
    print_flag(PublishFlags::no_cookie     , "no_cookie"     );
    print_flag(PublishFlags::update        , "update"        );
    print_flag(PublishFlags::use_wide_area , "use_wide_area" );
    print_flag(PublishFlags::use_multicast , "use_multicast" );
    return os;
}

AvahiPublishFlags to_avahi(PublishFlags f)
{
    return static_cast<AvahiPublishFlags>(static_cast<int>(f));
}
PublishFlags from_avahi(AvahiPublishFlags f)
{
    return static_cast<PublishFlags>(static_cast<int>(f));
}

static_assert(
    static_cast<int>(PublishFlags::unique        ) ==
    static_cast<int>(AVAHI_PUBLISH_UNIQUE        ) &&
    static_cast<int>(PublishFlags::no_probe      ) ==
    static_cast<int>(AVAHI_PUBLISH_NO_PROBE      ) &&
    static_cast<int>(PublishFlags::no_announce   ) ==
    static_cast<int>(AVAHI_PUBLISH_NO_ANNOUNCE   ) &&
    static_cast<int>(PublishFlags::allow_multiple) ==
    static_cast<int>(AVAHI_PUBLISH_ALLOW_MULTIPLE) &&
    static_cast<int>(PublishFlags::no_reverse    ) ==
    static_cast<int>(AVAHI_PUBLISH_NO_REVERSE    ) &&
    static_cast<int>(PublishFlags::no_cookie     ) ==
    static_cast<int>(AVAHI_PUBLISH_NO_COOKIE     ) &&
    static_cast<int>(PublishFlags::update        ) ==
    static_cast<int>(AVAHI_PUBLISH_UPDATE        ) &&
    static_cast<int>(PublishFlags::use_wide_area ) ==
    static_cast<int>(AVAHI_PUBLISH_USE_WIDE_AREA ) &&
    static_cast<int>(PublishFlags::use_multicast ) ==
    static_cast<int>(AVAHI_PUBLISH_USE_MULTICAST )
);

std::ostream& operator<<(std::ostream& o, const ClientState& s)
{
    switch(s)
    {
    case ClientState::registering: return o << "registering";
    case ClientState::running    : return o << "running"    ;
    case ClientState::collision  : return o << "collision"  ;
    case ClientState::failure    : return o << "failure"    ;
    case ClientState::connecting : return o << "connecting" ;
    default:                       return o << "?"                         ;
    }
}

AvahiClientState to_avahi(ClientState s)
{
    switch(s)
    {
    case ClientState::registering: return AVAHI_CLIENT_S_REGISTERING;
    case ClientState::running    : return AVAHI_CLIENT_S_RUNNING    ;
    case ClientState::collision  : return AVAHI_CLIENT_S_COLLISION  ;
    case ClientState::failure    : return AVAHI_CLIENT_FAILURE      ;
    case ClientState::connecting : return AVAHI_CLIENT_CONNECTING   ;
    default:                       return AVAHI_CLIENT_FAILURE;
    }
}

ClientState from_avahi(AvahiClientState s)
{
    switch(s)
    {
    case AVAHI_CLIENT_S_REGISTERING: return ClientState::registering;
    case AVAHI_CLIENT_S_RUNNING    : return ClientState::running    ;
    case AVAHI_CLIENT_S_COLLISION  : return ClientState::collision  ;
    case AVAHI_CLIENT_FAILURE      : return ClientState::failure    ;
    case AVAHI_CLIENT_CONNECTING   : return ClientState::connecting ;
    default: throw Exception(Error::bad_client_state);
    }
}

AvahiBrowserEvent to_avahi(BrowserEvent p)
{
    switch(p)
    {
    case BrowserEvent::new_           : return AVAHI_BROWSER_NEW            ;
    case BrowserEvent::remove         : return AVAHI_BROWSER_REMOVE         ;
    case BrowserEvent::cache_exhausted: return AVAHI_BROWSER_CACHE_EXHAUSTED;
    case BrowserEvent::all_for_now    : return AVAHI_BROWSER_ALL_FOR_NOW    ;
    case BrowserEvent::failure        : return AVAHI_BROWSER_FAILURE        ;
    default                           : return AVAHI_BROWSER_FAILURE        ;
    }

}
BrowserEvent from_avahi(AvahiBrowserEvent p)
{
    switch(p)
    {
    case AVAHI_BROWSER_NEW            : return BrowserEvent::new_           ;
    case AVAHI_BROWSER_REMOVE         : return BrowserEvent::remove         ;
    case AVAHI_BROWSER_CACHE_EXHAUSTED: return BrowserEvent::cache_exhausted;
    case AVAHI_BROWSER_ALL_FOR_NOW    : return BrowserEvent::all_for_now    ;
    case AVAHI_BROWSER_FAILURE        : return BrowserEvent::failure        ;
    default: throw Exception(Error::bad_browser_event)            ;
    }
}

std::ostream& operator<<(std::ostream& o, const BrowserEvent& p)
{
    switch(p)
    {
    case BrowserEvent::new_           : return o << "new"            ;
    case BrowserEvent::remove         : return o << "remove"         ;
    case BrowserEvent::cache_exhausted: return o << "cache_exhausted";
    case BrowserEvent::all_for_now    : return o << "all_for_now"    ;
    case BrowserEvent::failure        : return o << "failure"        ;
    default                           : return o << "?"              ;
    }
    return o;
}

AvahiResolverEvent to_avahi(ResolverEvent p)
{
    switch(p)
    {
    case ResolverEvent::found         : return AVAHI_RESOLVER_FOUND  ;
    case ResolverEvent::failure       : return AVAHI_RESOLVER_FAILURE;
    default                           : return AVAHI_RESOLVER_FAILURE;
    }
}
ResolverEvent from_avahi(AvahiResolverEvent p)
{
    switch(p)
    {
    case AVAHI_RESOLVER_FOUND    : return ResolverEvent::found   ;
    case AVAHI_RESOLVER_FAILURE  : return ResolverEvent::failure ;
    default: throw Exception(Error::bad_resolver_event);
    }
}

std::ostream& operator<<(std::ostream& o, const ResolverEvent& p)
{
    switch(p)
    {
    case ResolverEvent::found    : return o << "found"   ;
    case ResolverEvent::failure  : return o << "failure" ;
    default                      : return o << "?"       ;
    }
    return o;
}

AvahiDomainBrowserType to_avahi(DomainBrowserType p)
{
    switch(p)
    {
    case DomainBrowserType::browse          : return AVAHI_DOMAIN_BROWSER_BROWSE          ;
    case DomainBrowserType::browse_default  : return AVAHI_DOMAIN_BROWSER_BROWSE_DEFAULT  ;
    case DomainBrowserType::register_       : return AVAHI_DOMAIN_BROWSER_REGISTER        ;
    case DomainBrowserType::register_default: return AVAHI_DOMAIN_BROWSER_REGISTER_DEFAULT;
    case DomainBrowserType::browse_legacy   : return AVAHI_DOMAIN_BROWSER_BROWSE_LEGACY   ;
    default                                 : return AVAHI_DOMAIN_BROWSER_BROWSE          ;
    }
}
DomainBrowserType from_avahi(AvahiDomainBrowserType p)
{
    switch(p)
    {
    case AVAHI_DOMAIN_BROWSER_BROWSE          : return DomainBrowserType::browse          ;
    case AVAHI_DOMAIN_BROWSER_BROWSE_DEFAULT  : return DomainBrowserType::browse_default  ;
    case AVAHI_DOMAIN_BROWSER_REGISTER        : return DomainBrowserType::register_       ;
    case AVAHI_DOMAIN_BROWSER_REGISTER_DEFAULT: return DomainBrowserType::register_default;
    case AVAHI_DOMAIN_BROWSER_BROWSE_LEGACY   : return DomainBrowserType::browse_legacy   ;
    default: throw Exception(Error::bad_browser_type);
    }
}

std::ostream& operator<<(std::ostream& o, const DomainBrowserType& p)
{
    switch(p)
    {
    case DomainBrowserType::browse          : return o << "browse"          ;
    case DomainBrowserType::browse_default  : return o << "browse_default"  ;
    case DomainBrowserType::register_       : return o << "register"        ;
    case DomainBrowserType::register_default: return o << "register_default";
    case DomainBrowserType::browse_legacy   : return o << "browse_legacy"   ;
    default                                 : return o << "?"               ;
    }
}

uint16_t to_avahi(DnsType t)
{
    return static_cast<uint16_t>(t);
}

std::ostream& operator<<(std::ostream& o, const DnsType& t)
{
    switch(t)
    {
    case DnsType::a    : return o << "A"    ;
    case DnsType::ns   : return o << "NS"   ;
    case DnsType::cname: return o << "CNAME";
    case DnsType::soa  : return o << "SOA"  ;
    case DnsType::ptr  : return o << "PTR"  ;
    case DnsType::hinfo: return o << "HINFO";
    case DnsType::mx   : return o << "MX"   ;
    case DnsType::txt  : return o << "TXT"  ;
    case DnsType::aaaa : return o << "AAAA" ;
    case DnsType::srv  : return o << "SRV"  ;
    default            : return o << "?"    ;
    }
}

static_assert(
    static_cast<int>(DnsType::a    ) == AVAHI_DNS_TYPE_A     &&
    static_cast<int>(DnsType::ns   ) == AVAHI_DNS_TYPE_NS    &&
    static_cast<int>(DnsType::cname) == AVAHI_DNS_TYPE_CNAME &&
    static_cast<int>(DnsType::soa  ) == AVAHI_DNS_TYPE_SOA   &&
    static_cast<int>(DnsType::ptr  ) == AVAHI_DNS_TYPE_PTR   &&
    static_cast<int>(DnsType::hinfo) == AVAHI_DNS_TYPE_HINFO &&
    static_cast<int>(DnsType::mx   ) == AVAHI_DNS_TYPE_MX    &&
    static_cast<int>(DnsType::txt  ) == AVAHI_DNS_TYPE_TXT   &&
    static_cast<int>(DnsType::aaaa ) == AVAHI_DNS_TYPE_AAAA  &&
    static_cast<int>(DnsType::srv  ) == AVAHI_DNS_TYPE_SRV
);


AvahiEntryGroupState to_avahi(GroupState s)
{
    switch(s)
    {
    case GroupState::uncommitted: return AVAHI_ENTRY_GROUP_UNCOMMITED ;
    case GroupState::registering: return AVAHI_ENTRY_GROUP_REGISTERING;
    case GroupState::established: return AVAHI_ENTRY_GROUP_ESTABLISHED;
    case GroupState::collision  : return AVAHI_ENTRY_GROUP_COLLISION  ;
    case GroupState::failure    : return AVAHI_ENTRY_GROUP_FAILURE    ;
    default                     : return AVAHI_ENTRY_GROUP_FAILURE    ;
    }
}
GroupState from_avahi(AvahiEntryGroupState s)
{
    switch(s)
    {
    case AVAHI_ENTRY_GROUP_UNCOMMITED : return GroupState::uncommitted;
    case AVAHI_ENTRY_GROUP_REGISTERING: return GroupState::registering;
    case AVAHI_ENTRY_GROUP_ESTABLISHED: return GroupState::established;
    case AVAHI_ENTRY_GROUP_COLLISION  : return GroupState::collision  ;
    case AVAHI_ENTRY_GROUP_FAILURE    : return GroupState::failure    ;
    default: throw Exception(Error::bad_browser_type)       ;
    }
}
std::ostream& operator<<(std::ostream& o, const GroupState& s)
{
    switch(s)
    {
    case GroupState::uncommitted: return o << "uncommitted";
    case GroupState::registering: return o << "registering";
    case GroupState::established: return o << "established";
    case GroupState::collision  : return o << "collision"  ;
    case GroupState::failure    : return o << "failure"    ;
    default                     : return o << "?"          ;
    }
}

const char* to_avahi(const std::string& s)
{
    return s.c_str();
}

std::string from_avahi(const char* s)
{
    return s ? std::string{s} : std::string{};
}

AvahiStringList* to_avahi(const StrList& in)
{
    AvahiStringList* out = nullptr;

    for(auto it = in.rbegin(); it != in.rend(); ++it)
    {
        out = avahi_string_list_add_arbitrary(
            out,
            reinterpret_cast<const uint8_t*>(it->data()),
            it->size()
        );
    }
    return out;
}

StrList from_avahi(AvahiStringList* in)
{
    StrList out;
    for (auto it = in; it != nullptr; it = it->next)
        out.emplace_back(reinterpret_cast<const char*>(it->text), it->size);
    out.reverse();
    return out;
};



std::ostream& operator<<(std::ostream& o, const IPv4Address& a)
{
    char buf[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &a.value, buf, sizeof(buf));
    return o << buf;
}

AvahiIPv4Address to_avahi(const IPv4Address& in)
{
    return AvahiIPv4Address{ in.value };
}

IPv4Address from_avahi(const AvahiIPv4Address& in)
{
    return IPv4Address{ in.address };
}

std::ostream& operator<<(std::ostream& o, const IPv6Address& a)
{
    char buf[INET6_ADDRSTRLEN] = {};
    inet_ntop(AF_INET6, a.value.data(), buf, sizeof(buf));
    return o << buf;
}

AvahiIPv6Address to_avahi(const IPv6Address& in)
{
    AvahiIPv6Address addr;
    std::copy(in.value.begin(), in.value.end(), addr.address);
    return addr;
}
IPv6Address from_avahi(const AvahiIPv6Address& in)
{
    IPv6Address addr;
    std::copy(std::begin(in.address), std::end(in.address), addr.value.begin());
    return addr;
}

std::ostream& operator<<(std::ostream& o, const Address& a)
{
    if (a.IsV4())
        return o << a.GetV4();
    if (a.IsV6())
        return o << a.GetV6();

    return o;
}

AvahiAddress to_avahi(const Address& in)
{
    AvahiAddress addr;
    if (in.IsV4())
        return AvahiAddress {
            .proto = AVAHI_PROTO_INET,
            .data = {
                .ipv4 = to_avahi(in.GetV4())
            }
        };
    else if (in.IsV6())
        return AvahiAddress {
            .proto = AVAHI_PROTO_INET6,
            .data = {
                .ipv6 = to_avahi(in.GetV6())
            }
        };
    else
        return AvahiAddress {
            .proto = AVAHI_PROTO_UNSPEC
        };
}

Address from_avahi(const AvahiAddress& in)
{
    if (in.proto == AVAHI_PROTO_INET)
        return Address{ from_avahi(in.data.ipv4) };
    else if (in.proto == AVAHI_PROTO_INET6)
        return Address{ from_avahi(in.data.ipv6) };
    else
        return Address{};
}

Address from_avahi(const AvahiAddress* in)
{
    if (in == nullptr)
        return Address{};
    return from_avahi(*in);
}

}
