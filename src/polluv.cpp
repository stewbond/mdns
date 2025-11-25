#include "polluv.hpp"
#include <cassert>

namespace mdns {
PollUv::PollUv(Loop* loop)
    : m_loop(loop)
{
    m_poll.watch_new       = &PollUv::Watch::New;
    m_poll.watch_update    = &PollUv::Watch::Update;
    m_poll.watch_get_events= &PollUv::Watch::GetEvents;
    m_poll.watch_free      = &PollUv::Watch::Free;

    m_poll.timeout_new     = &PollUv::Timeout::New;
    m_poll.timeout_update  = &PollUv::Timeout::Update;
    m_poll.timeout_free    = &PollUv::Timeout::Free;

    m_poll.userdata        = this;
}

const AvahiPoll* PollUv::GetPoll() const
{
    uv_run(m_loop, UV_RUN_DEFAULT);
    return &m_poll;
}

std::error_code PollUv::PollUv::Run()
{
    int err = uv_run(m_loop, UV_RUN_DEFAULT);

    // This error casting only works on Posix systems.
    return std::make_error_code(static_cast<std::errc>(err));
}

std::error_code PollUv::PollUv::Stop()
{
    //uv_stop(m_loop);
    return Error::ok;
}

PollUv::Loop* PollUv::GetLoop()
{
    return m_loop;
}


/////////////////////////////////////////////////////////////////////////////////
/// \brief PollUv::Watch implementation
/////////////////////////////////////////////////////////////////////////////////

PollUv::Watch::Watch(
    Loop* loop,
    int fd,
    AvahiWatchEvent event,
    AvahiWatchCallback callback,
    void* userdata)
    : m_fd(fd)
    , m_callback(callback)
    , m_userdata(userdata)
{
    uv_poll_init(loop, &m_handle, fd);
    m_handle.data = this;

    Start(event);
}

PollUv::Watch::~Watch()
{
    uv_poll_stop(&m_handle);
}

AvahiWatch* PollUv::Watch::GetObj()
{
    return reinterpret_cast<AvahiWatch*>(this);
}

AvahiWatchEvent PollUv::Watch::GetEvents()
{
    return m_triggeredEvents;
}

void PollUv::Watch::Start(AvahiWatchEvent event)
{
    m_watchedEvents = event;

    int events = 0;

    if (event & AVAHI_WATCH_IN)
        events |= UV_READABLE;
    if (event & AVAHI_WATCH_OUT)
        events |= UV_WRITABLE;

    uv_poll_start(&m_handle, events, &Watch::OnEvent);
}

void PollUv::Watch::Update(AvahiWatchEvent newevents)
{
    uv_poll_stop(&m_handle);
    Start(newevents);
}

void PollUv::Watch::OnEvent(int status, int events)
{
    if (status < 0)
        return;

    AvahiWatchEvent event = NoEvent;
    if (events & UV_READABLE)
        event = static_cast<AvahiWatchEvent>(event | AVAHI_WATCH_IN);
    if (events & UV_WRITABLE)
        event = static_cast<AvahiWatchEvent>(event | AVAHI_WATCH_OUT);
    event = static_cast<AvahiWatchEvent>(event | CheckForExtras());

    m_triggeredEvents = event;

    if (m_callback)
        m_callback(GetObj(), m_fd, m_triggeredEvents, m_userdata);

    m_triggeredEvents = NoEvent;
}

AvahiWatchEvent PollUv::Watch::CheckForExtras()
{
    short int events = 0;
    auto output = NoEvent;

    if (m_watchedEvents & AVAHI_WATCH_HUP)
        events |= POLLHUP;

    if (m_watchedEvents & AVAHI_WATCH_ERR)
        events |= POLLERR;

    if (!events)
        return NoEvent;

    pollfd pfd { .fd = m_fd, .events = events };

    // Quick non-blocking poll to see if HUP/ERR was set.
    if (::poll(&pfd, 1, 0) <= 0)
        return output;

    if (pfd.revents & POLLHUP)
        output = static_cast<AvahiWatchEvent>(output | AVAHI_WATCH_HUP);

    if (pfd.revents & POLLERR)
        output = static_cast<AvahiWatchEvent>(output | AVAHI_WATCH_ERR);

    return output;
}

// static method for C-API
void PollUv::Watch::OnEvent(Handle* handle, int status, int events)
{
    Watch* watch = reinterpret_cast<Watch*>(handle->data);
    watch->OnEvent(status, events);
}


AvahiWatch* PollUv::Watch::New(
    const AvahiPoll* api,
    int fd,
    AvahiWatchEvent event,
    AvahiWatchCallback callback,
    void* userdata
)
{
    auto* poll = static_cast<PollUv*>(api->userdata);
    auto* w = new Watch(poll->GetLoop(), fd, event, callback, userdata);
    return w->GetObj();
}

void PollUv::Watch::Update(AvahiWatch *w, AvahiWatchEvent event)
{
    Watch* watch = reinterpret_cast<Watch*>(w);
    return watch->Update(event);
}

AvahiWatchEvent PollUv::Watch::GetEvents(AvahiWatch* w)
{
    Watch* watch = reinterpret_cast<Watch*>(w);
    return watch->GetEvents();
}

void PollUv::Watch::Free(AvahiWatch* w)
{
    assert(w);
    auto* watch = reinterpret_cast<Watch*>(w);
    delete watch;
}

const AvahiWatchEvent PollUv::Watch::NoEvent = static_cast<AvahiWatchEvent>(0);

/////////////////////////////////////////////////////////////////////////////////
/// \brief PollUv::Timeout implementation
/////////////////////////////////////////////////////////////////////////////////
PollUv::Timeout::Timeout(
    Loop* loop, const timeval* tv, AvahiTimeoutCallback cb, void* userdata
)
    : m_callback(cb)
    , m_userdata(userdata)
{
    uv_timer_init(loop, &m_handle);
    m_handle.data = this;

    Update(tv);
}

PollUv::Timeout::~Timeout()
{
    uv_timer_stop(&m_handle);
}

void PollUv::Timeout::Update(const timeval* tv)
{
    if (!tv)
    {
        uv_timer_stop(&m_handle);
        return;
    }

    uint64_t ms = tv->tv_sec * 1e3 + tv->tv_usec / 1e3;

    uv_timer_start(&m_handle, OnTimeout, ms, 0);
}

AvahiTimeout* PollUv::Timeout::GetObj()
{
    return reinterpret_cast<AvahiTimeout*>(this);
}

void PollUv::Timeout::OnTimeout()
{
    if (m_callback)
        m_callback(GetObj(), m_userdata);
}

void PollUv::Timeout::OnTimeout(Handle* handle)
{
    auto* timeout = reinterpret_cast<Timeout*>(handle->data);
    timeout->OnTimeout();
}

AvahiTimeout* PollUv::Timeout::New(
    const AvahiPoll* api, const timeval* tv, AvahiTimeoutCallback cb, void* user
)
{
    assert(api);
    auto* poll = static_cast<PollUv*>(api->userdata);
    auto* timeout = new Timeout(poll->GetLoop(), tv, cb, user);
    return timeout->GetObj();
}

void PollUv::Timeout::Update(AvahiTimeout* t, const timeval* tv)
{
    assert(t);
    auto* timeout = reinterpret_cast<Timeout*>(t);
    timeout->Update(tv);
}

void PollUv::Timeout::Free(AvahiTimeout* t)
{
    assert(t);
    auto* timeout = reinterpret_cast<Timeout*>(t);
    delete timeout;
}

}
