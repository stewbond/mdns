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
    Callback callback,
    DisconnectCb disconnectCb
) noexcept
{
    m_callback = callback;
    m_disconnectCb = disconnectCb;
    m_flags = flags;
    m_poll = poller.GetPoll();

    return Connect();
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

bool Client::IsConnected()
{
    return IsConnected( GetState() );
}

bool Client::IsConnected(ClientState state)
{
    // Per avahi Doxygen:
    //  > As soon as the daemon becomes available, the object will enter one
    //  > of the AVAHI_CLEINT_S_xxx states.  Make sure to not create browsers or
    //  > entry groups before the client object has entered one of those states.
    // The AVAHI_CLIENT_S_xxx states include running, registering, collision.
    // This function tells us if we are in one of those states.
    return
        state == ClientState::registering ||
        state == ClientState::running     ||
        state == ClientState::collision   ;
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

std::error_code Client::Connect()
{
    int error = AVAHI_OK;
    AvahiClient* p = avahi_client_new(
        m_poll,
        to_avahi(m_flags),
        StaticCallback, // this WILL get called before *_new() returns
        this, // No need for shared_from_this().  dtor calls _free() which prevents more callbacks *after* destruction (might still be a callback during).
        &error
    );

    assert(p || error);

    if (m_ptr == nullptr)
        m_ptr.reset(p);

    return priv::ErrFromAvahiInt(error);
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

    auto s = from_avahi(state);

    // I think it makes sense to trigger this callback before doing any reset logic
    //  Otherwise I assume the end-user could get callbacks in a different order
    if (self->m_callback)
        self->m_callback(s);

    bool has_disconnected =
        s == ClientState::failure &&
        avahi_client_errno(c) == AVAHI_ERR_DISCONNECTED;

    // This condition will occur when the avahi-daemon disconnects.
    // We should be able to recover by doing this
    if (has_disconnected && !self->m_destroying)
    {
        // Destroy all browsers, resolvers, entry groups related to this client.
        // Then delete the client
        if (self->m_disconnectCb)
            self->m_disconnectCb();
        self->m_ptr.reset();

        if ((self->m_flags & ClientFlags::no_fail) != ClientFlags::none)
            self->Connect();
    }
}

}
