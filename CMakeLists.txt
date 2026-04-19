#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira. All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
# ====================================================================
# Vix.cpp - Process Module
# ====================================================================
# Purpose:
#   Portable process management utilities for Vix.
#   Provides process spawning, waiting, termination, exit status,
#   environment overrides, working directory control, and stdio policies.
#
#   Builds as STATIC when sources exist, otherwise as a header-only
#   INTERFACE target.
#
# Public Targets:
#   - vix_process   : The actual library target (STATIC or INTERFACE)
#   - vix::process  : Namespaced alias for consumers
#
# Public Dependencies:
#   - vix::error
#   - vix::env
#   - vix::path
#
# Installation/Export:
#   - Umbrella build exports into: VixTargets
#   - Standalone build exports into: vix_processTargets
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(vix_process VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------
# Export set selection
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_PROCESS_EXPORT_SET VixTargets)
else()
  set(VIX_PROCESS_EXPORT_SET vix_processTargets)
endif()

# --------------------------------------------------------------------
# Umbrella build policy:
# When building inside the Vix umbrella, dependencies are managed by the
# umbrella (add_subdirectory order + options). Process must not FetchContent
# nor add sibling subdirectories in umbrella mode.
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_PROCESS_FETCH_ERROR OFF CACHE BOOL "Auto-fetch vix::error if missing" FORCE)
  set(VIX_PROCESS_FETCH_ENV   OFF CACHE BOOL "Auto-fetch vix::env if missing" FORCE)
  set(VIX_PROCESS_FETCH_PATH  OFF CACHE BOOL "Auto-fetch vix::path if missing" FORCE)
endif()

# --------------------------------------------------------------------
# Sources discovery
# --------------------------------------------------------------------
file(GLOB_RECURSE PROCESS_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------
option(VIX_PROCESS_FETCH_ERROR "Auto-fetch vix::error if missing" ON)
option(VIX_PROCESS_FETCH_ENV   "Auto-fetch vix::env if missing" ON)
option(VIX_PROCESS_FETCH_PATH  "Auto-fetch vix::path if missing" ON)
option(VIX_PROCESS_FETCH_TESTS "Auto-fetch vix::tests if missing" ON)

# error
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  if (NOT TARGET vix::error)
    message(FATAL_ERROR "[process] Umbrella build: vix::error must be provided by umbrella before process.")
  endif()
else()
  if (NOT TARGET vix::error)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../error/CMakeLists.txt")
      message(STATUS "[process] Adding error from sibling: ../error")
      add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../error" "error")
    else()
      find_package(vix_error CONFIG QUIET)
    endif()

    if (NOT TARGET vix::error)
      if (VIX_PROCESS_FETCH_ERROR)
        include(FetchContent)
        message(STATUS "[process] Fetching vix::error via FetchContent")
        FetchContent_Declare(vix_error
          GIT_REPOSITORY https://github.com/vixcpp/error.git
          GIT_TAG        v0.1.0
        )
        FetchContent_MakeAvailable(vix_error)
      else()
        message(FATAL_ERROR
          "vix::error not found. Enable VIX_PROCESS_FETCH_ERROR=ON, install vix_error, or provide the target before process."
        )
      endif()
    endif()
  endif()
endif()

# env
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  if (NOT TARGET vix::env)
    message(FATAL_ERROR "[process] Umbrella build: vix::env must be provided by umbrella before process.")
  endif()
else()
  if (NOT TARGET vix::env)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../env/CMakeLists.txt")
      message(STATUS "[process] Adding env from sibling: ../env")
      add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../env" "env")
    else()
      find_package(vix_env CONFIG QUIET)
    endif()

    if (NOT TARGET vix::env)
      if (VIX_PROCESS_FETCH_ENV)
        include(FetchContent)
        message(STATUS "[process] Fetching vix::env via FetchContent")
        FetchContent_Declare(vix_env
          GIT_REPOSITORY https://github.com/vixcpp/env.git
          GIT_TAG        v0.1.0
        )
        FetchContent_MakeAvailable(vix_env)
      else()
        message(FATAL_ERROR
          "vix::env not found. Enable VIX_PROCESS_FETCH_ENV=ON, install vix_env, or provide the target before process."
        )
      endif()
    endif()
  endif()
endif()

# path
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  if (NOT TARGET vix::path)
    message(FATAL_ERROR "[process] Umbrella build: vix::path must be provided by umbrella before process.")
  endif()
else()
  if (NOT TARGET vix::path)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../path/CMakeLists.txt")
      message(STATUS "[process] Adding path from sibling: ../path")
      add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../path" "path")
    else()
      find_package(vix_path CONFIG QUIET)
    endif()

    if (NOT TARGET vix::path)
      if (VIX_PROCESS_FETCH_PATH)
        include(FetchContent)
        message(STATUS "[process] Fetching vix::path via FetchContent")
        FetchContent_Declare(vix_path
          GIT_REPOSITORY https://github.com/vixcpp/path.git
          GIT_TAG        v0.1.0
        )
        FetchContent_MakeAvailable(vix_path)
      else()
        message(FATAL_ERROR
          "vix::path not found. Enable VIX_PROCESS_FETCH_PATH=ON, install vix_path, or provide the target before process."
        )
      endif()
    endif()
  endif()
endif()

set(VIX_ERROR_TARGET vix::error)
set(VIX_ENV_TARGET   vix::env)
set(VIX_PATH_TARGET  vix::path)

# --------------------------------------------------------------------
# Library target
# --------------------------------------------------------------------
if (PROCESS_SOURCES)
  message(STATUS "[process] Building STATIC library with detected sources.")

  add_library(vix_process STATIC ${PROCESS_SOURCES})
  add_library(vix::process ALIAS vix_process)

  target_compile_features(vix_process PUBLIC cxx_std_20)

  target_include_directories(vix_process
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_process
    PUBLIC
      ${VIX_ERROR_TARGET}
      ${VIX_ENV_TARGET}
      ${VIX_PATH_TARGET}
  )

  if (WIN32)
    target_link_libraries(vix_process PRIVATE
      kernel32
    )
  endif()

  # Build-only helpers must not leak into exported interfaces.
  if (NOT MSVC)
    target_compile_options(vix_process PRIVATE
      -Wall
      -Wextra
      -Wpedantic
    )
  endif()

  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(vix_process PRIVATE
      -fno-omit-frame-pointer
      -fsanitize=address,undefined
    )
    target_link_options(vix_process PRIVATE
      -fsanitize=address,undefined
    )
  endif()

  set_target_properties(vix_process PROPERTIES
    OUTPUT_NAME vix_process
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME process
  )

  install(TARGETS vix_process
    EXPORT ${VIX_PROCESS_EXPORT_SET}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

else()
  message(STATUS "[process] Building HEADER-ONLY library (no sources).")

  add_library(vix_process INTERFACE)
  add_library(vix::process ALIAS vix_process)

  target_compile_features(vix_process INTERFACE cxx_std_20)

  target_include_directories(vix_process INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_process INTERFACE
    ${VIX_ERROR_TARGET}
    ${VIX_ENV_TARGET}
    ${VIX_PATH_TARGET}
  )

  install(TARGETS vix_process
    EXPORT ${VIX_PROCESS_EXPORT_SET}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

# --------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN "*.hpp"
      PATTERN "*.h"
  )
endif()

# --------------------------------------------------------------------
# Standalone package config/export
# Only for non-umbrella builds
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vix_processConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_processConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_process"
  )

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vix_processConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vix_processConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_processConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_process"
  )

  install(EXPORT vix_processTargets
    FILE vix_processTargets.cmake
    NAMESPACE vix::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_process"
  )

endif()

# --------------------------------------------------------------------
# Tests dependency (vix::tests)
# --------------------------------------------------------------------
option(VIX_PROCESS_FETCH_TESTS "Auto-fetch vix::tests if missing" ON)

if (VIX_PROCESS_BUILD_TESTS)
  if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
    if (NOT TARGET vix::tests)
      message(FATAL_ERROR "[process] Umbrella build: vix::tests must be provided before process tests.")
    endif()
  else()
    if (NOT TARGET vix::tests)
      if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../tests/CMakeLists.txt")
        message(STATUS "[process] Adding tests from sibling: ../tests")
        add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../tests" "tests")
      else()
        find_package(vix_tests CONFIG QUIET)
      endif()

      if (NOT TARGET vix::tests)
        if (VIX_PROCESS_FETCH_TESTS)
          include(FetchContent)
          message(STATUS "[process] Fetching vix::tests via FetchContent")
          FetchContent_Declare(vix_tests
            GIT_REPOSITORY https://github.com/vixcpp/tests.git
            GIT_TAG        v0.1.0
          )
          FetchContent_MakeAvailable(vix_tests)
        else()
          message(FATAL_ERROR "vix::tests not found.")
        endif()
      endif()
    endif()
  endif()
endif()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
option(VIX_PROCESS_BUILD_TESTS "Build process module tests" OFF)

if (VIX_PROCESS_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()

# --------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------
option(VIX_PROCESS_BUILD_EXAMPLES "Build process module examples" OFF)

if (VIX_PROCESS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# --------------------------------------------------------------------
# Summary
# --------------------------------------------------------------------
message(STATUS "------------------------------------------------------")
message(STATUS "vix::process configured (${PROJECT_VERSION})")
if (PROCESS_SOURCES)
  message(STATUS "Mode: STATIC / sources found")
else()
  message(STATUS "Mode: HEADER-ONLY / no sources")
endif()
message(STATUS "Export set: ${VIX_PROCESS_EXPORT_SET}")
message(STATUS "Error target: ${VIX_ERROR_TARGET}")
message(STATUS "Env target:   ${VIX_ENV_TARGET}")
message(STATUS "Path target:  ${VIX_PATH_TARGET}")
message(STATUS "Include dir: ${CMAKE_CURRENT_SOURCE_DIR}/include (for <vix/process/...>)")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
