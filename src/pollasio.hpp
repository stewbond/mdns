#ifndef _f1540e3537bffd2d74757f1db61b0574
#define _f1540e3537bffd2d74757f1db61b0574
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/posix/descriptor.hpp>
#include <avahi-common/watch.h>
#include "polliface.hpp"

namespace mdns {

// Integrate Mdns into boost::asio's event loop
class PollAsio : public PollIface
{
public:
    using ErrorCode = boost::system::error_code;
    using IoContext = boost::asio::io_context;
    using Strand    = boost::asio::strand<IoContext::executor_type>;

    PollAsio(IoContext& io);
    const AvahiPoll* GetPoll() const override;
    std::error_code  Run()           override;
    std::error_code  Stop()          override;
    std::error_code Post(std::function<void()>) override;
private:
    class Watch;
    class Timeout;

    Strand& GetIo();

    Strand    m_io;
    AvahiPoll m_poll;
};

class PollAsio::Watch
{
    using Descriptor = boost::asio::posix::stream_descriptor;

    Watch(Strand&, int, AvahiWatchEvent, AvahiWatchCallback, void*);
    void            Update(AvahiWatchEvent event);
    void            Wait(AvahiWatchEvent event);
    AvahiWatch*     GetObj();
    AvahiWatchEvent GetEvents();
    AvahiWatchEvent CheckForExtras();
    void            OnEvent(AvahiWatchEvent, const ErrorCode&);

    static const AvahiWatchEvent NoEvent;
    Descriptor         m_fd;
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

class PollAsio::Timeout
{
    using Timer = boost::asio::steady_timer;

    Timeout(Strand&, const timeval*, AvahiTimeoutCallback, void*);
    void          Update(const timeval*);
    AvahiTimeout* GetObj();
    void          OnTimeout(const ErrorCode& ec);

    Timer                m_timer;
    AvahiTimeoutCallback m_callback;
    void*                m_userdata;

public:
    static AvahiTimeout* New(
        const AvahiPoll*, const timeval*, AvahiTimeoutCallback, void*
    );
    static void Update(AvahiTimeout*, const timeval*);
    static void Free(AvahiTimeout*);
};

}

#endif
