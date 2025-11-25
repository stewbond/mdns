#ifndef _df3f8514a0d7e24cc6e8f10c00ba5238
#define _df3f8514a0d7e24cc6e8f10c00ba5238
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class ServiceResolver
{
public:
    using Callback = ResolvedServiceCallback;

    explicit ServiceResolver(std::shared_ptr<Client> client);
    ServiceResolver(const ServiceResolver&)            = delete;
    ServiceResolver(ServiceResolver&&)                 = default;
    ServiceResolver& operator=(const ServiceResolver&) = delete;
    ServiceResolver& operator=(ServiceResolver&&)      = default;

    std::error_code Start(
        ServiceInfo request,
        Protocol protocol,
        LookupFlags flags,
        Callback callback
    );

    void Cancel();

    std::error_code GetLastError();

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<AvahiServiceResolver, int(*)(AvahiServiceResolver*)> m_ptr;

    static void ServiceResolverCallback (
        AvahiServiceResolver* r,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiResolverEvent event,
        const char* name,
        const char* type,
        const char* domain,
        const char* hostname,
        const AvahiAddress* a,
        uint16_t port,
        AvahiStringList* txt,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
