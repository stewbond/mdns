# libmdns

This library wraps avahi-client in C++ and can be integrated with a number of
event loops (including boost::asio or libuv).

Browse, resolve, or publish any mDNS or mDNS-SD records.

## mDNS & mDNS-SD

mDNS is multicast DNS.  It allows machines on the same network to send
information about DNS records.  This allows machines to publish information
they understand such as: 

- A or AAAA records with my IP/hostname (Where is the EIS?)
- PTR records that allow reverse lookups (Who is at this IP?)

Tools like avahi-daemon will listen for these mDNS messages and maintain a local
cache.  
When resolving a hostname, a user-space application will typically call 
gethostbyname() or getaddrinfo().  
These tools consule nsswitch for how to resolve hostnames.  This can be
configured in /etc/nsswitch.conf: 

```
hosts: files mdns4 dns
```
Order can be seen here.  

- `files`: causes gethostbyname() to consult /etc/hosts
- `mdns4`: causes gethostbyname() to consule any mDNS cache, such as
  avahi-daemon for IPv4 (A) records.
- A request is sent to the configure DNS service, only if the two prior items
  are not resolved.

mDNS-SD adds "Service Discovery" over mDNS.  
This adds more records than just A or AAAA records, and can be used to announce
services. 

For example, if you want to find printers that can communicate via IPP (over
TCP), you could browse for "type" `_ipp._tcp`.  
The resulting services will have human readable names, hostnames, and ports.
Those services can be resolved further to include IP addresses, and TXT records.

Another example is to browse for Sim Server instances on the local network.
Browse for "type" `_simserver._http._tcp`.   The TXT records could contain a
list of HTTP routes available on that service.  Such as: 

- "QTG=/qtg"
- "IOM=/iom"
- "ThalesFMS=/fms3000"

Then you could browse for sim-server instances, and filter for any that have a
ThalesFMS TXT record, and then talk to it on that route.

## Intention

I would love to be able to stop configuring IP addresses in the load, scattered
throughout various configuration files configuring them in a central location. 

For example, our current DUs on the display machine are launched by the host
like this:

```
ExecStart=/usr/bin/remotetrigger -u ws://10.10.1.17:13668/process -f services/du1.json
```

I would love to change that from `10.10.1.17` to `display`. 

```
ExecStart=/usr/bin/remotetrigger -u ws://display:13668/process -f services/du1.json
```
This would let us move the DUs to any machine, without the need to change
several configuration files.   This is especially useful when we start expanding
our product line.  For example, on an IPT we may choose to run the panels and 
GPWS rehost on the same machine.  We can do that by giving resolvable hostnames 
so a GPWS client can find the GPWS service, and the panels client can find the
panels service.

We can already do this to an extent by adding `10.10.1.17 display` in 
`/etc/hosts`, but we have no way of bulk-deploying `/etc/hosts` without 
overriding a default.  That's what DNS solves.

By adding mDNS, we can have each computer announce what services they are 
running.  The EIS package could add EIS records to the mDNS, so a host can
simply browse, find, and start communicating with any EIS on the current
network.

This means we don't need any configuration at all.

In reality, some tools won't work out of the box.  Windows doesn't handle mDNS
well, and so we may need to delegate some of that responsibility to the host
which can publish records about other machines too.

The mDNS-SD takes us one step further, and can resolve the ports.  Imagine the
EFCS announcing its in/out UDP ports.  A host could simply lookup the type of 
"_efcs._udp", and learn that it needs to send to 10.10.1.1 on port 5595 and
listen on port 5596.  Those numbers are set by the EFCS package itself, so there
is no host configuration needed.  That means you just need to install the EFCS
*somewhere* on the network, and it'll just work.

While hostnames work out of the box, full type/port resolutions need a custom
resolver.  That means you can't simply point your webbrowser to
`http://entrance-screen/entrance` and hope it resolves to
`http://10.10.1.1:13668/entrance`.  You'll need a custom tool to resolve that.

## Capabilities

This library allows you browse, resolve, or publish any mDNS-SD records.

- Browse domains: 
  - Given a domain (e.g. ".local"), return all subdomains (e.g. ".sim.local"). 
  - I recommend sticking with the default "local".  Things get complicated
    otherwise.
- Browse service types
  - Given an interface ( 1: loopback, 2: enp0s31f6, ... ), protocol (IPv4, IPv6) 
    and domain, list all service types ("_http._tcp", or "_collins._ig._udp").
- Browse services
  - Given a service type (and interface, protocol, domain), list the unique name
    of all services that implement it.
- Resolve a service
  - Given a service name (plus type, interface, protocol, domain), get the
    hostname, port, address, and any TXT records.
    that implement it.
- Resolve a hostname
  - Given an IP address, resolve the hostname (revert DNS lookup)
- Resolve an address
  - Given a hostname, resolve the IP address
- Publish a hostname
  - This can be your own hostname, or publish on someone else's behalf.
- Publish a service
  - This will contain the service name, host name, port, type, domain, protocol,
    interface, and any TXT records or subtype aliases.  Domain, protocol, and
    interface can all be left default.

Avahi Daemon already provides a good way to publish services.  If we want to
deploy anything, consider adding a drop-in to `/etc/avahi/services/*.service`. 
That's probably less buggy and more powerful than anything provided by this
library.  This library will be needed for publishing once we start to
dynamically create services (through sim-server configurations for example).
See `man avahi.service.3` for details.

Example:
```
$ cat -p services/ig-to.service 
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
  <name>ig-to</name>
  <service>
    <type>_ep._ig._udp</type>
    <domain-name>local</domain-name>
    <host-name>ig.local</host-name>
    <port>5001</port>
    <txt-record>UDP Opcodes to the IG from the host</txt-record>
  </service>
</service-group>
```

With this we can search `_ep._ig._udp` and focus on `ig-to`.  The
collins subtype tells us to use the EP protocol (as opposed to an RSI or CIGI 
protocol).  Then we send UDP packets to ig.local which will be an address
defined elsewhere.


This library will first focus on browsing/resolving, then it will iron out any
kinks in publishing.

## Usage

See examples for full C++ examples.  Here's a taste. 

```
boost::asio::io_context io;

Mdns dns;
dns.Connect( ClientFlags::no_fail, [](auto){ /* callback */ } );

io.run();
```
In the callback, you get the state of the DNS client.  
When it reaches "running", you know we've connected to the avahi-daemon, and we 
can do look up all web-servers (any interface, any protocol): 

```
dns.BrowseServices(
  { .type="_http._tcp" }, 
  LookupFlags::none,
  [&dns)(auto event, auto result, auto flag, auto err) {
      std::cout << event
          << " iface:"  << result.interface
          << " proto:"  << result.protocol
          << " name:'"  << result.name
          << "' type:"  << result.type
          << " domain:" << result.domain << std::endl;
  }
);
```
The service browser will first exhaust the local avahi-daemon cache before
listening on the network, so results usually come in pretty quickly.  You'll get
a `cache_exhaused` event to indicate that things will slow down a little.

We can pass the result from the service browser, into a service resolver to
finish the resolution:

```
dns.BrowseServices(
  { .type="_http._tcp" }, 
  LookupFlags::none,
  [&dns)(auto event, auto result, auto flag, auto err) {
    if (err) return;
    if (event == BrowserEvent::_new)
    {
      dns.ResolveService(
        std::move(result), 
        Protocol::ipv4,  // Ignore AAAA records. 
        LookupFlags::none, 
        [](auto event, auto result, auto flags, auto err) {
          std::cout 
            << " iface:"    << result.interface.value()
            << " proto:"    << result.protocol
            << " name:'"    << result.name
            << "' type:"    << result.type
            << " domain:"   << result.domain
            << " hostname:" << result.hostname
            << " address:"  << result.addr << ":" << result.port << std::endl;
        }
      );
    }
  }
);
```
