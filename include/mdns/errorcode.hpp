#ifndef _7db3641f0d8701712693a35edd1debad
#define _7db3641f0d8701712693a35edd1debad
#include <system_error>

namespace mdns {

enum class Error
{
    // avahi-defined error codes
    // Re-defined here to avoid an include-dependency
    // static_asserted in sources to verify
    ok                   = 0,   // OK
    failure              = -1,  // Generic error code
    bad_state            = -2,  // Object was in a bad state
    invalid_host_name    = -3,  // Invalid host name
    invalid_domain_name  = -4,  // Invalid domain name
    no_network           = -5,  // No suitable network protocol available
    invalid_ttl          = -6,  // Invalid DNS TTL
    is_pattern           = -7,  // RR key is pattern
    collision            = -8,  // Name collision
    invalid_record       = -9,  // Invalid RR
    invalid_service_name = -10, // Invalid service name
    invalid_service_type = -11, // Invalid service type
    invalid_port         = -12, // Invalid port number
    invalid_key          = -13, // Invalid key
    invalid_address      = -14, // Invalid address
    timeout              = -15, // Timeout reached
    too_many_clients     = -16, // Too many clients
    too_many_objects     = -17, // Too many objects
    too_many_entries     = -18, // Too many entries
    os                   = -19, // OS error
    access_denied        = -20, // Access denied
    invalid_operation    = -21, // Invalid operation
    dbus_error           = -22, // An unexpected D-Bus error occurred
    disconnected         = -23, // Daemon connection failed
    no_memory            = -24, // Memory exhausted
    invalid_object       = -25, // The object passed to this function was invalid
    no_daemon            = -26, // Daemon not running
    invalid_interface    = -27, // Invalid interface
    invalid_protocol     = -28, // Invalid protocol
    invalid_flags        = -29, // Invalid flags
    not_found            = -30, // Not found
    invalid_config       = -31, // Configuration error
    version_mismatch     = -32, // Verson mismatch
    invalid_service_subt = -33, // Invalid service subtype
    invalid_packet       = -34, // Invalid packet
    invalid_dns_error    = -35, // Invlaid DNS return code
    dns_formerr          = -36, // DNS Error: Form error
    dns_servfail         = -37, // DNS Error: Server Failure
    dns_nxdomain         = -38, // DNS Error: No such domain
    dns_notimp           = -39, // DNS Error: Not implemented
    dns_refused          = -40, // DNS Error: Operation refused
    dns_yxdomain         = -41,
    dns_yxrrset          = -42,
    dns_nxrrset          = -43,
    dns_notauth          = -44, // DNS Error: Not authorized
    dns_notzone          = -45,
    invalid_rdata        = -46, // Invalid RDATA
    invalid_dns_class    = -47, // Invalid DNS class
    invalid_dns_type     = -48, // Invalid DNS type
    not_supported        = -49, // Not supported
    not_permitted        = -50, // Operation not permitted
    invalid_argument     = -51, // Invalid argument
    is_empty             = -52, // Is empty
    no_change            = -53, // The requested operation is invalid because it is redundant

    // Extra error codes defined by this cxx implementation
    could_not_construct = 1,
    aborted,
    bad_protocol,
    bad_client_state,
    bad_browser_event,
    bad_resolver_event,
    bad_browser_type,
    bad_group_type,
    not_connected,
    bad_group_name,

    max // Not a real-error.  Must always be last
};


class ErrorCategory : public std::error_category
{
public:
    virtual const char* name() const noexcept override;
    virtual std::string message(int ev) const override;

    // Static, no allocations, appropriate for exceptions.
    static const char* c_msg(int ev) noexcept; // No allocations, safe for exceptions
};


// The category must be a singleton, so std::error_code doesn't confuse these
// codes with other codes.
const std::error_category& ErrorCategory() noexcept;


// Not an intentional part of the public-API.  This is used by std::error_code
// to implicitly cast the enum to an error code.
std::error_code make_error_code(Error code) noexcept;


namespace priv {
    // Convert an avahi return-code to an Error or std::error_code
    Error AvahiErrorFromInt(int ec) noexcept;
    std::error_code ErrFromAvahiInt(int ec) noexcept;
}

} // namespace

template<>
struct std::is_error_code_enum<mdns::Error> : public std::true_type {};

#endif
