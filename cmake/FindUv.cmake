# - Find libuv
#
# Options
#   UV_USE_STATIC     - If set to ON uv::uv will point to the static library,
#                       otherwise, it will point to a shared library
#
# Output varibles
#   Uv_FOUND          - True if libuv was found.
#   Uv_INCLUDE_DIR    - The include directory containing uv.h
#   Uv_SHARED_LIBRARY - The libuv library (shared)
#   Uv_STATIC_LIBRARY - The libuv library (static)
#   Uv_LIBRARY        - The libuv library (shared or static based on user-defined UV_USE_STATIC)
#
# It also provides the imported target:
#
#   uv::shared
#   uv::static
#   uv::uv     <-- Points to shared or static based on ${UV_USE_STATIC}
#
# Usage:
#   find_package(uv REQUIRED)
#

include(FindPackageHandleStandardArgs)
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(PC_UV QUIET libuv)
    pkg_check_modules(PC_UV_STATIC QUIET libuv-static)
endif()

find_path(
    Uv_INCLUDE_DIR
    NAMES uv.h
    PATH_SUFFIXES uv
    HINTS ${PC_UV_INCLUDE_DIRS}
)

find_library(
    Uv_SHARED_LIBRARY
    NAMES uv libuv
    HINTS ${PC_UV_LIBRARY_DIRS}
)

find_library(
    Uv_STATIC_LIBRARY
    NAMES uv_a libuv_a libuv.a uv.a
    HINDS ${PC_UV_STATIC_LIBRARY_DIRS}
)

mark_as_advanced(Uv_INCLUDE_DIR Uv_SHARED_LIBRARY Uv_STATIC_LIBRARY)

# Handle REQUIRED, QUIET, VERSION and sets _FOUND
find_package_handle_standard_args(
    Uv
    REQUIRED_VARS
        Uv_INCLUDE_DIR
        Uv_SHARED_LIBRARY
        Uv_STATIC_LIBRARY
    VERSION_VAR PC_UV_VERSION
)

# Create imported target if not already defined
if (Uv_FOUND AND NOT TARGET uv::shared)
    add_library(uv::shared UNKNOWN IMPORTED)

    set_target_properties(uv::shared PROPERTIES
        IMPORTED_LOCATION "${Uv_SHARED_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Uv_INCLUDE_DIR}"
    )
endif()

if (Uv_FOUND AND NOT TARGET uv::static)
    add_library(uv::static UNKNOWN IMPORTED)

    set_target_properties(uv::static PROPERTIES
        IMPORTED_LOCATION "${Uv_STATIC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Uv_INCLUDE_DIR}"
    )
endif()

option(UV_USE_STATIC "Use static libuv" OFF)
if (UV_USE_STATIC AND TARGET uv::static)
    add_library(uv::uv ALIAS uv::static)
    set (Uv_LIBRARY ${Uv_STATIC_LIBRARY})
elseif(TARGET uv::shared)
    add_library(uv::uv ALIAS uv::shared)
    set (Uv_LIBRARY ${Uv_SHARED_LIBRARY})
endif()
