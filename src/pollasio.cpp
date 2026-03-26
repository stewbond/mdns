#include "pollasio.hpp"
#include <cassert>
#include <boost/asio/steady_timer.hpp>

namespace mdns {

PollAsio::PollAsio(IoContext& io)
    : m_io(boost::asio::make_strand(io))
{
    m_poll.watch_new       = &PollAsio::Watch::New;
    m_poll.watch_update    = &PollAsio::Watch::Update;
    m_poll.watch_get_events= &PollAsio::Watch::GetEvents;
    m_poll.watch_free      = &PollAsio::Watch::Free;

    m_poll.timeout_new     = &PollAsio::Timeout::New;
    m_poll.timeout_update  = &PollAsio::Timeout::Update;
    m_poll.timeout_free    = &PollAsio::Timeout::Free;

    m_poll.userdata        = this;
}

const AvahiPoll* PollAsio::GetPoll() const
{
    return &m_poll;
}

std::error_code PollAsio::Run()
{
    m_io.get_inner_executor().context().run();
    return Error::ok;
}
std::error_code PollAsio::Stop()
{
    return Error::ok;
}

std::error_code PollAsio::Post(std::function<void()> work)
{
    boost::asio::post(m_io, std::move(work));
    return Error::ok;
}

PollAsio::Strand& PollAsio::GetIo()
{
    return m_io;
}



/////////////////////////////////////////////////////////////////////////////////
/// \brief PollAsio::Watch implementation
/////////////////////////////////////////////////////////////////////////////////
PollAsio::Watch::Watch(
    Strand& io,
    int fd,
    AvahiWatchEvent event,
    AvahiWatchCallback callback,
    void* userdata
)
    : m_fd(io, fd)
    , m_callback(callback)
    , m_userdata(userdata)
    , m_watchedEvents(event)
    , m_triggeredEvents(NoEvent)
{
    Wait(event);
}

void PollAsio::Watch::Wait(AvahiWatchEvent event)
{
    using namespace std::placeholders;
    if (event & AVAHI_WATCH_IN)
        m_fd.async_wait(
            Descriptor::wait_read,
            std::bind(&PollAsio::Watch::OnEvent, this, AVAHI_WATCH_IN, _1)
        );

    if (event & AVAHI_WATCH_OUT)
        m_fd.async_wait(
            Descriptor::wait_write,
            std::bind(&PollAsio::Watch::OnEvent, this, AVAHI_WATCH_OUT, _1)
        );

    // POLLERR and POLLHUP will be included in POLLIN or POLLOUT revents.
    // POLLERR and POLLHUP are meaningless themselves as inputs
    // No need to add another wait unless there is no read/write.
    if ( (event & (AVAHI_WATCH_ERR | AVAHI_WATCH_HUP)) &&
          !(event & (AVAHI_WATCH_IN | AVAHI_WATCH_OUT))
    )
        m_fd.async_wait(
            Descriptor::wait_error,
            std::bind(&PollAsio::Watch::OnEvent, this, AVAHI_WATCH_ERR, _1)
        );
}

AvahiWatch* PollAsio::Watch::GetObj()
{
    return reinterpret_cast<AvahiWatch*>(this);
}

AvahiWatchEvent PollAsio::Watch::GetEvents()
{
    return m_triggeredEvents;
}

AvahiWatchEvent PollAsio::Watch::CheckForExtras()
{
    short int events = 0;
    auto output = NoEvent;

    if (m_watchedEvents & AVAHI_WATCH_HUP)
        events |= POLLHUP;

    if (m_watchedEvents & AVAHI_WATCH_ERR)
        events |= POLLERR;

    if (!events)
        return NoEvent;

    pollfd pfd { .fd = m_fd.native_handle(), .events = events };

     // Quick non-blocking poll to see if HUP/ERR was set.
    if (::poll(&pfd, 1, 0) <= 0)
        return output;

    if (pfd.revents & POLLHUP)
        output = static_cast<AvahiWatchEvent>(output | AVAHI_WATCH_HUP);

    if (pfd.revents & POLLERR)
        output = static_cast<AvahiWatchEvent>(output | AVAHI_WATCH_ERR);

    return output;
}

void PollAsio::Watch::OnEvent(AvahiWatchEvent event, const ErrorCode& ec)
{
    if (ec)
        return;

    m_triggeredEvents = static_cast<AvahiWatchEvent>(event | CheckForExtras());

    if (m_callback)
        m_callback(GetObj(), m_fd.native_handle(), m_triggeredEvents, m_userdata);

    m_triggeredEvents = NoEvent;

    Wait(event);
}

const AvahiWatchEvent PollAsio::Watch::NoEvent = static_cast<AvahiWatchEvent>(0);


void PollAsio::Watch::Update(AvahiWatchEvent event)
{
    m_watchedEvents = event;
    m_fd.cancel();
    Wait(event);
}


AvahiWatch* PollAsio::Watch::New(
    const AvahiPoll *api,
    int fd,
    AvahiWatchEvent event,
    AvahiWatchCallback callback,
    void *userdata
)
{
    assert(api);
    auto* poll = static_cast<PollAsio*>(api->userdata);
    auto* watch = new Watch(poll->GetIo(), fd, event, callback, userdata);
    return watch->GetObj();
}

void PollAsio::Watch::Update(AvahiWatch* w, AvahiWatchEvent event)
{
    assert(w);
    auto* watch = reinterpret_cast<Watch*>(w);
    watch->Update(event);
}

AvahiWatchEvent PollAsio::Watch::GetEvents(AvahiWatch *w)
{
    assert(w);
    auto* watch = reinterpret_cast<Watch*>(w);
    return watch->GetEvents();
}

void PollAsio::Watch::Free(AvahiWatch* w)
{
    assert(w);
    auto* watch = reinterpret_cast<Watch*>(w);
    delete watch;
}


/////////////////////////////////////////////////////////////////////////////////
/// \brief PollAsio::Timeout implementation
/////////////////////////////////////////////////////////////////////////////////
PollAsio::Timeout::Timeout(
    Strand& io,
    const struct timeval* tv,
    AvahiTimeoutCallback callback,
    void* userdata
)
    : m_timer(io)
    , m_callback(callback)
    , m_userdata(userdata)
{
    Update(tv);
}

void PollAsio::Timeout::Update(const struct timeval* tv)
{
    using namespace std::placeholders;
    if (tv == nullptr)
    {
        m_timer.cancel();
    }
    else
    {
        m_timer.expires_after(
            std::chrono::seconds(tv->tv_sec) +
            std::chrono::microseconds(tv->tv_usec)
        );
        m_timer.async_wait(
            std::bind(&PollAsio::Timeout::OnTimeout, this, _1)
        );
    }
}

AvahiTimeout* PollAsio::Timeout::GetObj()
{
    return reinterpret_cast<AvahiTimeout*>(this);
}

void PollAsio::Timeout::OnTimeout(const ErrorCode& ec)
{
    if (!ec && m_callback)
        m_callback(GetObj(), m_userdata);
}

AvahiTimeout* PollAsio::Timeout::New(
    const AvahiPoll *api,
    const struct timeval *tv,
    AvahiTimeoutCallback callback,
    void *userdata
)
{
    assert(api);
    auto* poll = static_cast<PollAsio*>(api->userdata);
    auto* timer = new Timeout(poll->GetIo(), tv, callback, userdata);
    return timer->GetObj();
}

void PollAsio::Timeout::Update(AvahiTimeout* obj, const struct timeval* tv)
{
    assert(obj);
    auto* timer = reinterpret_cast<Timeout*>(obj);
    timer->Update(tv);
}

void PollAsio::Timeout::Free(AvahiTimeout *obj)
{
    assert(obj);
    auto* timer = reinterpret_cast<Timeout*>(obj);
    delete timer;
}

}
