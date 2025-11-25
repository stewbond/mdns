#include <mdns/errorcode.hpp>
#include <avahi-common/error.h>
#include <cassert>

namespace mdns {

const char* ErrorCategory::name() const noexcept
{
    return "avahi";
}

std::string ErrorCategory::message(int ev) const
{
    std::string msg = c_msg(ev);

    if (msg == "Unhandled error code")
        msg += ": " + std::to_string(ev);

    return msg;
}

const char* ErrorCategory::c_msg(int ev) noexcept
{
    if (AVAHI_ERR_MAX < ev && ev <= AVAHI_OK)
        return avahi_strerror(ev);

    if (ev <= AVAHI_ERR_MAX)
        return "Unhandled error code";

    if (ev >= static_cast<int>(Error::max))
        return "Unhandled error code";

    switch(static_cast<Error>(ev))
    {
    case Error::could_not_construct: return "Could not construct object";
    case Error::aborted            : return "Operation aborted";
    case Error::bad_protocol       : return "An integer was supplied as a protocol outside of the range expected";
    case Error::bad_client_state   : return "An integer was supplied as a client state outside of the range expected";
    case Error::bad_browser_event  : return "An integer was supplied as a browser event outside of the range expected";
    case Error::bad_resolver_event : return "An integer was supplied as a resolver event outside of the range expected";
    case Error::bad_browser_type   : return "An integer was supplied as a domain brower type outside of the range expected";
    case Error::bad_group_type     : return "An integer was supplied as a group entry type outside of the range expected";
    case Error::not_connected      : return "Operation attempted before client is connected";
    case Error::bad_group_name     : return "A service was added to a group with a conflicting name"; // All services in a group must have identical names
    default                        : return "Unhandled error code";
    }
}


const std::error_category& ErrorCategory() noexcept
{
    static class ErrorCategory c;
    return c;
}

std::error_code make_error_code(Error code) noexcept
{
    return std::error_code(static_cast<int>(code), ErrorCategory());
}

// Guarantee these values are equal (protects against avahi updates)
// This is necessary because we hard-coded the values in the header
// to avoid an external header dependency.
static_assert(static_cast<int>(Error::ok                  ) == AVAHI_OK                         );
static_assert(static_cast<int>(Error::failure             ) == AVAHI_ERR_FAILURE                );
static_assert(static_cast<int>(Error::bad_state           ) == AVAHI_ERR_BAD_STATE              );
static_assert(static_cast<int>(Error::invalid_host_name   ) == AVAHI_ERR_INVALID_HOST_NAME      );
static_assert(static_cast<int>(Error::invalid_domain_name ) == AVAHI_ERR_INVALID_DOMAIN_NAME    );
static_assert(static_cast<int>(Error::no_network          ) == AVAHI_ERR_NO_NETWORK             );
static_assert(static_cast<int>(Error::invalid_ttl         ) == AVAHI_ERR_INVALID_TTL            );
static_assert(static_cast<int>(Error::is_pattern          ) == AVAHI_ERR_IS_PATTERN             );
static_assert(static_cast<int>(Error::collision           ) == AVAHI_ERR_COLLISION              );
static_assert(static_cast<int>(Error::invalid_record      ) == AVAHI_ERR_INVALID_RECORD         );
static_assert(static_cast<int>(Error::invalid_service_name) == AVAHI_ERR_INVALID_SERVICE_NAME   );
static_assert(static_cast<int>(Error::invalid_service_type) == AVAHI_ERR_INVALID_SERVICE_TYPE   );
static_assert(static_cast<int>(Error::invalid_port        ) == AVAHI_ERR_INVALID_PORT           );
static_assert(static_cast<int>(Error::invalid_key         ) == AVAHI_ERR_INVALID_KEY            );
static_assert(static_cast<int>(Error::invalid_address     ) == AVAHI_ERR_INVALID_ADDRESS        );
static_assert(static_cast<int>(Error::timeout             ) == AVAHI_ERR_TIMEOUT                );
static_assert(static_cast<int>(Error::too_many_clients    ) == AVAHI_ERR_TOO_MANY_CLIENTS       );
static_assert(static_cast<int>(Error::too_many_objects    ) == AVAHI_ERR_TOO_MANY_OBJECTS       );
static_assert(static_cast<int>(Error::too_many_entries    ) == AVAHI_ERR_TOO_MANY_ENTRIES       );
static_assert(static_cast<int>(Error::os                  ) == AVAHI_ERR_OS                     );
static_assert(static_cast<int>(Error::access_denied       ) == AVAHI_ERR_ACCESS_DENIED          );
static_assert(static_cast<int>(Error::invalid_operation   ) == AVAHI_ERR_INVALID_OPERATION      );
static_assert(static_cast<int>(Error::dbus_error          ) == AVAHI_ERR_DBUS_ERROR             );
static_assert(static_cast<int>(Error::disconnected        ) == AVAHI_ERR_DISCONNECTED           );
static_assert(static_cast<int>(Error::no_memory           ) == AVAHI_ERR_NO_MEMORY              );
static_assert(static_cast<int>(Error::invalid_object      ) == AVAHI_ERR_INVALID_OBJECT         );
static_assert(static_cast<int>(Error::no_daemon           ) == AVAHI_ERR_NO_DAEMON              );
static_assert(static_cast<int>(Error::invalid_interface   ) == AVAHI_ERR_INVALID_INTERFACE      );
static_assert(static_cast<int>(Error::invalid_protocol    ) == AVAHI_ERR_INVALID_PROTOCOL       );
static_assert(static_cast<int>(Error::invalid_flags       ) == AVAHI_ERR_INVALID_FLAGS          );
static_assert(static_cast<int>(Error::not_found           ) == AVAHI_ERR_NOT_FOUND              );
static_assert(static_cast<int>(Error::invalid_config      ) == AVAHI_ERR_INVALID_CONFIG         );
static_assert(static_cast<int>(Error::version_mismatch    ) == AVAHI_ERR_VERSION_MISMATCH       );
static_assert(static_cast<int>(Error::invalid_service_subt) == AVAHI_ERR_INVALID_SERVICE_SUBTYPE);
static_assert(static_cast<int>(Error::invalid_packet      ) == AVAHI_ERR_INVALID_PACKET         );
static_assert(static_cast<int>(Error::invalid_dns_error   ) == AVAHI_ERR_INVALID_DNS_ERROR      );
static_assert(static_cast<int>(Error::dns_formerr         ) == AVAHI_ERR_DNS_FORMERR            );
static_assert(static_cast<int>(Error::dns_servfail        ) == AVAHI_ERR_DNS_SERVFAIL           );
static_assert(static_cast<int>(Error::dns_nxdomain        ) == AVAHI_ERR_DNS_NXDOMAIN           );
static_assert(static_cast<int>(Error::dns_notimp          ) == AVAHI_ERR_DNS_NOTIMP             );
static_assert(static_cast<int>(Error::dns_refused         ) == AVAHI_ERR_DNS_REFUSED            );
static_assert(static_cast<int>(Error::dns_yxdomain        ) == AVAHI_ERR_DNS_YXDOMAIN           );
static_assert(static_cast<int>(Error::dns_yxrrset         ) == AVAHI_ERR_DNS_YXRRSET            );
static_assert(static_cast<int>(Error::dns_nxrrset         ) == AVAHI_ERR_DNS_NXRRSET            );
static_assert(static_cast<int>(Error::dns_notauth         ) == AVAHI_ERR_DNS_NOTAUTH            );
static_assert(static_cast<int>(Error::dns_notzone         ) == AVAHI_ERR_DNS_NOTZONE            );
static_assert(static_cast<int>(Error::invalid_rdata       ) == AVAHI_ERR_INVALID_RDATA          );
static_assert(static_cast<int>(Error::invalid_dns_class   ) == AVAHI_ERR_INVALID_DNS_CLASS      );
static_assert(static_cast<int>(Error::invalid_dns_type    ) == AVAHI_ERR_INVALID_DNS_TYPE       );
static_assert(static_cast<int>(Error::not_supported       ) == AVAHI_ERR_NOT_SUPPORTED          );
static_assert(static_cast<int>(Error::not_permitted       ) == AVAHI_ERR_NOT_PERMITTED          );
static_assert(static_cast<int>(Error::invalid_argument    ) == AVAHI_ERR_INVALID_ARGUMENT       );
static_assert(static_cast<int>(Error::is_empty            ) == AVAHI_ERR_IS_EMPTY               );
static_assert(static_cast<int>(Error::no_change           ) == AVAHI_ERR_NO_CHANGE              );


namespace priv {

    // Convert an avahi return-code to an AvahiError or std::error_code
    Error AvahiErrorFromInt(int ec) noexcept
    {
        assert(AVAHI_ERR_MAX < ec && ec <= AVAHI_OK);
        return static_cast<Error>(ec);
    }

    std::error_code ErrFromAvahiInt(int ec) noexcept
    {
        return make_error_code( AvahiErrorFromInt(ec) );
    }
}

}
