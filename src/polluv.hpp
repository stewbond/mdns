#ifndef _f3e48d85d2fb2070da906528ffaeea8e
#define _f3e48d85d2fb2070da906528ffaeea8e

#include <uv.h>
#include <avahi-common/watch.h>
#include "polliface.hpp"

namespace mdns {

// Integrates Mdns into libuv's event loop
class PollUv : public PollIface
{
public:
    using Loop = uv_loop_t;

    PollUv(Loop* loop);
    const AvahiPoll* GetPoll() const override;
    std::error_code  Run()           override;
    std::error_code  Stop()          override;
private:
    class Watch;
    class Timeout;

    Loop* GetLoop();

    Loop* m_loop;
    AvahiPoll m_poll;
};

class PollUv::Watch
{
private:
    using Handle = uv_poll_t;

    Watch(Loop*, int, AvahiWatchEvent, AvahiWatchCallback, void*);
    ~Watch();
    AvahiWatch* GetObj();
    AvahiWatchEvent GetEvents();
    void Start(AvahiWatchEvent);
    void Stop();
    void Update(AvahiWatchEvent);


    AvahiWatchEvent CheckForExtras();

    void OnEvent(int status, int events);
    static void OnEvent(Handle* handle, int status, int events);

    static const AvahiWatchEvent NoEvent;
    Handle             m_handle;
    int                m_fd;
    AvahiWatchCallback m_callback;
    void*              m_userdata;
    AvahiWatchEvent    m_watchedEvents;
    AvahiWatchEvent    m_triggeredEvents;


public:
    static AvahiWatch* New(
        const AvahiPoll*, int fd, AvahiWatchEvent, AvahiWatchCallback, void*
    );
    static void Update(AvahiWatch *w, AvahiWatchEvent event);
    static AvahiWatchEvent GetEvents(AvahiWatch *w);
    static void Free(AvahiWatch *w);
};

class PollUv::Timeout
{
    using Handle = uv_timer_t;

    Timeout(Loop* loop, const timeval*, AvahiTimeoutCallback, void*);
    ~Timeout();
    void Update(const timeval*);
    AvahiTimeout* GetObj();
    void OnTimeout();
    static void OnTimeout(Handle* handle);

    Handle m_handle;
    AvahiTimeoutCallback m_callback;
    void* m_userdata;

public:
    static AvahiTimeout* New(
        const AvahiPoll*, const timeval*, AvahiTimeoutCallback, void*
    );
    static void Update(AvahiTimeout*, const timeval*);
    static void Free(AvahiTimeout*);
};

}

#endif
