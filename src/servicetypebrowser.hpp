#ifndef _cff7d52fef4f2ea9d76a6f28514ee490
#define _cff7d52fef4f2ea9d76a6f28514ee490
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class ServiceTypeBrowser
{
public:
    using Callback = ServiceTypeCallback;

    explicit ServiceTypeBrowser(std::shared_ptr<Client> client);
    ServiceTypeBrowser(const ServiceTypeBrowser&)            = delete;
    ServiceTypeBrowser(ServiceTypeBrowser&&)                 = default;
    ServiceTypeBrowser& operator=(const ServiceTypeBrowser&) = delete;
    ServiceTypeBrowser& operator=(ServiceTypeBrowser&&)      = default;

    std::error_code Start(
        DomainInfo request,
        LookupFlags flags,
        Callback callback
    );

    void Cancel();

    std::error_code GetLastError();

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<
        AvahiServiceTypeBrowser,
        int(*)(AvahiServiceTypeBrowser*) // destructor auto _free()'s
    > m_ptr;

    static void ServiceTypeBrowserCallback (
        AvahiServiceTypeBrowser* b,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char* type,
        const char* domain,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
