#ifndef _cef56ade557f6e4271472b8585cadf2b
#define _cef56ade557f6e4271472b8585cadf2b

#include "polliface.hpp"  // PollIface
#include <system_error>   // std::error_code
#include <memory>         // std::unique_ptr

struct AvahiThreadedPoll;

namespace mdns {

// CPP wrapper around AvahiThreadedPoll
class PollThreaded : public PollIface
{
public:
    PollThreaded();

    PollThreaded(PollThreaded&&)                 = default;
    PollThreaded(const PollThreaded&)            = delete;
    PollThreaded& operator=(PollThreaded&&)      = default;
    PollThreaded& operator=(const PollThreaded&) = delete;

    const AvahiPoll* GetPoll() const override;
    std::error_code Run() override;

    // Request that the main loop quits
    // Next call to poll_iterate returns with AvahiError::abnormal
    std::error_code Stop() override;

    virtual void Lock() override;
    virtual void Unlock() override;

private:
    std::unique_ptr<
        AvahiThreadedPoll,
        void(*)(AvahiThreadedPoll*)
    > m_pollobj;
};

}
#endif
