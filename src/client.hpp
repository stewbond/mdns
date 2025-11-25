#ifndef _497c3a999960eef979de5faa766da71b
#define _497c3a999960eef979de5faa766da71b
#include <memory> // std::unique_ptr<>
#include <avahi-client/client.h> // AvahiClientState, AvahiClientFlags, AvahiClient
#include <string> // std::string
#include <system_error> // std::error_ocde
#include <cstdint> // uint32_t
#include <mdns/structures.hpp>

struct AvahiClient;

namespace mdns {
class PollIface;

class Client
{
public:
    using Callback = ClientCallback;

    Client() noexcept;
    ~Client();

    std::error_code Start(
        const PollIface& poller,
        ClientFlags flags,
        Callback callback
    ) noexcept;

    Client(const Client&)            = delete;
    Client(Client&&)                 = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&)      = default;

    AvahiClient* GetClient() noexcept;
    const AvahiClient* GetClient() const noexcept;
    std::string GetVersion(std::error_code&) noexcept;
    std::string GetVersion();
    std::string GetHostname(std::error_code&) noexcept;
    std::string GetHostname();
    std::string GetDomain(std::error_code&) noexcept;
    std::string GetDomain();
    std::string GetFqdn(std::error_code&) noexcept;
    std::string GetFqdn();
    ClientState GetState();
    uint32_t GetLocalServiceCookie(); // Returns AVAHI_SERVICE_COOKIE_INVALID on failure
    std::error_code GetLastError() noexcept;
    void SetHostname(const std::string&, std::error_code& ) noexcept;
    void SetHostname(const std::string&);
    bool SupportsMdnsLookups() const noexcept; // gethostbyname() supports mDNS lookups

private:
    // Order is important!
    //   m_callback must be defined *before* m_ptr,
    //   so the final callback during m_ptr's destructor actually makes it to its
    //   destination.
    bool m_destroying;
    Callback m_callback;
    std::unique_ptr<AvahiClient, void(*)(AvahiClient*)> m_ptr;

    static void StaticCallback(AvahiClient*,AvahiClientState,void*);
};

}
#endif
