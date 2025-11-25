#include "client.hpp"
#include "polliface.hpp"
#include <mdns/exception.hpp>
#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <cassert>
#include "structures_priv.hpp"
namespace mdns{

Client::Client() noexcept
    : m_destroying(false)
    , m_ptr(nullptr, avahi_client_free)
{}

Client::~Client()
{
    m_destroying = true;
}

std::error_code Client::Start(
    const PollIface& poller,
    ClientFlags flags,
    Callback callback
) noexcept
{
    m_callback = callback;

    int error = AVAHI_OK;
    AvahiClient* p = avahi_client_new(
        poller.GetPoll(),
        to_avahi(flags),
        StaticCallback, // this WILL get called before *_new() returns
        this, // No need for shared_from_this().  dtor calls _free() which prevents more callbacks *after* destruction (might still be a callback during).
        &error
    );

    assert(p || error);

    if (m_ptr == nullptr)
        m_ptr.reset(p);

    return priv::ErrFromAvahiInt(error);

}

AvahiClient* Client::GetClient() noexcept
{
    return m_ptr.get();
}

const AvahiClient* Client::GetClient() const noexcept
{
    return m_ptr.get();
}

std::string Client::GetVersion(std::error_code& ec) noexcept
{
    auto c = avahi_client_get_version_string(m_ptr.get());

    if (c == nullptr)
    {
        ec = GetLastError();
        return std::string{};
    }
    else
    {
        ec = make_error_code(Error::ok);
        return std::string(c);
    }
}

std::string Client::GetVersion()
{
    std::error_code ec;
    auto str = GetVersion(ec);
    if (ec)
        throw Exception(ec);
    return str;
}

std::string Client::GetHostname(std::error_code& ec) noexcept
{
    auto c = avahi_client_get_host_name(m_ptr.get());

    if (c == nullptr)
    {
        ec = GetLastError();
        return {};
    }
    else
    {
        ec = make_error_code(Error::ok);
        return std::string(c);
    }
}

std::string Client::GetHostname()
{
    std::error_code ec;
    auto str = GetHostname(ec);
    if (ec)
        throw Exception(ec);
    return str;
}

std::string Client::GetDomain(std::error_code& ec) noexcept
{
    auto c = avahi_client_get_domain_name(m_ptr.get());

    if (c == nullptr)
    {
        ec = GetLastError();
        return std::string{};
    }
    else
    {
        ec = make_error_code(Error::ok);
        return std::string(c);
    }
}

std::string Client::GetDomain()
{
    std::error_code ec;
    auto str = GetDomain(ec);
    if (ec)
        throw Exception(ec);
    return str;
}

std::string Client::GetFqdn(std::error_code& ec) noexcept
{
    auto c = avahi_client_get_host_name_fqdn(m_ptr.get());

    if (c == nullptr)
    {
        ec = GetLastError();
        return std::string{};
    }
    else
    {
        ec = make_error_code(Error::ok);
        return std::string(c);
    }
}

std::string Client::GetFqdn()
{
    std::error_code ec;
    auto str = GetFqdn(ec);
    if (ec)
        throw Exception(ec);
    return str;
}

// not guaranteeing noexcept in case we add an error_code overload the future.
ClientState Client::GetState()
{
    // I assume we don't need error checking,
    // a problem is probably reported as AVAHI_CLIENT_FAILURE instead of errno
    return from_avahi(avahi_client_get_state(m_ptr.get()));
}

uint32_t Client::GetLocalServiceCookie()
{
    // Returns AVAHI_SERVICE_COOKIE_INVALID on failure
    return avahi_client_get_local_service_cookie(m_ptr.get());
}

std::error_code Client::GetLastError() noexcept
{
    return priv::AvahiErrorFromInt(avahi_client_errno(m_ptr.get()));
}

void Client::SetHostname(
    const std::string& hostname,
    std::error_code& ec
    ) noexcept
{
    int r = avahi_client_set_host_name(m_ptr.get(), hostname.c_str());
    ec = r < 0 ? priv::AvahiErrorFromInt(r) : Error::ok;
}

void Client::SetHostname(const std::string& hostname)
{
    std::error_code ec;
    SetHostname(hostname, ec);
    if (ec)
        throw Exception(ec);
}

bool Client::SupportsMdnsLookups() const noexcept
{
    return avahi_nss_support();
}


void Client::StaticCallback(
    AvahiClient* c,
    AvahiClientState state,
    void* userdata)
{
    assert(c);
    assert(userdata);

    auto self = static_cast<Client*>(userdata);

    // This is first called *during* avahi_client_new(), before m_ptr is set.
    // Ensure m_ptr is valid before calling the client's callback.
    if (!self->m_destroying && !self->m_ptr) // Avoid touching m_ptr during ~dtor
        self->m_ptr.reset(c);

    if (self->m_callback)
        self->m_callback(from_avahi(state));
}

}
