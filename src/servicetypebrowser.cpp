#include "servicetypebrowser.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
using namespace mdns;

ServiceTypeBrowser::ServiceTypeBrowser(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_service_type_browser_free)
{
}

std::error_code ServiceTypeBrowser::Start(
    DomainInfo request,
    LookupFlags flags,
    Callback callback
)
{
    m_callback = callback;

    auto p = avahi_service_type_browser_new (
        m_client->GetClient(),
        to_avahi(request.interface),
        to_avahi(request.protocol),
        to_avahi(request.domain),
        to_avahi(flags),
        &ServiceTypeBrowser::ServiceTypeBrowserCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);

    return Error::ok;
}

void ServiceTypeBrowser::Cancel()
{
    m_ptr.reset();
    m_callback = Callback{};
}

std::error_code ServiceTypeBrowser::GetLastError()
{
    return m_client->GetLastError();
}

void ServiceTypeBrowser::ServiceTypeBrowserCallback (
    AvahiServiceTypeBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* type,
    const char* domain,
    AvahiLookupResultFlags flags,
    void* userdata
)
{
    assert(b);
    assert(userdata);

    auto self = static_cast<ServiceTypeBrowser*>(userdata);

    std::error_code ec = (event != AVAHI_BROWSER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    TypeInfo result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(domain),
        from_avahi(type)
    };

    if(self->m_callback)
        self->m_callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );
}
