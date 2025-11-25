#ifndef _138a1d10b385d55ff799994b0baa0c28
#define _138a1d10b385d55ff799994b0baa0c28

// CPP wrapper around AvahiSimplePoll
#include "polliface.hpp" // PollIface
#include <system_error>  // std::error_code
#include <chrono>        // std::chrono::milliseconds
#include <functional>    // std::function
#include <memory>        // std::unique_ptr

struct AvahiSimplePoll;
struct pollfd;

namespace mdns {

class PollSimple : public PollIface
{
public:
    PollSimple();

    PollSimple(PollSimple&&) = default;
    PollSimple(const PollSimple&) = delete;
    PollSimple& operator=(PollSimple&&) = default;
    PollSimple& operator=(const PollSimple&) = delete;

    const AvahiPoll* GetPoll() const override;
    std::error_code Run() override;
    std::error_code Stop() override;

    // Run forever
    std::error_code loop() noexcept;

    static const std::chrono::milliseconds forever;
    static const std::chrono::milliseconds noblock;

    // Return values:
    //    false = read-again
    //    true = stop (quit or error)
    bool iterate(
        std::error_code& ec, // AvahiError::aborted is expected after poll_quit().  Any other error is abnormal.
        std::chrono::milliseconds timeout // >0 = timeout, 0 = noblock, -1 = forever
    ) noexcept;

    // Throws AvahiException on any abnormal error
    //    AvahiError::aborted results in a 'true' return, not an exception.
    bool iterate(std::chrono::milliseconds timeout);

    // Force blocked call to return (for threaded enviornment)
    void wakeup() noexcept;

    // Allow user to define an alternate poll() implementation.
    // Default is to use system's poll.
    typedef std::function<int(pollfd*,unsigned int, int)> PollFunc;
    void SetPollFunc(PollFunc f);

private:
    std::unique_ptr< AvahiSimplePoll, void(*)(AvahiSimplePoll *)> m_pollobj;
    PollFunc m_pollfunc;

    static int StaticPoll(struct pollfd*,unsigned int, int, void*);
};

}
#endif
