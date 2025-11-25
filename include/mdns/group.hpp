#ifndef _c80a7bf3ef6ebadd3d0f10fa113c8f32
#define _c80a7bf3ef6ebadd3d0f10fa113c8f32
#include <mdns/errorcode.hpp>
#include <mdns/structures.hpp>
#include <mdns/mdns.hpp>
#include <memory>
namespace mdns {

class Client;

// A 'Group' is a unit that manages publishing several service with the same
//   name. A group can also publish hostnames or arbitrary DNS records.
// It should not be constructed directly.  Use an Mdns context for that.
// If the group ever enters a 'collision' state, an attempt will be made to
//   modify the name, and re-commit the services with a new name.
// A callback can be registered to monitor the state of the group.
//   If there is ever a collision, and a new name is chosen, it can be read in
//   that callback.
class Group
{
public:
    using Callback = GroupCallback;

    Group(const Group&) = delete;
    Group(Group&&);
    ~Group();
    Group& operator=(const Group&) = delete;
    Group& operator=(Group&&);

    // AddService() will add services to the local context, but they won't be
    // published to the network until Commit() is called.  Do this when ready.
    std::error_code Commit();

    // In response to a client reconnection, call this to:
    //   - Reset(),
    //   - re-add all services/hosts/records (with the last known name), and
    //   - Commit() the changes
    std::error_code Recommit();

    // There is less network traffic if you Reset()
    // instead of creating a completely new Group().
    // This will unpublish any services that were registered with this group.
    // The argument newname gives you the opportunity to set a new name in case
    //   you don't like the default one we suggested after a collision
    std::error_code Reset(const std::string& newname = {});

    // An alternative to tracking the group state in a callback is to poll the
    // state with this function.
    GroupState GetGroupState();

    bool IsEmpty();

    const std::string& GetName() const;

    // Args: Service
    //   service.name is only required for the first service added
    //   All subsequent services MUST have the same (or empty) name.
    // If a collision with another local group is detected,
    //   An attempt is made to register with an alternate name.
    //   You can detect this happened if GetName() does not match your latest
    //   service.  All subsequent services should use the new name, or Reset()
    //   and try something new.
    //   If we fail, even with the alternate name, you'll be notified in
    //   std::error_code.
    // Subtypes:
    //   You may add as many subtypes as you like.
    //   They are usually of the form: "_foo._sub._bar._tcp"
    //     for subtype "foo" in protocol "_bar._tcp".
    //   Subtypes just add a PTR record in the DNS that lets us filter for a
    //     specific service.
    //   Subtypes do not appear in type-browsing
    std::error_code AddService(const ServiceRecord& service);

    // Adds a hostname/address pair.
    // Adds A, AAAA, and PTR records to the DNS
    // record.hostname must be a FQDN, probably with a .local TLD
    std::error_code AddHost(const AddressHostnameInfo& record);

    // Adds an arbitrary record to the DNS
    // Advanced use only.  Only use if you know what you're doing.
    std::error_code AddRecord(const DnsRecord& record);

    // You may update the TXT records for an existing service.
    // This causes less disruption to the network than Reset() and adding a
    // new service.
    std::error_code UpdateTxt(
        const ServiceInfo& service,       // Should be identical to an existing service
        const StrList& txt // New TXT
    );

    // If there was a collision, set a new name.
    // If no argument is provided, a new name will be deduced from the old name
    //   using NameAlternative()
    void ChangeName(const std::string& newname = {});

private:
    Group() = delete;

    friend class Mdns::Impl; // Refer to class Mdns to create a group
    explicit Group(
        std::shared_ptr<Client> c,
        Callback callback,
        const std::string& name,
        PublishFlags flags
    );

    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}
#endif
