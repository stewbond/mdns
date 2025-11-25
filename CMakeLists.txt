cmake_minimum_required(VERSION 3.18)

project(mdns VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

cmake_policy(SET CMP0167 NEW) # Addresses warnings about FindBoost in cmake < 3.30
find_package(Boost COMPONENTS system QUIET)    # target: Boost::system
find_package(Uv QUIET)                         # target: uv::uv
find_package(Avahi COMPONENTS client REQUIRED) # target: avahi::client

add_library(mdns
  # Public objects class
  src/mdns.cpp
  src/group.cpp

  # main run-context
  src/client.cpp

  # High level wrappers
  src/servicetypebrowser.cpp # Get type from given Iface, Proto, Domain
  src/servicebrowser.cpp     # Get name from given Iface, Proto, Domain, Type
  src/serviceresolver.cpp    # Get details from given name, iface, proto, domain, type

  # Low level, probably not useful, wrappers
  src/addressresolver.cpp # Get hostname from address
  src/hostnameresolver.cpp # Get address from hostname
  src/domainbrowser.cpp # Finds sub-domains in a domain (default=local)
  src/recordbrowser.cpp # Get A, AAAA, PTR, TXT, MX, CNAME, etc records from DNS

  # cpp support
  src/mdnsimpl.cpp
  src/groupimpl.cpp
  src/errorcode.cpp
  src/exception.cpp
  src/pollsimple.cpp
  src/pollthreaded.cpp
  $<$<TARGET_EXISTS:Boost::system>:src/pollasio.cpp>
  $<$<TARGET_EXISTS:uv::uv>:src/polluv.cpp>
  src/structures.cpp
)

target_compile_definitions(mdns
  PUBLIC
    $<$<TARGET_EXISTS:Boost::system>:MDNS_BOOST>
    $<$<TARGET_EXISTS:uv::uv>:MDNS_UV>
)

target_link_libraries(mdns
  PRIVATE
    avahi::client
  PUBLIC
    $<TARGET_NAME_IF_EXISTS:Boost::system>
    $<TARGET_NAME_IF_EXISTS:uv::uv>
)

target_include_directories(mdns
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

list(APPEND mdns_PUBLIC_HEADER
  include/mdns/address.hpp
  include/mdns/errorcode.hpp
  include/mdns/exception.hpp
  include/mdns/group.hpp
  include/mdns/mdns.hpp
  include/mdns/structures.hpp
)

set_target_properties(mdns PROPERTIES
  PUBLIC_HEADER "${mdns_PUBLIC_HEADER}"
  VERSION ${mdns_VERSION} # Not redundant, creates symlink lib*.so.1.0.0
  SOVERSION 1
)

list(APPEND examples
  browse-domains
  browse-services
  browse-all
  browse-uvloop
  publish
  publish-conflict
)

option(MDNS_EXAMPLES "Build examples" OFF)
if (MDNS_EXAMPLES)
  foreach(example ${examples})
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} PRIVATE mdns)
  endforeach()
endif()

include(GNUInstallDirs)
install(TARGETS mdns
  EXPORT mdnsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mdns/
)

install(EXPORT mdnsTargets
  FILE mdnsConfig.cmake
  NAMESPACE mdns::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mdns
)

foreach(example ${examples})
  install(FILES examples/${example}.cpp
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
  )
endforeach()
