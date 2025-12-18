#include "servicebrowser.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns {

ServiceBrowser::ServiceBrowser(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_service_browser_free)
{
}

std::error_code ServiceBrowser::Start(StartParams&& params)
{
    m_startparams = std::move(params);

    auto p = avahi_service_browser_new (
        m_client->GetClient(),
        to_avahi(m_startparams.request.interface),
        to_avahi(m_startparams.request.protocol),
        to_avahi(m_startparams.request.type),
        to_avahi(m_startparams.request.domain),
        to_avahi(m_startparams.flags),
        &ServiceBrowser::ServiceBrowserCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);

    return Error::ok;
}

void ServiceBrowser::Cancel()
{
    m_ptr.reset();
    m_startparams.callback = Callback{};
}

std::error_code ServiceBrowser::GetLastError()
{
    return m_client->GetLastError();
}

ServiceBrowser::StartParams ServiceBrowser::GetStartParams() const
{
    return m_startparams;
}

void ServiceBrowser::ServiceBrowserCallback (
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AvahiLookupResultFlags flags,
    void* userdata
)
{
    assert(b);
    assert(userdata);

    auto self = static_cast<ServiceBrowser*>(userdata);

    std::error_code ec = (event != AVAHI_BROWSER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    ServiceInfo result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(domain),
        from_avahi(type),
        from_avahi(name)
    };

    if(self->m_startparams.callback)
        self->m_startparams.callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );
}
}
