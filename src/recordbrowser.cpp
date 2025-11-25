#include "recordbrowser.hpp"
#include "client.hpp"
#include <mdns/errorcode.hpp>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns {

RecordBrowser::RecordBrowser(std::shared_ptr<Client> client)
    : m_client(client)
    , m_ptr(nullptr, avahi_record_browser_free)
{}

std::error_code RecordBrowser::Start(
    RecordRequest request,
    LookupFlags flags,
    Callback callback
    )
{
    m_callback = callback;

    auto p = avahi_record_browser_new(
        m_client->GetClient(),
        to_avahi(request.interface),
        to_avahi(request.protocol),
        to_avahi(request.name),
        AVAHI_DNS_CLASS_IN, // Fine to hard-code.  Avahi provides no other options.
        to_avahi(request.type),
        to_avahi(flags),
        &RecordBrowser::RecordBrowserCallback,
        this
    );

    if (p == nullptr)
        return GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

void RecordBrowser::Cancel()
{
    m_ptr.reset();
    m_callback = Callback{};
}

std::error_code RecordBrowser::GetLastError()
{
    return m_client->GetLastError();
}

void RecordBrowser::RecordBrowserCallback(
    AvahiRecordBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    uint16_t,           // Always AVAHI_DNS_CLASS_IN... ignoring
    uint16_t type,
    const void *rdata,
    size_t size,
    AvahiLookupResultFlags flags,
    void *userdata
)
{
    assert(b);
    assert(userdata);

    auto self = static_cast<RecordBrowser*>(userdata);

    std::error_code ec = (event != AVAHI_BROWSER_FAILURE) ?
        Error::ok :
        self->GetLastError();

    RecordResult result = {
        static_cast<IfIndex>(interface),
        from_avahi(protocol),
        from_avahi(name),
        static_cast<DnsType>(type),
        rdata,
        size
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
