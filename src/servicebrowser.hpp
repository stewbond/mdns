#ifndef _1c40b343f1746e256b4d283696dac057
#define _1c40b343f1746e256b4d283696dac057
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class ServiceBrowser
{
public:
    using Callback = ServiceCallback;

    explicit ServiceBrowser(std::shared_ptr<Client> client);
    ServiceBrowser(const ServiceBrowser&)            = delete;
    ServiceBrowser(ServiceBrowser&&)                 = default;
    ServiceBrowser& operator=(const ServiceBrowser&) = delete;
    ServiceBrowser& operator=(ServiceBrowser&&)      = default;

    std::error_code Start(
        TypeInfo request,
        LookupFlags flags, // Can (1) force WAN or MCAST, (2) skip TXT, A, or AAAA records
        Callback callback
    );

    void Cancel();

    std::error_code GetLastError();

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<AvahiServiceBrowser, int(*)(AvahiServiceBrowser*)> m_ptr;

    static void ServiceBrowserCallback (
        AvahiServiceBrowser* b,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char* name,
        const char* type,
        const char* domain,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
