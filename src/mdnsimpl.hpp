#ifndef _8b3ef8ad0762091763b349b7b8820641
#define _8b3ef8ad0762091763b349b7b8820641
#include <mdns/mdns.hpp>
#include <variant>
#include <queue>
#include "polliface.hpp"
#include "servicetypebrowser.hpp"
#include "servicebrowser.hpp"
#include "domainbrowser.hpp"
#include "recordbrowser.hpp"
#include "addressresolver.hpp"
#include "hostnameresolver.hpp"
#include "serviceresolver.hpp"

namespace mdns {

class Client;

class Mdns::Impl
{
public:
    Impl();

    Impl(InternalThread);

#ifdef MDNS_BOOST
    Impl(boost::asio::io_context&);
#endif
#ifdef MDNS_UV
    Impl(uv_loop_t*);
#endif

    std::error_code Connect(
        const ClientFlags& flags,
        const ClientCallback& callback
    );

    std::error_code Run();

    void Cancel();
    void Stop();

    std::error_code BrowseDomains(
        const DomainInfo& request,
        const DomainBrowserType& type,
        const LookupFlags& flags,
        const DomainCallback& callback
    );
    std::error_code BrowseDomains(DomainBrowser::StartParams&&);

    std::error_code BrowseTypes(
        const DomainInfo& request,
        const LookupFlags& flags,
        const ServiceTypeCallback& callback
    );
    std::error_code BrowseTypes(ServiceTypeBrowser::StartParams&&);

    std::error_code BrowseServices(
        const TypeInfo& request,
        const LookupFlags& flags,
        const ServiceCallback& callback
    );
    std::error_code BrowseServices(ServiceBrowser::StartParams&&);

    std::error_code BrowseRecords(
        const RecordRequest& request,
        const LookupFlags& flags,
        const RecordCallback& callback
    );
    std::error_code BrowseRecords(RecordBrowser::StartParams&&);

    std::error_code ResolveAddress(
        const DeviceInfo& device,
        const Address& a,
        const LookupFlags& flags,
        const AddressCallback& callback
    );

    std::error_code ResolveHostname(
        const DeviceInfo& device,
        const std::string& hostname,
        const Protocol& aprotocol,
        const LookupFlags& flags,
        const AddressCallback& callback
    );

    std::error_code ResolveService(
        const ServiceInfo& request,
        const Protocol& protocol,
        const LookupFlags& flags,
        const ResolvedServiceCallback& callback
    );

    Group MakeGroup(
        GroupCallback callback,
        const std::string& name,
        PublishFlags flags
    );

private:
    void OnClientState(ClientState); // Local callback
    void OnClientDisconnect(); // Callback for when the client disconnects
    void ProcessBrowserQueue();

    std::unique_ptr<PollIface>    m_poll;
    std::shared_ptr<Client>       m_client;
    std::list<DomainBrowser>      m_domainbrowsers;
    std::list<ServiceTypeBrowser> m_typebrowsers;
    std::list<ServiceBrowser>     m_servicebrowsers;
    std::list<RecordBrowser>      m_recordbrowsers;
    std::list<AddressResolver>    m_addressresolvers;
    std::list<HostNameResolver>   m_hostnameresolvers;
    std::list<ServiceResolver>    m_serviceresolvers;

    typedef std::variant<
        DomainBrowser::StartParams,
        ServiceTypeBrowser::StartParams,
        ServiceBrowser::StartParams,
        RecordBrowser::StartParams
    > BrowserParams;

    std::queue<BrowserParams> m_browserQueue;

    ClientCallback m_userClientCallback;
};

}

#endif
