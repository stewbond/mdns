#ifndef _c555a288e736566b3ba0c76cd32a0e87
#define _c555a288e736566b3ba0c76cd32a0e87
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class HostNameResolver
{
public:
    using Callback = AddressCallback;

    explicit HostNameResolver(std::shared_ptr<Client> client);
    HostNameResolver(const HostNameResolver&)            = delete;
    HostNameResolver(HostNameResolver&&)                 = default;
    HostNameResolver& operator=(const HostNameResolver&) = delete;
    HostNameResolver& operator=(HostNameResolver&&)      = default;

    std::error_code Start(
        DeviceInfo device,
        std::string hostname,
        Protocol aprotocol,
        LookupFlags flags,
        Callback callback
    );

    void Cancel();

    std::error_code GetLastError();
    bool IsResolved() const;

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<
        AvahiHostNameResolver,
        int(*)(AvahiHostNameResolver*) // destructor auto _free()'s
    > m_ptr;
    bool m_resolved;

    static void HostNameResolverCallback (
        AvahiHostNameResolver* r,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiResolverEvent event,
        const char* name,
        const AvahiAddress* a,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
