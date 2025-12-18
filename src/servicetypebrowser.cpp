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

std::error_code ServiceTypeBrowser::Start(StartParams&& params)
{
    m_startparams = std::move(params);

    auto p = avahi_service_type_browser_new (
        m_client->GetClient(),
        to_avahi(m_startparams.request.interface),
        to_avahi(m_startparams.request.protocol),
        to_avahi(m_startparams.request.domain),
        to_avahi(m_startparams.flags),
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
    m_startparams.callback = Callback{};
}

std::error_code ServiceTypeBrowser::GetLastError()
{
    return m_client->GetLastError();
}

ServiceTypeBrowser::StartParams ServiceTypeBrowser::GetStartParams() const
{
    return m_startparams;
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

    if(self->m_startparams.callback)
        self->m_startparams.callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );
}
