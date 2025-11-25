#include <mdns/mdns.hpp>
#include <mdns/group.hpp>
#include <iostream>

using namespace mdns;
using namespace std::placeholders;
void inline Report(std::string prefix, std::error_code ec)
{
    std::cout << "[" << prefix << "] ";
    if (ec)
        std::cout << "error: " << ec.message() << std::endl;
    else
        std::cout << "success" << std::endl;
}

class TheApp
{
public:
    TheApp() : m_dns(m_io){};
    void Run()
    {
        auto ec = m_dns.Connect(
            ClientFlags::no_fail,
            std::bind(&TheApp::OnClient, this, _1)
        );

        Report("Client::start", ec);

        {// Good service
            auto group = m_dns.MakeGroup(
                std::bind(&TheApp::OnGroup, this, _1, _2, _3)
            );

            ec = group.AddService({
                IfIndex::any(),
                Protocol::ipv4,
                {},                         // domain
                "_example._tcp",            // type
                "Example Service",          // name
                {},                         // hostname (empty = localhost)
                13668,                      // port
                {"txtExample=value"},       // TXT record
                {"_qtg._sub._example._tcp"} // Subtypes
            });
            Report("Group::AddService", ec);

            ec = group.Commit();
            Report("Group::Commit", ec);

            m_groups.emplace_back(std::move(group));
        }

        {// Bad service
            auto group = m_dns.MakeGroup(
                std::bind(&TheApp::OnBadGroup, this, _1, _2, _3)
            );

            ec = group.AddService({
                IfIndex::any(),
                Protocol::ipv4,
                {},
                "_example._tcp",
                "Example Service",          // Same name
                {},
                13669,                      // Different port
                {"txtExample=another value"},
                {}
            });
            Report("BadGroup::AddService", ec);

            ec = group.Commit();
            Report("BadGroup::Commit", ec);

            m_groups.emplace_back(std::move(group));
        }

        m_io.run();
    }

private:
    void OnClient(ClientState state)
    {
        std::cout << "[Client::callback] " << state << std::endl;

        switch(state)
        {
        case ClientState::running:
            for (auto& group : m_groups)
                group.Recommit();
            break;
        case ClientState::failure:
            m_dns.Cancel();
            break;
        case ClientState::collision:
        case ClientState::connecting:
            for (auto& group : m_groups)
                group.Reset();
            break;
        case ClientState::registering:
            break;
        }
    }

    void OnGroup(std::string name, GroupState state, std::error_code ec)
    {
        std::cout << "[Group::callback] '" << name << "':" << state;
        if (ec)
            std::cout << " Error: " << ec.message();
        std::cout << std::endl;
    }

    void OnBadGroup(std::string name, GroupState state, std::error_code ec)
    {
        std::cout << "[BadGroup::callback] '" << name << "':" << state;
        if (ec)
            std::cout << " Error: " << ec.message();
        std::cout << std::endl;

        // Exiting the test now, but if you want to keep this on the network,
        //     keep this program running.
        if (state == GroupState::established)
            m_io.stop();
    }

    boost::asio::io_context m_io;
    Mdns m_dns;
    std::list<Group> m_groups;
};

int main()
{
    TheApp app;
    app.Run();
    return 0;
}
