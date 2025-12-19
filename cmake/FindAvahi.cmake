# Find Avahi (avahi-client and avahi-common)
#
# Components:
#   common  - Avahi common library (avahi-common)
#   client  - Avahi client library (avahi-client)
#
# Imported targets:
#   avahi::common
#   avahi::client
#
# Result variables:
#   Avahi_common_FOUND
#   Avahi_common_INCLUDE_DIR
#   Avahi_common_LIBRARY
#   Avahi_client_FOUND
#   Avahi_client_INCLUDE_DIR
#   Avahi_client_LIBRARY
#
# Compatibility variables:
#   Avahi_common_INCLUDE_DIRS
#   Avahi_common_LIBRARIES
#   Avahi_client_INCLUDE_DIRS
#   Avahi_client_LIBRARIES

include(FindPackageHandleStandardArgs)
find_package(PkgConfig QUIET)

option(AVAHI_USE_STATIC_LIBS "Use static Avahi libraries" OFF)

if (AVAHI_USE_STATIC_LIBS)
    set(_avahi_orig_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

if(PkgConfig_FOUND)
    pkg_check_modules(PC_AVAHI_COMMON QUIET avahi-common)
    pkg_check_modules(PC_AVAHI_CLIENT QUIET avahi-client)
endif()

# 'client' depends on 'common', so specifying 'client' should implicitly bring
#     in 'common'
if("common" IN_LIST Avahi_FIND_COMPONENTS OR
   "client" IN_LIST Avahi_FIND_COMPONENTS)

    find_path(Avahi_common_INCLUDE_DIR
        NAMES avahi-common/defs.h
        HINTS ${PC_AVAHI_COMMON_INCLUDE_DIRS}
    )

    find_library(Avahi_common_LIBRARY
        NAMES avahi-common
        HINTS ${PC_AVAHI_COMMON_LIBRARY_DIRS}
    )

    set(Avahi_common_FOUND FALSE)
    if (Avahi_common_INCLUDE_DIR AND Avahi_common_LIBRARY)
        set(Avahi_common_FOUND TRUE)
    endif()

    set(Avahi_common_INCLUDE_DIRS "${Avahi_common_INCLUDE_DIR}")
    set(Avahi_common_LIBRARIES    "${Avahi_common_LIBRARY}")

    mark_as_advanced(
        Avahi_common_INCLUDE_DIR
        Avahi_common_LIBRARY
    )

    if(Avahi_common_FOUND AND NOT TARGET avahi::common)
        add_library(avahi::common UNKNOWN IMPORTED)

        set_target_properties(avahi::common PROPERTIES
            IMPORTED_LOCATION             "${Avahi_common_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_common_INCLUDE_DIR}"
        )
        if (PC_AVAHI_COMMON_CFLAGS_OTHER)
            set_target_properties(avahi::common PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${PC_AVAHI_COMMON_CFLAGS_OTHER}"
            )
        endif()
    endif()
endif()

if("client" IN_LIST Avahi_FIND_COMPONENTS)
    find_path(Avahi_client_INCLUDE_DIR
        NAMES avahi-client/client.h
        HINTS ${PC_AVAHI_CLIENT_INCLUDE_DIRS}
    )

    find_library(Avahi_client_LIBRARY
        NAMES avahi-client
        HINTS ${PC_AVAHI_CLIENT_LIBRARY_DIRS}
    )

    set(Avahi_client_FOUND FALSE)
    if (Avahi_client_INCLUDE_DIR AND Avahi_client_LIBRARY AND Avahi_common_FOUND)
        set(Avahi_client_FOUND TRUE)
    endif()

    set(Avahi_client_INCLUDE_DIRS "${Avahi_client_INCLUDE_DIR}")
    set(Avahi_client_LIBRARIES    "${Avahi_client_LIBRARY}")

    mark_as_advanced(
        Avahi_client_INCLUDE_DIR
        Avahi_client_LIBRARY
    )

    if(Avahi_client_FOUND AND NOT TARGET avahi::client)
        add_library(avahi::client UNKNOWN IMPORTED)
        set_target_properties(avahi::client PROPERTIES
            IMPORTED_LOCATION             "${Avahi_client_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_client_INCLUDE_DIR}"
        )
        if (PC_AVAHI_CLIENT_CFLAGS_OTHER)
            set_target_properties(avahi::client avahi::common PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${PC_AVAHI_CLIENT_CFLAGS_OTHER}"
            )
        endif()

        target_link_libraries(avahi::client INTERFACE
          $<TARGET_NAME_IF_EXISTS:avahi::common>
          $<$<BOOL:AVAHI_USE_STATIC_LIBS>:dbus-1>
        )
    endif()
endif()

find_package_handle_standard_args(Avahi
    VERSION_VAR PC_AVAHI_CLIENT_VERSION
    HANDLE_COMPONENTS
)

if (AVAHI_USE_STATIC_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_avahi_orig_suffixes})
endif()

