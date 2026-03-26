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
    m_userClientCallback = callback;
    return m_client->Start(
        *m_poll,
        flags,
        std::bind(&Mdns::Impl::OnClientState, this, std::placeholders::_1),
        std::bind(&Mdns::Impl::OnClientDisconnect, this)
    );
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
}

void Mdns::Impl::Stop()
{
    Cancel();
    m_poll->Stop();
}

std::error_code Mdns::Impl::BrowseDomains(
    const DomainInfo& request,
    const DomainBrowserType& type,
    const LookupFlags& flags,
    const DomainCallback& callback
)
{
    return BrowseDomains({request, type, flags, callback});
}

std::error_code Mdns::Impl::BrowseDomains(DomainBrowser::StartParams&& params)
{
    if (!m_client->IsConnected())
    {
        m_browserQueue.emplace( std::move(params)  );
        return Error::not_connected; // Todo:  Save details in a browser queue.  Start processing when we enter the "connected" state.
    }

    auto& browser = m_domainbrowsers.emplace_back(m_client);
    return browser.Start( std::move(params) );
}

std::error_code Mdns::Impl::BrowseTypes(
    const DomainInfo& request,
    const LookupFlags& flags,
    const ServiceTypeCallback& callback
)
{
    return BrowseTypes({request, flags, callback});
}

std::error_code Mdns::Impl::BrowseTypes(ServiceTypeBrowser::StartParams&& params)
{
    if (!m_client->IsConnected())
    {
        m_browserQueue.emplace( std::move(params) );
        return Error::not_connected;
    }

    auto& browser = m_typebrowsers.emplace_back(m_client);
    return browser.Start( std::move(params) );
}

std::error_code Mdns::Impl::BrowseServices(
    const TypeInfo& request,
    const LookupFlags& flags,
    const ServiceCallback& callback
)
{
    return BrowseServices({request, flags, callback});
}

std::error_code Mdns::Impl::BrowseServices(ServiceBrowser::StartParams&& params)
{
    if (!m_client->IsConnected())
    {
        m_browserQueue.emplace( std::move(params) );
        return Error::not_connected;
    }

    auto& browser = m_servicebrowsers.emplace_back(m_client);
    return browser.Start( std::move(params) );
}

std::error_code Mdns::Impl::BrowseRecords(
    const RecordRequest& request,
    const LookupFlags& flags,
    const RecordCallback& callback
)
{
    return BrowseRecords({request, flags, callback});
}

std::error_code Mdns::Impl::BrowseRecords(RecordBrowser::StartParams&& params)
{
    if (!m_client->IsConnected())
    {
        m_browserQueue.emplace( std::move(params) );
        return Error::not_connected;
    }

    auto& browser = m_recordbrowsers.emplace_back(m_client);
    return browser.Start( std::move(params) );
}

std::error_code Mdns::Impl::ResolveAddress(
    const DeviceInfo& device,
    const Address& a,
    const LookupFlags& flags,
    const AddressCallback& callback
)
{
    if (!m_client->IsConnected())
        return Error::not_connected;

    // Todo: Erase resolver after the handler is invoked
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
    if (!m_client->IsConnected())
        return Error::not_connected;

    // Todo: Erase resolver after the handler is invoked
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
    if (!m_client->IsConnected())
        return Error::not_connected;

    auto& resolver = m_serviceresolvers.emplace_back(m_client);
    return resolver.Start( request, protocol, flags,
        [this, cb=callback](auto evt, auto info, auto flags, auto err){

            cb(evt,std::move(info), flags, err);
            // Service resolver is on the stack right now (we are called by it)
            // It is not safe to clean it up yet.
            // So we post() it to the event loop to be processed later.
            m_poll->Post(std::bind(&Mdns::Impl::CleanupServiceResolvers, this));
        }
    );
}

Group Mdns::Impl::MakeGroup(GroupCallback callback, const std::string& name, PublishFlags flags)
{
    return Group(m_client, callback, name, flags);
}

void Mdns::Impl::OnClientState(ClientState state)
{
    if ( Client::IsConnected(state) )
    {
        ProcessBrowserQueue();
    }
    if (m_userClientCallback)
        m_userClientCallback(state);
}

void Mdns::Impl::OnClientDisconnect()
{
    auto SaveBrowser = [this](const auto& list){
        for (const auto& item : list )
            m_browserQueue.emplace( item.GetStartParams() );
    };

    SaveBrowser(m_domainbrowsers);
    SaveBrowser(m_typebrowsers);
    SaveBrowser(m_servicebrowsers);
    SaveBrowser(m_recordbrowsers);

    Cancel();
}

void Mdns::Impl::ProcessBrowserQueue()
{
    while (!m_browserQueue.empty())
    {
        auto& startParams = m_browserQueue.front();
        std::visit( [this](auto&& arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_same_v<T,DomainBrowser::StartParams>)
            {
                BrowseDomains(std::move(arg));
            }
            else if constexpr(std::is_same_v<T,ServiceTypeBrowser::StartParams>)
            {
                BrowseTypes(std::move(arg));
            }
            else if constexpr(std::is_same_v<T,ServiceBrowser::StartParams>)
            {
                BrowseServices(std::move(arg));
            }
            else if constexpr(std::is_same_v<T,RecordBrowser::StartParams>)
            {
                BrowseRecords(std::move(arg));
            }
        }, startParams);
        m_browserQueue.pop();
    }
}

void Mdns::Impl::CleanupServiceResolvers()
{
    for (auto it = m_serviceresolvers.begin();
         it != m_serviceresolvers.end(); )
    {
        if (it->IsResolved())
        {
            it->Cancel();
            it = m_serviceresolvers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

}
