#ifndef _7c72b5dd4d3f483a6c884b2217eafea1
#define _7c72b5dd4d3f483a6c884b2217eafea1
#include <avahi-client/lookup.h>
#include <memory>
#include <mdns/structures.hpp>
namespace mdns {

class Client;

class RecordBrowser
{
public:
    using Callback = RecordCallback;

    explicit RecordBrowser(std::shared_ptr<Client> client);
    RecordBrowser(const RecordBrowser&)            = delete;
    RecordBrowser(RecordBrowser&&)                 = default;
    RecordBrowser& operator=(const RecordBrowser&) = delete;
    RecordBrowser& operator=(RecordBrowser&&)      = default;

    std::error_code Start(
        RecordRequest request,
        LookupFlags flags,
        Callback
    );

    void Cancel();

    std::error_code GetLastError();

private:
    std::shared_ptr<Client> m_client;
    Callback m_callback;
    std::unique_ptr<AvahiRecordBrowser, int(*)(AvahiRecordBrowser*)> m_ptr;

    static void RecordBrowserCallback(
        AvahiRecordBrowser *b,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char *name,
        uint16_t clazz,
        uint16_t type,
        const void *rdata,
        size_t size,
        AvahiLookupResultFlags flags,
        void *userdata
    );
};
}
#endif
