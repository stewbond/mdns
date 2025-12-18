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
    struct StartParams
    {
        DomainInfo request;
        DomainBrowserType type; // AVAHI_DOMAIN_BROWSER_BROWSE
        LookupFlags flags;
        Callback callback;
    };

    explicit DomainBrowser(std::shared_ptr<Client> client);
    DomainBrowser(const DomainBrowser&)            = delete;
    DomainBrowser(DomainBrowser&&)                 = default;
    DomainBrowser& operator=(const DomainBrowser&) = delete;
    DomainBrowser& operator=(DomainBrowser&&)      = default;

    std::error_code Start(StartParams&&);

    void Cancel();

    std::error_code GetLastError();

    StartParams GetStartParams() const;

private:
    std::shared_ptr<Client> m_client;
    StartParams m_startparams;
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
