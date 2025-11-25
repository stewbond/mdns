#include "pollsimple.hpp"
#include <mdns/errorcode.hpp>
#include <mdns/exception.hpp>
#include <avahi-common/simple-watch.h>
#include <avahi-client/client.h>
#include <cassert>
namespace mdns {
using namespace priv;

PollSimple::PollSimple()
    : m_pollobj(avahi_simple_poll_new(), avahi_simple_poll_free)
{
    if (!m_pollobj)
        throw Exception(Error::could_not_construct);
}

const AvahiPoll* PollSimple::GetPoll() const
{
    return avahi_simple_poll_get(m_pollobj.get());
}

std::error_code PollSimple::Run()
{
    return loop();
}

std::error_code PollSimple::Stop()
{
    avahi_simple_poll_quit(m_pollobj.get());
    return Error::ok;
}

std::error_code PollSimple::loop() noexcept
{
    int r = avahi_simple_poll_loop(m_pollobj.get());
    if (r <= 0)
        return ErrFromAvahiInt(r);
    return Error::aborted;
}

bool PollSimple::iterate(
    std::error_code& ec,
    std::chrono::milliseconds timeout) noexcept
{
    int r = avahi_simple_poll_iterate(m_pollobj.get(), timeout.count());
    ec = r > 0 ? make_error_code(Error::aborted) : ErrFromAvahiInt(r);
    return r; // true if error, or actual stop request.
}

bool PollSimple::iterate(std::chrono::milliseconds timeout)
{
    int r = avahi_simple_poll_iterate(m_pollobj.get(), timeout.count());
    if (r < 0)
        throw Exception(ErrFromAvahiInt(r));
    return r;
}

void PollSimple::wakeup() noexcept
{
    avahi_simple_poll_wakeup(m_pollobj.get());
}

void PollSimple::SetPollFunc(PollFunc f)
{
    m_pollfunc = f;
    if (f)
    {
        avahi_simple_poll_set_func(
            m_pollobj.get(),
            &PollSimple::StaticPoll,
            this // no need to shared_from_this() because _free() prevents late callbacks
        );
    }
    else
    {
        // No idea if this actually works
        avahi_simple_poll_set_func(m_pollobj.get(), nullptr, nullptr);
    }
}

int PollSimple::StaticPoll(
    struct pollfd *ufds,
    unsigned int nfds,
    int timeout,
    void*userdata
)
{
    assert(userdata);
    auto self = static_cast<PollSimple*>(userdata);
    if (self && self->m_pollfunc)
    {
        return self->m_pollfunc(ufds, nfds, timeout);
    }
    return -1;
}

using namespace std::chrono_literals;
const std::chrono::milliseconds PollSimple::forever = -1ms;
const std::chrono::milliseconds PollSimple::noblock = 0ms;

/*
 *  These functions are expert-only
 *  These are the only parts not implemented, and that's intentional
 *    int avahi_simple_poll_prepare(AvahiSimplePoll *s, int timeout);
 *    int avahi_simple_poll_run(AvahiSimplePoll *s);
 *    int avahi_simple_poll_dispatch(AvahiSimplePoll *s);
 */

}
