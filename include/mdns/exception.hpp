#ifndef _dcbbcf20ac098188ab9d79bfceeac3cd
#define _dcbbcf20ac098188ab9d79bfceeac3cd
#include <exception>
#include <mdns/errorcode.hpp>

namespace mdns {

class Exception : public std::exception
{
public:
    Exception(const Error& e) noexcept;
    Exception(const std::error_code& e) noexcept;
    Exception(int e) noexcept; // When you're sure it's coming from avahi_*()
    const char* what() const noexcept override;
private:
    Error err;
};


} // namespace
#endif
