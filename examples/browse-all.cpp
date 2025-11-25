#include <mdns/mdns.hpp>
#include <mdns/group.hpp>
#include <iostream>
#include <boost/asio.hpp>

using namespace std::placeholders;
template<class T> bool list_has(const std::list<T>& list, const T& item)
{
    return std::find(list.begin(), list.end(), item) != list.end();
}

void inline Report(std::string prefix, std::error_code ec)
{
    std::cout << "[" << prefix << "] ";
    if (ec)
        std::cout << "error: " << ec.message() << std::endl;
    else
        std::cout << "success" << std::endl;
}

using namespace mdns;

class TheApp
{
public:
    TheApp();
    void Run();

private:
    void OnClient(ClientState);
    void OnDomain(         BrowserEvent , DomainInfo         , ResultFlags, std::error_code);
    void OnServiceType(    BrowserEvent , TypeInfo           , ResultFlags, std::error_code);
    void OnService(        BrowserEvent , ServiceInfo        , ResultFlags, std::error_code);
    void OnResolvedService(ResolverEvent, ResolvedServiceInfo, ResultFlags, std::error_code);
    void Browse();

    boost::asio::io_context m_io;
    Mdns m_dns;
    std::list<Group> m_groups;
    std::list<std::string> m_domainCache;
};

TheApp::TheApp()
    : m_dns(m_io)
{}

void TheApp::Run()
{
    auto ec = m_dns.Connect(
        ClientFlags::no_fail,
        std::bind(&TheApp::OnClient, this, std::placeholders::_1)
    );

    Report("Client::start", ec);

    boost::asio::steady_timer timer(m_io, std::chrono::seconds(5));
    timer.async_wait([&](auto){ m_io.stop(); });

    m_io.run();
}

void TheApp::OnClient(ClientState state)
{
    std::cout << "[Client::callback] " << state << std::endl;

    switch(state)
    {
    case ClientState::running:
        Browse();
        break;
    case ClientState::failure:
        m_dns.Cancel();
        break;
    case ClientState::collision:
    case ClientState::connecting:
    case ClientState::registering:
        break;
    }
}

void TheApp::OnDomain(
    BrowserEvent event,
    DomainInfo result,
    ResultFlags flags,
    std::error_code ec
)
{
    std::cout << "[DomainBrowser::callback] "
              << event
              << " iface:" << result.interface
              << " proto:" << result.protocol
              << " domain:" << result.domain
              << " flags:" << flags;

    if (ec)
        std::cout << "error:" << ec.message();

    if (event == BrowserEvent::new_ &&
        !list_has(m_domainCache, result.domain))
    {
        // Browsing 'sim.local' will also return 'sim.local'
        m_domainCache.push_back(result.domain);

        m_dns.BrowseDomains(
            result,
            DomainBrowserType::browse,
            LookupFlags::none,
            std::bind(&TheApp::OnDomain, this, _1, _2, _3, _4)
        );
    }

    std::cout << std::endl;
}

void TheApp::OnService(
    BrowserEvent event,
    ServiceInfo result,
    ResultFlags flags,
    std::error_code ec
)
{
    std::cout << "[ServiceBrowser::callback] "
              << event
              << " iface:" << result.interface
              << " proto:" << result.protocol
              << " name:'" << result.name
              << "' type:" << result.type
              << " domain:" << result.domain;

    if (ec)
        std::cout << "error:'" << ec.message() << "'";

    std::cout << std::endl;

    if (event == BrowserEvent::new_)
    {
        m_dns.ResolveService(
            std::move(result),
            Protocol::unspec,
            LookupFlags::none,
            std::bind(&TheApp::OnResolvedService, this, _1, _2, _3, _4)
            );
    }
}

void TheApp::OnServiceType(
    BrowserEvent event,
    TypeInfo result,
    ResultFlags flags,
    std::error_code ec
)
{
    std::cout << "[TypeBrowser::callback] "
              << event
              << " iface:" << result.interface
              << " proto:" << result.protocol
              << " type:" << result.type
              << " domain:" << result.domain;

    if (ec)
        std::cout << "error:'" << ec.message() << "'";
    std::cout << std::endl;

    if (event == BrowserEvent::new_)
        m_dns.BrowseServices(
            result,
            LookupFlags::none,
            std::bind(&TheApp::OnService, this, _1, _2, _3, _4)
        );
}

void TheApp::OnResolvedService(
    ResolverEvent event,
    ResolvedServiceInfo result,
    ResultFlags,
    std::error_code ec
)
{
    std::cout << "[Resolver::callback] " << event;

    std::string proto;
    switch (result.protocol)
    {
    case Protocol::ipv4  : proto = "A   "; break;
    case Protocol::ipv6  : proto = "AAAA"; break;
    case Protocol::unspec: proto = "?   "; break;
    }

    std::cout << " iface:"    << result.interface.value()
              << " proto:"    << proto
              << " name:'"    << result.name
              << "' type:"    << result.type
              << " domain:"   << result.domain
              << " hostname:" << result.hostname
              << " address:"  << result.addr << ":" << result.port;

    if (!result.txt.empty())
        std::cout << " txt: ";
    for(auto& t : result.txt)
        std::cout << '"' << t << "\" ";

    if (ec)
        std::cout << " error: '" << ec.message() << "'";

    std::cout << std::endl;
}

void TheApp::Browse()
{
    m_dns.BrowseTypes(
        { DeviceInfo{ .interface = 1} },
        LookupFlags::none,
        std::bind(&TheApp::OnServiceType, this, _1, _2, _3, _4)
    );

    m_dns.BrowseDomains(
        { DeviceInfo{ .interface = 1} }, // Default domain info
        DomainBrowserType::browse,
        LookupFlags::none,
        std::bind(&TheApp::OnDomain, this, _1, _2, _3, _4)
    );
}


int main()
{
    TheApp app;
    app.Run();
    return 0;
}
