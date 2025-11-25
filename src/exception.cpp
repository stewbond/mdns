#include <mdns/exception.hpp>
#include <cassert>

namespace mdns {

Exception::Exception(const Error& e) noexcept
    : err(e)
{}

Exception::Exception(const std::error_code& e) noexcept
    : err(static_cast<Error>(e.value()))
{
//    assert(&e.category() == &AvahiErrorCategory());
}

Exception::Exception(int e) noexcept
    : Exception(priv::AvahiErrorFromInt(e))
{}

const char* Exception::what() const noexcept
{
    return ErrorCategory::c_msg(static_cast<int>(err));
}

}
