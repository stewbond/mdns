#include <mdns/mdns.hpp>
#include <iostream>
#include <boost/asio.hpp>

using namespace mdns;

void OnDomain(
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

    std::cout << std::endl;
}


void OnClient(Mdns& dns, ClientState state)
{
    std::cout << "[Client::callback] " << state << std::endl;

    switch(state)
    {
    case ClientState::running:
        dns.BrowseDomains(
            {}, // Default domain info
            DomainBrowserType::browse,
            LookupFlags::none,
            &OnDomain
        );
        break;
    case ClientState::failure:
        dns.Cancel();
        break;
    case ClientState::collision:
    case ClientState::connecting:
    case ClientState::registering:
        break;
    }
}

int main()
{
    // Using external io_context
    boost::asio::io_context io;
    Mdns dns;

    dns.Connect( ClientFlags::no_fail, [&dns](auto evt){ OnClient(dns, evt);} );

    boost::asio::steady_timer timer(io, std::chrono::seconds(5));
    timer.async_wait( [&](const std::error_code&){
        dns.Cancel();
        io.stop();
    });

    io.run();

    return 0;
}
