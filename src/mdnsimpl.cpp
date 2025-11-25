#include "mdnsimpl.hpp"
#include "client.hpp"
#include "pollsimple.hpp"
#include "pollthreaded.hpp"
#ifdef MDNS_BOOST
#include "pollasio.hpp"
#endif
#ifdef MDNS_UV
#include "polluv.hpp"
#endif
#include <mdns/group.hpp>
namespace mdns {

Mdns::Impl::Impl()
    : m_poll(std::make_unique<PollSimple>())
    , m_client(std::make_shared<Client>())
{
}

Mdns::Impl::Impl(InternalThread)
    : m_poll(std::make_unique<PollThreaded>())
    , m_client(std::make_shared<Client>())
{
}

#ifdef MDNS_BOOST
Mdns::Impl::Impl(boost::asio::io_context& io)
    : m_poll(std::make_unique<PollAsio>(io))
    , m_client(std::make_shared<Client>())
{
}
#endif

#ifdef MDNS_UV
Mdns::Impl::Impl(uv_loop_t* loop)
    : m_poll(std::make_unique<PollUv>(loop))
    , m_client(std::make_shared<Client>())
{
}
#endif

std::error_code Mdns::Impl::Connect(
    const ClientFlags& flags,
    const ClientCallback& callback
)
{
    return m_client->Start(*m_poll, flags, callback);
}

std::error_code Mdns::Impl::Run()
{
    return m_poll->Run();
}

void Mdns::Impl::Cancel()
{
    auto CancelAll = [](auto& list){
        for (auto& item : list )
            item.Cancel();
        list.clear();
    };

    CancelAll(m_domainbrowsers);
    CancelAll(m_typebrowsers);
    CancelAll(m_servicebrowsers);
    CancelAll(m_recordbrowsers);
    CancelAll(m_addressresolvers);
    CancelAll(m_hostnameresolvers);
    CancelAll(m_serviceresolvers);

    m_poll->Stop();
}

std::error_code Mdns::Impl::BrowseDomains(
    const DomainInfo& request,
    const DomainBrowserType& type,
    const LookupFlags& flags,
    const DomainCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& browser = m_domainbrowsers.emplace_back(m_client);
    return browser.Start(request, type, flags, callback);
}

std::error_code Mdns::Impl::BrowseTypes(
    const DomainInfo& request,
    const LookupFlags& flags,
    const ServiceTypeCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& browser = m_typebrowsers.emplace_back(m_client);
    return browser.Start(request, flags, callback);
}

std::error_code Mdns::Impl::BrowseServices(
    const TypeInfo& request,
    const LookupFlags& flags,
    const ServiceCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& browser = m_servicebrowsers.emplace_back(m_client);
    return browser.Start(request, flags, callback);
}


std::error_code Mdns::Impl::BrowseRecords(
    const RecordRequest& request,
    const LookupFlags& flags,
    const RecordCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& browser = m_recordbrowsers.emplace_back(m_client);
    return browser.Start(request, flags, callback);
}

std::error_code Mdns::Impl::ResolveAddress(
    const DeviceInfo& device,
    const Address& a,
    const LookupFlags& flags,
    const AddressCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& resolver = m_addressresolvers.emplace_back(m_client);
    return resolver.Start(device, a, flags, callback);
}

std::error_code Mdns::Impl::ResolveHostname(
    const DeviceInfo& device,
    const std::string& hostname,
    const Protocol& aprotocol,
    const LookupFlags& flags,
    const AddressCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    auto& resolver = m_hostnameresolvers.emplace_back(m_client);
    return resolver.Start(device, hostname, aprotocol, flags, callback);
}

std::error_code Mdns::Impl::ResolveService(
    const ServiceInfo& request,
    const Protocol& protocol, // AAAA vs A records.  Careful here.  A records aren't published over IPv6 mDNS.
    const LookupFlags& flags,
    const ResolvedServiceCallback& callback
)
{
    if (m_client->GetState() != ClientState::running)
        return Error::not_connected;

    // Todo: Erase resolver after the handler is invoked
    auto& resolver = m_serviceresolvers.emplace_back(m_client);
    return resolver.Start( request, protocol, flags, callback);
}

Group Mdns::Impl::MakeGroup(GroupCallback callback, const std::string& name, PublishFlags flags)
{
    return Group(m_client, callback, name, flags);
}

}
