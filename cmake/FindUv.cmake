# - Find libuv
#
# Output varibles
#   Uv_FOUND          - True if libuv was found.
#   Uv_INCLUDE_DIR    - The include directory containing uv.h
#   Uv_LIBRARY        - The libuv library
#
# It also provides the imported target:
#
#   uv::uv
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

    set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
find_library(
    Uv_LIBRARY
    NAMES uv libuv
    HINTS ${PC_UV_LIBRARY_DIRS}
)

mark_as_advanced(Uv_INCLUDE_DIR Uv_LIBRARY)

# Handle REQUIRED, QUIET, VERSION and sets _FOUND
find_package_handle_standard_args(Uv
    REQUIRED_VARS Uv_INCLUDE_DIR Uv_LIBRARY
    VERSION_VAR PC_UV_VERSION
)

# Create imported target if not already defined
if (Uv_FOUND AND NOT TARGET uv::uv)
    add_library(uv::uv UNKNOWN IMPORTED)

    set_target_properties(uv::uv PROPERTIES
        IMPORTED_LOCATION "${Uv_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Uv_INCLUDE_DIR}"
    )
endif()
