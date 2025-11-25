#include "serviceresolver.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns{

ServiceResolver::ServiceResolver(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_service_resolver_free)
{}

std::error_code ServiceResolver::Start(
    ServiceInfo request,
    Protocol protocol,
    LookupFlags flags,
    Callback callback
)
{
    assert(!m_ptr);

    m_callback = callback;

    auto p = avahi_service_resolver_new(
        m_client->GetClient(),
        to_avahi(request.interface),
        to_avahi(request.protocol), // mDNS queries are IPv4 or IPv6 (use browser_key)
        to_avahi(request.name),
        to_avahi(request.type),
        to_avahi(request.domain),
        to_avahi(protocol),
        to_avahi(flags),
        &ServiceResolver::ServiceResolverCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

void ServiceResolver::Cancel()
{
    m_ptr.reset();
    m_callback = Callback{};
}

std::error_code ServiceResolver::GetLastError()
{
    return m_client->GetLastError();
}

void ServiceResolver::ServiceResolverCallback (
    AvahiServiceResolver* r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char* name,
    const char* type,
    const char* domain,
    const char* hostname,
    const AvahiAddress* a,
    uint16_t port,
    AvahiStringList* txt,
    AvahiLookupResultFlags flags,
    void *userdata
)
{
    assert(r);
    assert(userdata);

    auto self = static_cast<ServiceResolver*>(userdata);

    std::error_code ec = (event != AVAHI_RESOLVER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    // IfIndex interface
    // Protocol protocol
    // std::string domain.
    // std;:string type
    // std::string name
    // std::string hostname;
    // uint16_t port
    // StrList txt
    // StrList subtypes
    // Address addr;
    ResolvedServiceInfo result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(domain),
        from_avahi(type),
        from_avahi(name),
        from_avahi(hostname),
        port,
        from_avahi(txt),
        from_avahi(a)
    };

    if(self->m_callback)
        self->m_callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );
}
}
