project(TurboVulcan_Engine VERSION 0.1.0)

# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

add_library(${PROJECT_NAME} ${PUBLIC_FILES} ${PRIVATE_FILES})

# Set target language
set_target_properties(TurboVulcan_Engine PROPERTIES LINKER_LANGUAGE CXX)

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)

target_include_directories(${PROJECT_NAME} PUBLIC Public)

target_link_libraries(${PROJECT_NAME} INTERFACE stdc++)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} PUBLIC -lm)

target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(${PROJECT_NAME} PRIVATE  VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)

add_dependencies(${PROJECT_NAME} TurboVulcan_Shaders)

add_compile_definitions(TURBO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(TURBO_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(TURBO_VERSION_PATCH=${PROJECT_VERSION_PATCH})

# Vulkan HPP definitions
add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)
add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

# Tracy profiler support
# Enable profiler when the `WITH_PROFILER` command line option is provided
SET(WITH_PROFILER false CACHE BOOL "Enable profiling support.")

if (${WITH_PROFILER})
    message("[TURBO] Enabling profiling support")

    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)

    option(TRACY_ENABLE "" ON)
    option(TRACY_ON_DEMAND "" ON)
    add_compile_definitions(WITH_PROFILER=1)

else ()
    message("[TURBO] Disabling profiling support")

    add_compile_definitions(WITH_PROFILER=0)
endif ()





