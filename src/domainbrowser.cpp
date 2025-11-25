#include "domainbrowser.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns{

DomainBrowser::DomainBrowser(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_domain_browser_free)
{}

std::error_code DomainBrowser::Start(
    DomainInfo request,
    DomainBrowserType type,
    LookupFlags flags,
    Callback callback
)
{
    m_callback = callback;

    auto p = avahi_domain_browser_new (
        m_client->GetClient(),
        to_avahi(request.interface),
        to_avahi(request.protocol),
        to_avahi(request.domain),
        to_avahi(type),
        to_avahi(flags),
        &DomainBrowser::DomainBrowserCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

void DomainBrowser::Cancel()
{
    m_ptr.reset();
    m_callback = Callback{};
}

std::error_code DomainBrowser::GetLastError()
{
    return m_client->GetLastError();
}

void DomainBrowser::DomainBrowserCallback(
    AvahiDomainBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *userdata
)
{
    assert(b);
    assert(userdata);

    auto self = static_cast<DomainBrowser*>(userdata);

    std::error_code ec = (event != AVAHI_BROWSER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    DomainInfo result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(domain)
    };

    if (self->m_callback)
        self->m_callback(
            from_avahi(event),
            std::move(result),
            from_avahi(flags),
            ec
        );
}

}
