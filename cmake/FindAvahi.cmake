# - Find Avahi (avahi-client and avahi-common)
#
# Components:
#   common  - Avahi common library (avahi-common)
#   client  - Avahi client library (avahi-client)
#
# Input variables:
#   AVAHI_USE_STATIC (default OFF)
#     Causes avahi::common to point to a static library
#     Causes avahi::client to point to a static library
#
# Imported targets:
#   avahi::common <-- Aliased based on AVAHI_USE_STATIC
#   avahi::common-shared
#   avahi::common-static
#   avahi::client <-- Aliased based on AVAHI_USE_STATIC
#   avahi::client-shared
#   avahi::client-static
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

if(PkgConfig_FOUND)
    pkg_check_modules(PC_AVAHI_COMMON QUIET avahi-common)
    pkg_check_modules(PC_AVAHI_CLIENT QUIET avahi-client)
endif()

option(AVAHI_USE_SHARED "use static libraries" OFF)

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

    find_library(Avahi_common_STATIC_LIBRARY
        NAMES avahi-common.a libavahi-common.a
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
        Avahi_common_STATIC_LIBRARY
    )

    if(Avahi_common_FOUND AND NOT TARGET avahi::common)
        add_library(avahi::common-shared SHARED IMPORTED)
        add_library(avahi::common-static STATIC IMPORTED)

        if (AVAHI_USE_SHARED)
            add_library(avahi::common ALIAS avahi::common-shared)
        else()
            add_library(avahi::common ALIAS avahi::common-static)
        endif()
        set_target_properties(avahi::common-shared PROPERTIES
            IMPORTED_LOCATION             "${Avahi_common_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_common_INCLUDE_DIR}"
        )
        set_target_properties(avahi::common-static PROPERTIES
            IMPORTED_LOCATION             "${Avahi_common_STATIC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_common_INCLUDE_DIR}"
        )
        if (PC_AVAHI_COMMON_CFLAGS_OTHER)
            set_target_properties(avahi::common-shared avahi::common-static PROPERTIES
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

    find_library(Avahi_client_STATIC_LIBRARY
        NAMES avahi-client.a libavahi-client.a
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
        Avahi_client_STATIC_LIBRARY
    )

    if(Avahi_client_FOUND AND NOT TARGET avahi::client)
        add_library(avahi::client-shared SHARED IMPORTED)
        add_library(avahi::client-static STATIC IMPORTED)
        if (AVAHI_USE_SHARED)
            add_library(avahi::client ALIAS avahi::client-shared)
        else()
            add_library(avahi::client ALIAS avahi::client-static)
        endif()
        set_target_properties(avahi::client-shared PROPERTIES
            IMPORTED_LOCATION             "${Avahi_client_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_client_INCLUDE_DIR}"
        )
        set_target_properties(avahi::client-static PROPERTIES
            IMPORTED_LOCATION             "${Avahi_client_STATIC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Avahi_client_INCLUDE_DIR}"
        )
        if (PC_AVAHI_CLIENT_CFLAGS_OTHER)
            set_target_properties(avahi::client-shared avahi::common-static PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${PC_AVAHI_CLIENT_CFLAGS_OTHER}"
            )
        endif()

        if(TARGET avahi::common)
            target_link_libraries(avahi::client-shared INTERFACE avahi::common-shared)
            target_link_libraries(avahi::client-static INTERFACE avahi::common-static)
        endif()
    endif()
endif()

find_package_handle_standard_args(Avahi
    VERSION_VAR PC_AVAHI_CLIENT_VERSION
    HANDLE_COMPONENTS
)
