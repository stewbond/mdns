#ifndef _e50b7f7c61045854edac227d6e9ea15a
#define _e50b7f7c61045854edac227d6e9ea15a
#include <cstdint>
#include <array>
#include <variant>
#include <ostream>

namespace mdns {

struct IPv4Address
{
    std::uint32_t value;
    constexpr auto operator<=>(const IPv4Address&) const noexcept = default;
    friend std::ostream& operator<<(std::ostream&, const IPv4Address&);
};

struct IPv6Address
{
    std::array<uint8_t, 16> value;
    constexpr auto operator<=>(const IPv6Address&) const noexcept = default;
    friend std::ostream& operator<<(std::ostream&, const IPv6Address&);
};

struct InvalidAddress
{
    constexpr auto operator<=>(const InvalidAddress&) const noexcept = default;
};

class Address
{
public:
    constexpr Address(IPv4Address v4) noexcept : m_data(v4) {}
    constexpr Address(IPv6Address v6) noexcept : m_data(v6) {}
    constexpr Address() noexcept : m_data(InvalidAddress{}) {}

    constexpr bool IsV4() const noexcept { return std::holds_alternative<IPv4Address>(m_data); }
    constexpr bool IsV6() const noexcept { return std::holds_alternative<IPv6Address>(m_data); }

    constexpr const IPv4Address& GetV4() const { return std::get<IPv4Address>(m_data); }
    constexpr const IPv6Address& GetV6() const { return std::get<IPv6Address>(m_data); }

    constexpr auto operator<=>(const Address&) const noexcept = default;
private:
    std::variant<InvalidAddress, IPv4Address, IPv6Address> m_data;

    friend std::ostream& operator<<(std::ostream&, const Address&);
};

}
#endif
