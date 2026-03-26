#ifndef _1fddf2971383bcebded27ed2e41ebf8d
#define _1fddf2971383bcebded27ed2e41ebf8d
#include <avahi-client/lookup.h>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Client;

class AddressResolver
{
public:
    using Callback = AddressCallback;

    explicit AddressResolver(std::shared_ptr<Client> client);
    AddressResolver(const AddressResolver&)            = delete;
    AddressResolver(AddressResolver&&)                 = default;
    AddressResolver& operator=(const AddressResolver&) = delete;
    AddressResolver& operator=(AddressResolver&&)      = default;

    std::error_code Start(
        DeviceInfo device,
        Address a,
        LookupFlags flags,
        Callback callback
    );

    void Cancel();

    std::error_code GetLastError();
    bool IsResolved() const;

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<AvahiAddressResolver, int(*)(AvahiAddressResolver*)> m_ptr;
    bool m_resolved;

    static void AddressResolverCallback (
        AvahiAddressResolver *r,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiResolverEvent event,
        const AvahiAddress *a,
        const char *name,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
