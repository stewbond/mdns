#ifndef _02e78fbb1a13308ec957867bd23eede7
#define _02e78fbb1a13308ec957867bd23eede7
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class DomainBrowser
{
public:
    using Callback = DomainCallback;

    explicit DomainBrowser(std::shared_ptr<Client> client);
    DomainBrowser(const DomainBrowser&)            = delete;
    DomainBrowser(DomainBrowser&&)                 = default;
    DomainBrowser& operator=(const DomainBrowser&) = delete;
    DomainBrowser& operator=(DomainBrowser&&)      = default;

    std::error_code Start(
        DomainInfo,
        DomainBrowserType, // AVAHI_DOMAIN_BROWSER_BROWSE
        LookupFlags,
        Callback
    );

    void Cancel();

    std::error_code GetLastError();

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<AvahiDomainBrowser, int(*)(AvahiDomainBrowser*)> m_ptr;

    static void DomainBrowserCallback(
        AvahiDomainBrowser *b,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char *domain,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
