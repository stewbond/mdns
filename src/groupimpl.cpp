#include "groupimpl.hpp"
#include "structures_priv.hpp"
#include "client.hpp"
#include <mdns/exception.hpp>
#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
namespace mdns {

Group::Impl::Impl(
    std::shared_ptr<Client> c,
    const std::string& name,
    PublishFlags flags,
    Callback callback
)
    : m_callback(callback)
    , m_client(c)
    , m_ptr(nullptr, avahi_entry_group_free)
    , m_name(name)
    , m_flags(flags)
{
}

std::error_code Group::Impl::Commit()
{
    MakeGroupPtr();

    // I suspect this function returns is_empty when appropriate.
    //  No need to check that again here.
    auto err = avahi_entry_group_commit(m_ptr.get());
    return priv::AvahiErrorFromInt(err);
}

std::error_code Group::Impl::ResetPublic(const std::string& newname)
{
    MakeGroupPtr();

    m_name = newname;
    return ResetPrivate();
}

GroupState Group::Impl::GetGroupState()
{
    MakeGroupPtr();

    int state = avahi_entry_group_get_state(m_ptr.get());

    if (state < 0)
        throw Exception(state);

    return from_avahi( static_cast<AvahiEntryGroupState>(state) );
}

bool Group::Impl::IsEmpty()
{
    MakeGroupPtr();

    int result = avahi_entry_group_is_empty( m_ptr.get() );

    if (result < 0)
        throw Exception(result);

    return static_cast<bool>(result);
}

const std::string& Group::Impl::GetName() const
{
    return m_name;
}

std::error_code Group::Impl::AddServicePublic(ServiceRecord service)
{
    MakeGroupPtr();

    if (auto ec = CheckSetName(service.name); ec)
        return ec;

    // Save for later
    auto& svc = m_services.emplace_back(std::move(service));
    svc.name = {}; // clear name in case we need to re-add with a new name

    return AddServicePrivate(svc);
}

std::error_code Group::Impl::AddHostPublic(AddressHostnameInfo record)
{
    MakeGroupPtr();

    auto& host = m_hosts.emplace_back(std::move(record));
    return AddHostPrivate(host);
}

std::error_code Group::Impl::AddRecordPublic(DnsRecord record)
{
    MakeGroupPtr();

    auto& rec = m_records.emplace_back(std::move(record));
    return AddRecordPrivate(rec);
}

std::error_code Group::Impl::UpdateTxtPublic(
    const ServiceInfo& service,
    StrList new_txt
)
{
    if (!service.name.empty() && service.name != m_name)
        return Error::bad_group_name;

    auto areEqual = [](const auto& l, const auto& r)
    {
        return l.domain    == r.domain &&
               l.interface == r.interface &&
               l.protocol  == r.protocol &&
               l.type      == r.type;
    };

    for(auto& svc : m_services)
    {
        if (areEqual(svc, service) )
        {
            svc.txt = std::move(new_txt);
            return UpdateTxtPrivate(svc);
        }
    }

    return Error::not_found;
}

void Group::Impl::ChangeName(const std::string& name)
{
    m_name = name.empty() ? NameAlternative() : name;
}

std::error_code Group::Impl::MakeGroupPtr()
{
    if (m_ptr)
        return Error::ok;

    auto p = avahi_entry_group_new(
        m_client->GetClient(),
        &Group::Impl::EntryGroupCallback,
        this
    );

    if (p == nullptr)
        return m_client->GetLastError();

    m_ptr.reset(p);
    return Error::ok;
}

std::error_code Group::Impl::CheckSetName(const std::string& name)
{
    if (m_name.empty())
        m_name = name;

    if (m_name.empty()) // if still empty
        return Error::bad_group_name;

    if (!name.empty() && name != m_name)
        return Error::bad_group_name;

    return Error::ok;
}

std::error_code Group::Impl::AddServicePrivate(const ServiceRecord& service)
{
    AvahiStringList* txt = to_avahi(service.txt);

    int r = avahi_entry_group_add_service_strlst(
        m_ptr.get(),
        to_avahi(service.interface),
        to_avahi(service.protocol),
        to_avahi(m_flags),
        to_avahi(m_name),
        to_avahi(service.type),
        to_avahi(service.domain),
        to_avahi(service.hostname),
        to_avahi(service.port),
        txt
    );
    std::error_code ec = Error{r};

    avahi_string_list_free(txt);

    for(auto& subtype : service.subtypes)
        ec = ec ? ec : AddSubtype(service, subtype);

    if (ec == Error::collision)
    {
        // Collision with a local service.  Pick a new name.
        return HandleCollision();
    }

    return ec;
}

std::error_code Group::Impl::AddSubtype(
    const ServiceInfo& service,
    const std::string& subtype
    )
{
    int ec = avahi_entry_group_add_service_subtype(
        m_ptr.get(),
        to_avahi(service.interface),
        to_avahi(service.protocol),
        to_avahi(m_flags),
        to_avahi(m_name),
        to_avahi(service.type),
        to_avahi(service.domain),
        to_avahi(subtype)
    );

    return Error{ec};
}

std::error_code Group::Impl::AddHostPrivate(const AddressHostnameInfo& record)
{
    AvahiAddress addr = to_avahi(record.addr);

    int ec = avahi_entry_group_add_address(
        m_ptr.get(),
        to_avahi(record.interface),
        to_avahi(record.protocol),
        to_avahi(m_flags),
        to_avahi(record.hostname),
        &addr
    );

    return Error{ec};
}

std::error_code Group::Impl::AddRecordPrivate(const DnsRecord& record)
{
    int ec = avahi_entry_group_add_record(
        m_ptr.get(),
        to_avahi(record.interface),
        to_avahi(record.protocol),
        to_avahi(m_flags),
        to_avahi(record.name),
        AVAHI_DNS_CLASS_IN,
        to_avahi(record.type),
        record.ttl,
        record.rdata,
        record.size
    );
    return Error{ec};
}

std::error_code Group::Impl::UpdateTxtPrivate(
    const ServiceRecord& service
    )
{
    AvahiStringList* txt = to_avahi(service.txt);

    int ec = avahi_entry_group_update_service_txt_strlst(
        m_ptr.get(),
        to_avahi(service.interface),
        to_avahi(service.protocol),
        to_avahi(m_flags),
        to_avahi(m_name),
        to_avahi(service.type),
        to_avahi(service.domain),
        txt
    );

    avahi_string_list_free(txt);

    return Error{ec};
}

std::string Group::Impl::NameAlternative(const std::string& name)
{
    char* newname = avahi_alternative_service_name(
        name.empty() ? m_name.c_str() : name.c_str()
    );
    std::string s{newname};
    avahi_free(newname);
    return s;
}

std::error_code Group::Impl::HandleCollision()
{
    // if (option to handle collisions automatically)
    // {
    auto ec = Rename();
    return ec ? ec : Recommit();
    // }
}

std::error_code Group::Impl::Rename()
{
    std::error_code ec = Error::ok;

    char* n = avahi_alternative_service_name(m_name.c_str());

    if (n != nullptr)
        m_name = std::string{n};

    avahi_free(n);
    return ec;
}

std::error_code Group::Impl::Readd()
{
    std::error_code ec = Error::ok;

    for (const auto& service : m_services)
        ec = ec ? ec : AddServicePrivate(service);

    for (const auto& host : m_hosts)
        ec = ec ? ec : AddHostPrivate(host);

    for (const auto& record : m_records)
        ec = ec ? ec : AddRecordPrivate(record);

    return ec;
}

std::error_code Group::Impl::Recommit()
{
    auto ec = ResetPrivate();
    ec = ec ? ec : Readd();
    return ec ? ec : Commit();
}

std::error_code Group::Impl::ResetPrivate()
{
    auto ec = avahi_entry_group_reset( m_ptr.get() );
    return Error{ec};
}

void Group::Impl::EntryGroupCallback (
    AvahiEntryGroup* g,
    AvahiEntryGroupState avahi_state,
    void *userdata
)
{
    Group::Impl* self = static_cast<Group::Impl*>(userdata);
    auto state = from_avahi(avahi_state);

    // Name collision with remote service.
    if (state == GroupState::collision)
        self->HandleCollision();

    self->m_callback(self->m_name, state, Error::ok);
}

}
