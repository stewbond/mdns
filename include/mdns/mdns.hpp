#ifndef _5fe2af04622dc09f94c7da8416bee148
#define _5fe2af04622dc09f94c7da8416bee148
#ifdef MDNS_BOOST
#include <boost/asio/io_context.hpp>
#endif
#ifdef MDNS_UV
struct uv_loop_s;
typedef struct uv_loop_s uv_loop_t;
#endif
#include <mdns/errorcode.hpp>
#include <mdns/structures.hpp>
#include <memory>
namespace mdns {

class Group;

// Multicast DNS handler
// Main object in this library.
class Mdns
{
public: // Ctor/Dtor

    struct InternalThread{};

    // Default consturctor uses an internal poll object and blocks on Run()
    // All Browse/Resolve* functions should return immediately.
    Mdns();

    // InternalThread constructor uses an internal poll object
    //    which handles events in a dedicated thread.
    //    All callbacks occur in that thread, so consider thread-protection
    //    Calls to Browse/Resolve* functions may block for as long as it
    //    takes to lock the underlying resources and post the request.
    //    Callbacks will hold open this lock.
    Mdns(InternalThread);

#ifdef MDNS_BOOST
    // Runs all async operations in the threads supplied by asio's io_context.
    // The avahi backend itself is not thread-safe, but all operations are done
    //     in a single strand, so you don't need to worry about protecting the
    //     internals.
    Mdns(boost::asio::io_context&);
#endif

#ifdef MDNS_UV
    // Runs all operations in the libuv event loop defined.
    // No explicit multi-threading protection is active.
    Mdns(uv_loop_t*);
#endif

    // Avahi supports glib and qt event loops too.
    // Willing to support if requested.

    ~Mdns();


public: // Client functions

    // Connect to avahi-daemon
    std::error_code Connect(
        const ClientFlags& flags,
        const ClientCallback& callback = {}
    );

    // Starts the loop of the Poll interface.
    // If created with an internal poll object,
    //     this will block forever
    // If created with a threaded poll object,
    //     this will spawn a thread and return
    // If created with a boost::asio::io_context
    //     this will call context.run() and block.
    //     In most boost::asio cases, you don't want to call this function
    //     because you'll probably do that on the context directly elsewhere.
    // If created with uv_loop_t, this will run the uv loop and block.
    //     You probably want to handle this externally to maintain control.
    std::error_code Run();

    // Stops all current browsers and resolvers.
    void Cancel();

    // Stops all current browsers and resolvers, then
    // If using an InternalThread, that thread will be stopped.
    // If using an interal poll object,
    //   call this from a callback for Run() to return
    //   To restart the event loop, call Run() again.
    // If using a boost::asio::io_context, all browsing and resolving operations
    //   will be cancelled, but the io_context is left alone.
    void Stop();

public: // Lookup functions

    // Find all domains under a certain domain.
    // By default 'local' always exists and won't be returned.
    // If you search for an existing subdomain, that domain will be returned here too.
    std::error_code BrowseDomains(
        const DomainInfo& request,
        const DomainBrowserType& type,
        const LookupFlags& flags,
        const DomainCallback& callback
    );

    // Find all types in a given domain
    // Does not return sub-types... those are just aliases/filters for main types.
    // Most apps know which types they want to use, so this isn't usually needed.
    std::error_code BrowseTypes(
        const DomainInfo& request,
        const LookupFlags& flags,
        const ServiceTypeCallback& callback
    );

    // Find services for a given type
    // May set type as a sub-type to filter.
    // Returns named services
    std::error_code BrowseServices(
        const TypeInfo& request,
        const LookupFlags& flags,
        const ServiceCallback& callback
    );

    // Find all records of a specified type for a given service name
    //  Ex: you can look for all PTR records associated with "to-ig"
    // Very low-level and probably not very useful.
    std::error_code BrowseRecords(
        const RecordRequest& request,
        const LookupFlags& flags,
        const RecordCallback& callback
    );

    // Gets hostname from address (reverse DNS)
    std::error_code ResolveAddress(
        const DeviceInfo& device,
        const Address& a,
        const LookupFlags& flags,
        const AddressCallback& callback
    );

    // Gets address from Hostname
    std::error_code ResolveHostname(
        const DeviceInfo& device,
        const std::string& hostname,
        const Protocol& aprotocol,
        const LookupFlags& flags,
        const AddressCallback& callback
    );

    // Given a service type+name, returns resolved info (IP, port, TXT)
    std::error_code ResolveService(
        const ServiceInfo& request,
        const Protocol& protocol, // AAAA vs A records.  Careful here.  A records aren't published over IPv6 mDNS.
        const LookupFlags& flags,
        const ResolvedServiceCallback& callback
    );

public: // Publishing functions

    // Make a group to publish things in the MDNS world,
    //   - Each group must have a unique name.  This name will be entered into the DNS.
    //   - A service may support multiple types or ports.
    //   - Each type or port combo is technically a different service, but they all share a name.
    //   - Services that share a name must be in the same group
    // Arguments:
    //   - callback:  Invoked whenever the group state changes.
    //      - GroupState::established: Do all add/update/commit operations in this state
    //      - GroupState::collision  : Name collision with a remote service.
    //                                 This library will automatically choose a new name and try to re-establish
    //      - GroupState::failure    : Something went horribly wrong, probably delete this group and start again.
    //      - GroupState::uncommitted: Missing a connection to the underlying avahi-daemon.
    //                                 Wait until it comes back up and try again.
    //      - GroupState::registering: Almost ready.
    //   - name:  Defines the name used by all services in this group
    //      - Add* methods will return Error::bad_group_name if a service is given with a different (non-empty) name
    //      - If empty, it will be inferred by the first service that gets added
    //   - flags: Advanced use only
    Group MakeGroup(
        GroupCallback callback,
        const std::string& name = {},
        PublishFlags flags = PublishFlags::none
    );

public: // Implementation class forward-declaration (must be public)
    class Impl;
private:
    std::unique_ptr<Impl> m_impl;
};

}
#endif
