#include <mdns/mdns.hpp>
#include <mdns/group.hpp>
#include "mdnsimpl.hpp"

namespace mdns {

Mdns::Mdns()
    : m_impl(std::make_unique<Impl>())
{}

Mdns::Mdns(InternalThread)
    : m_impl(std::make_unique<Impl>(InternalThread{}))
{}

#ifdef MDNS_BOOST
Mdns::Mdns(boost::asio::io_context& io)
    : m_impl(std::make_unique<Impl>(io))
{}
#endif

#ifdef MDNS_UV
Mdns::Mdns(uv_loop_t* loop)
    : m_impl(std::make_unique<Impl>(loop))
{}
#endif

Mdns::~Mdns() = default;

std::error_code Mdns::Connect(
    const ClientFlags& flags,
    const ClientCallback& callback
)
{
    return m_impl->Connect(flags, callback);
}

std::error_code Mdns::Run()
{
    return m_impl->Run();
}

void Mdns::Cancel()
{
    return m_impl->Cancel();
}

std::error_code Mdns::BrowseDomains(
    const DomainInfo& request,
    const DomainBrowserType& type,
    const LookupFlags& flags,
    const DomainCallback& callback
)
{
    return m_impl->BrowseDomains(request, type, flags, callback);
}

std::error_code Mdns::BrowseTypes(
    const DomainInfo& request,
    const LookupFlags& flags,
    const ServiceTypeCallback& callback
)
{
    return m_impl->BrowseTypes(request, flags, callback);
}


std::error_code Mdns::BrowseServices(
    const TypeInfo& request,
    const LookupFlags& flags,
    const ServiceCallback& callback
)
{
    return m_impl->BrowseServices(request, flags, callback);
}


std::error_code Mdns::BrowseRecords(
    const RecordRequest& request,
    const LookupFlags& flags,
    const RecordCallback& callback
)
{
    return m_impl->BrowseRecords(request, flags, callback);
}

std::error_code Mdns::ResolveAddress(
    const DeviceInfo& device,
    const Address& a,
    const LookupFlags& flags,
    const AddressCallback& callback
)
{
    return m_impl->ResolveAddress(device, a, flags, callback);
}

std::error_code Mdns::ResolveHostname(
    const DeviceInfo& device,
    const std::string& hostname,
    const Protocol& aprotocol,
    const LookupFlags& flags,
    const AddressCallback& callback
)
{
    return m_impl->ResolveHostname(
        device, hostname, aprotocol, flags, callback
    );
}

std::error_code Mdns::ResolveService(
    const ServiceInfo& request,
    const Protocol& protocol, // AAAA vs A records.  Careful here.  A records aren't published over IPv6 mDNS.
    const LookupFlags& flags,
    const ResolvedServiceCallback& callback
)
{
    return m_impl->ResolveService(request, protocol, flags, callback);
}

Group Mdns::MakeGroup(
    GroupCallback callback,
    const std::string& name,
    PublishFlags flags
)
{
    return m_impl->MakeGroup(callback, name, flags);
}

}
