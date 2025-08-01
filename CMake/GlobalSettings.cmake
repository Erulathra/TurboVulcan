set(CMAKE_CXX_STANDARD 23)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DWITH_TURBO_64)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DWITH_TURBO_32)
endif ()

#if (EXISTS "/usr/bin/mold" AND UNIX)
#    add_link_options("-fuse-ld=mold")
#endif ()

if (MSVC)
    add_definitions(-DWITH_MSVC)
endif ()

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(BuildType false CACHE STRING "Build type")

set(BuildType_Development "Development")
set(BuildType_Shipping "Shipping")

if (${BuildType} STREQUAL BuildType_Shipping)
    add_compile_definitions(TURBO_BUILD_SHIPPING)
    set(TURBO_BUILD_SHIPPING true)
else ()
    add_compile_definitions(TURBO_BUILD_DEVELOPMENT)
    set(TURBO_BUILD_DEVELOPMENT true)
endif ()
