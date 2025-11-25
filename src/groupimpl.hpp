#ifndef _cffc4e7092b173d829713d0036d1d160
#define _cffc4e7092b173d829713d0036d1d160
#include <avahi-client/publish.h>
#include <mdns/group.hpp>
#include <memory>
namespace mdns {

// Consider
class Group::Impl
{
public:
    using Callback = Group::Callback;

    Impl(
        std::shared_ptr<Client> c,
        const std::string& name,
        PublishFlags flags, // Default to PublishFlags::none for end-user
        Callback callback
    );
    Impl(const Impl&)            = delete;
    Impl(Impl&&)                 = default;
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&)      = default;

    std::error_code Commit();
    std::error_code Recommit();

    std::error_code ResetPublic(const std::string& name);

    GroupState GetGroupState();

    bool IsEmpty();
    const std::string& GetName() const;

    // Validates, saves a copy, and calls *Private(...)
    std::error_code AddServicePublic(ServiceRecord);
    std::error_code AddHostPublic(AddressHostnameInfo);
    std::error_code AddRecordPublic(DnsRecord);
    std::error_code UpdateTxtPublic(const ServiceInfo& service, StrList txt);

    void ChangeName(const std::string& name);

private:
    Callback m_callback;
    std::shared_ptr<Client> m_client;
    std::unique_ptr<AvahiEntryGroup, int(*)(AvahiEntryGroup*)> m_ptr;
    std::string m_name;
    PublishFlags m_flags;

    std::list<ServiceRecord>       m_services;
    std::list<AddressHostnameInfo> m_hosts;
    std::list<DnsRecord>           m_records;

    std::error_code MakeGroupPtr();
    std::error_code CheckSetName(const std::string& name);

    // Translate to API, no save, no validation
    // Can be used by reset requests.
    std::error_code AddServicePrivate(const ServiceRecord& service);
    std::error_code AddSubtype(
        const ServiceInfo& service,
        const std::string& subtype
    );
    std::error_code AddHostPrivate(const AddressHostnameInfo&);
    std::error_code AddRecordPrivate(const DnsRecord&);
    std::error_code UpdateTxtPrivate(const ServiceRecord& service);

    std::string NameAlternative(const std::string& name = {});
    std::error_code HandleCollision();
    std::error_code Rename();
    std::error_code Readd();
    std::error_code ResetPrivate();

    static void EntryGroupCallback (
        AvahiEntryGroup* g,
        AvahiEntryGroupState state,
        void *userdata
    );
};

}
#endif
