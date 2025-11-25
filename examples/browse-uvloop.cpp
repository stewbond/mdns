#include <uv.h>
#include <mdns/mdns.hpp>
#include <mdns/group.hpp>
#include <iostream>

using namespace mdns;

void OnResolve(
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

void OnService(
    Mdns& dns,
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
        dns.ResolveService(
            std::move(result),
            Protocol::unspec,
            LookupFlags::none,
            &OnResolve
        );
    }
    else if (event == BrowserEvent::all_for_now)
    {
        uv_stop(uv_default_loop());
    }
}

void OnClient(Mdns& dns, ClientState state)
{
    std::cout << "[Client::callback] " << state << std::endl;

    switch(state)
    {
    case ClientState::running:
        dns.BrowseServices(
            {.type = "_http._tcp"}, // Must define a type to look for.
            LookupFlags::none,
            [&dns](auto evt, auto result, auto flag, auto err){
                OnService(dns, evt, result, flag, err);
            }
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
    uv_loop_t loop;
    uv_loop_init(&loop);


    Mdns dns(&loop);

    dns.Connect(
        ClientFlags::no_fail,
        [&](auto evt){ OnClient(dns, evt); }
    );

    uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);
}
