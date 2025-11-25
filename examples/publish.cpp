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

        auto group = m_dns.MakeGroup(
            std::bind(&TheApp::OnGroup, this, _1, _2, _3)
        );

        m_group = std::make_shared<Group>(std::move(group));

        ec = m_group->AddService({
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

        ec = m_group->Commit();
        Report("Group::Commit", ec);

        m_io.run();
    }

private:
    void OnClient(ClientState state)
    {
        std::cout << "[Client::callback] " << state << std::endl;

        switch(state)
        {
        case ClientState::running:
            if (m_group)
                m_group->Recommit();
            break;
        case ClientState::failure:
            m_dns.Cancel();
            break;
        case ClientState::collision:
        case ClientState::connecting:
            if (m_group)
                m_group->Reset();
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

        // Exiting the test now, but if you want to keep this on the network,
        //     keep this program running.
        if (state == GroupState::established)
            m_io.stop();
    }

    boost::asio::io_context m_io;
    Mdns m_dns;
    std::shared_ptr<Group> m_group;
};

int main()
{
    TheApp app;
    app.Run();
    return 0;
}
