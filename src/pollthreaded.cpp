#include "pollthreaded.hpp"
#include <avahi-common/thread-watch.h>
namespace mdns {

PollThreaded::PollThreaded()
    : m_pollobj(avahi_threaded_poll_new(), avahi_threaded_poll_free)
{
}

const AvahiPoll* PollThreaded::GetPoll() const
{
    return avahi_threaded_poll_get(m_pollobj.get());
}

std::error_code PollThreaded::Run()
{
    return priv::AvahiErrorFromInt(
        avahi_threaded_poll_start(m_pollobj.get())
    );
}

std::error_code PollThreaded::Stop()
{
    Lock();
    return priv::AvahiErrorFromInt(
        avahi_threaded_poll_stop(m_pollobj.get())
    );
    Unlock();
}

void PollThreaded::Lock()
{
    avahi_threaded_poll_lock(m_pollobj.get());
}

void PollThreaded::Unlock()
{
    avahi_threaded_poll_unlock(m_pollobj.get());
}

}
