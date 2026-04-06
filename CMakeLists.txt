cmake_minimum_required(VERSION 3.10)

# Default to size-optimized build (-Os), matching Arduino IDE behaviour.
# Override with: cmake -B build -DCMAKE_BUILD_TYPE=Debug
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type" FORCE)
endif()

project(ostx VERSION 1.0.0 LANGUAGES C)

# C89 -- required for 8-bit MCU compatibility.
# avr-gcc, arm-none-eabi-gcc, sdcc, and xc8 all support C89/ANSI mode.
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Optional tuning  (override on the command line: cmake -DOSTX_ID_MAX=12 .)
# ---------------------------------------------------------------------------
# These translate to -D flags that override the #ifndef defaults in
# ostx_config.h without touching the installed header.
set(OSTX_ID_MAX   "" CACHE STRING "Override OSTX_ID_MAX   (max sensor ID length + NUL)")
set(OSTX_UNIT_MAX "" CACHE STRING "Override OSTX_UNIT_MAX (max unit string length + NUL)")
set(OSTX_BODY_MAX "" CACHE STRING "Override OSTX_BODY_MAX (max body byte count)")

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------
add_library(ostx STATIC
    src/ostx_crc.c
    src/ostx_b62.c
    src/ostx_packet.c
    src/ostx_sensor.c
    src/ostx_static.c
    src/ostx_stream.c
)
target_include_directories(ostx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ostx>
)

if(OSTX_ID_MAX)
    target_compile_definitions(ostx PUBLIC OSTX_ID_MAX=${OSTX_ID_MAX})
endif()
if(OSTX_UNIT_MAX)
    target_compile_definitions(ostx PUBLIC OSTX_UNIT_MAX=${OSTX_UNIT_MAX})
endif()
if(OSTX_BODY_MAX)
    target_compile_definitions(ostx PUBLIC OSTX_BODY_MAX=${OSTX_BODY_MAX})
endif()

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
option(OSTX_BUILD_EXAMPLES "Build example programs" ON)
if(OSTX_BUILD_EXAMPLES)
    add_executable(basic_tx examples/basic_tx.c)
    target_link_libraries(basic_tx PRIVATE ostx)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
option(OSTX_BUILD_TESTS "Build test vector suite" ON)
if(OSTX_BUILD_TESTS)
    enable_testing()

    add_executable(test_vector tests/test_vector.c)
    target_link_libraries(test_vector PRIVATE ostx)

    add_test(NAME test_vector COMMAND test_vector)
endif()

# ---------------------------------------------------------------------------
# Install + CMake package distribution
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 1. Install the library binary and export the CMake target.
install(TARGETS ostx
    EXPORT  ostxTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 2. Install the public headers.
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ostx)

# 3. Generate and install the export file (ostxTargets.cmake).
install(EXPORT ostxTargets
    FILE      ostxTargets.cmake
    NAMESPACE ostx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ostx
)

# 4. Generate and install package config + version files.
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ostxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ostxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ostx
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ostxConfigVersion.cmake"
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ostxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ostxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ostx
)

