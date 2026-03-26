#include "hostnameresolver.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns {

HostNameResolver::HostNameResolver(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_host_name_resolver_free)
    , m_resolved(false)
{}

std::error_code HostNameResolver::Start(
    DeviceInfo device,
    std::string hostname,
    Protocol aprotocol,
    LookupFlags flags,
    Callback callback
)
{
    assert(!m_ptr);

    m_callback = callback;

    auto p = avahi_host_name_resolver_new(
        m_client->GetClient(),
        to_avahi(device.interface),
        to_avahi(device.protocol),
        to_avahi(hostname),
        to_avahi(aprotocol),
        to_avahi(flags),
        &HostNameResolver::HostNameResolverCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

void HostNameResolver::Cancel()
{
    m_ptr.reset();
    m_callback = Callback{};
}

std::error_code HostNameResolver::GetLastError()
{
    return m_client->GetLastError();
}

bool HostNameResolver::IsResolved() const
{
    return m_resolved;
}

void HostNameResolver::HostNameResolverCallback (
    AvahiHostNameResolver* r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char* name,
    const AvahiAddress* a,
    AvahiLookupResultFlags flags,
    void *userdata
)
{
    assert(r);
    assert(userdata);

    auto self = static_cast<HostNameResolver*>(userdata);

    std::error_code ec = (event != AVAHI_RESOLVER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    AddressHostnameInfo result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(name),
        from_avahi(a)
    };

    if(self->m_callback)
        self->m_callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );

    self->m_resolved = true;
}
}
