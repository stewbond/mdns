#ifndef _67ec35875962c71112e71315fe37b166
#define _67ec35875962c71112e71315fe37b166
#include <mdns/errorcode.hpp>
struct AvahiPoll;

namespace mdns {

class PollIface
{
public:
    virtual const AvahiPoll* GetPoll() const = 0;
    virtual std::error_code Run() = 0;
    virtual std::error_code Stop() = 0;
    virtual void Lock() {};
    virtual void Unlock() {};
};

}

#endif
