#include "addressresolver.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns{

AddressResolver::AddressResolver(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_address_resolver_free)
    , m_resolved(false)
{}

std::error_code AddressResolver::Start(
    DeviceInfo device,
    Address addr,
    LookupFlags flags,
    Callback callback
)
{
    assert(!m_ptr);

    m_callback = callback;

    AvahiAddress a = to_avahi(addr);

    auto p = avahi_address_resolver_new(
        m_client->GetClient(),
        to_avahi(device.interface),
        to_avahi(device.protocol),
        &a,
        to_avahi(flags),
        &AddressResolver::AddressResolverCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

void AddressResolver::Cancel()
{
    // If m_ptr is populated, calls avahi_address_resolver_free().
    m_ptr.reset();

    // Todo:
    //    Will avahi_address_resolver_free() trigger a callback to a pending
    //    operation?  If not, we may want to m_callback(operation_aborted);
    m_callback = Callback{};
}

std::error_code AddressResolver::GetLastError()
{
    return m_client->GetLastError();
}

bool AddressResolver::IsResolved() const
{
    return m_resolved;
}

void AddressResolver::AddressResolverCallback (
    AvahiAddressResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *a,
    const char *name,
    AvahiLookupResultFlags flags,
    void *userdata
)
{
    assert(r);
    assert(userdata);

    auto self = static_cast<AddressResolver*>(userdata);

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
