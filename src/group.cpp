#include <mdns/group.hpp>
#include "groupimpl.hpp"
namespace mdns {

Group::Group(
    std::shared_ptr<Client> c,
    Callback callback,
    const std::string& name,
    PublishFlags flags
)
    : m_impl(std::make_unique<Group::Impl>(c, name, flags, callback))
{
}

Group::Group(Group&&) = default;
Group::~Group() = default;
Group& Group::operator=(Group&&) = default;

std::error_code Group::Commit()
{
    return m_impl->Commit();
}

std::error_code Group::Recommit()
{
    return m_impl->Recommit();
}

std::error_code Group::Reset(const std::string& name)
{
    return m_impl->ResetPublic(name);
}

GroupState Group::GetGroupState()
{
    return m_impl->GetGroupState();
}

bool Group::IsEmpty()
{
    return m_impl->IsEmpty();
}

const std::string& Group::GetName() const
{
    return m_impl->GetName();
}

std::error_code Group::AddService(const ServiceRecord& service)
{
    return m_impl->AddServicePublic(service);
}

std::error_code Group::AddHost(const AddressHostnameInfo& record)
{
    return m_impl->AddHostPublic(record);
}

std::error_code Group::AddRecord(const DnsRecord& record)
{
    return m_impl->AddRecordPublic(record);
}

std::error_code Group::UpdateTxt(
    const ServiceInfo& service,
    const StrList& new_txt
)
{
    return m_impl->UpdateTxtPublic(service, new_txt);
}

void Group::ChangeName(const std::string& newname)
{
    return m_impl->ChangeName(newname);
}

}
