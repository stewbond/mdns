#include "polliface.hpp"
#include <avahi-common/watch.h>

using namespace mdns;

std::error_code PollIface::Post(std::function<void()> work)
{
    // By default "Post" will create work with a timer that expires immediately.
    //  That should defer this work to a future execution in the event loop.
    Lock();

    struct PostContext
    {
        std::function<void()> work;
        const AvahiPoll* api;
    };

    const AvahiPoll* api = GetPoll();
    auto* ctx = new PostContext{ std::move(work), api};

    struct timeval tv = {0, 0}; // Add to queue immediately

    api->timeout_new(
        api,
        &tv,
        [](AvahiTimeout* t, void* userdata) {
            auto* ctx = static_cast<PostContext*>(userdata);
            ctx->work();
            ctx->api->timeout_free(t);
            delete ctx;
        },
        ctx
    );

    Unlock();
    return Error::ok;
}

void PollIface::Lock()
{
    // no-op
}

void PollIface::Unlock()
{
    // no-op
}
