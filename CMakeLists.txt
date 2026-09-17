cmake_minimum_required(VERSION 4.4.2)

project(torrnado LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

set_property(
  CACHE CMAKE_BUILD_TYPE
  PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel
)

option(
  TORRNADO_ENABLE_SANITIZERS
  "Enable AddressSanitizer and UndefinedBehaviorSanitizer"
  OFF
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(torrnado
  lib/bencode.cpp
)

target_include_directories(torrnado
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

target_compile_features(torrnado PUBLIC cxx_std_26)

if(TORRNADO_ENABLE_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
      torrnado
      PUBLIC
        -fsanitize=address,undefined
        -fno-sanitize-recover=undefined
        -fno-omit-frame-pointer
    )

    target_link_options(
      torrnado
      INTERFACE
        -fsanitize=address,undefined
    )
  else()
    message(FATAL_ERROR "Sanitizers require GCC or Clang")
  endif()
endif()

add_executable(torrnado_cli
  app/main.cpp
)

set_target_properties(torrnado_cli PROPERTIES
  OUTPUT_NAME torrnado
)

target_link_libraries(torrnado_cli PRIVATE torrnado)

include(CTest)

if(BUILD_TESTING)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL

    "https://github.com/google/googletest/archive/063de7e9578f82b369302001269680b4b1553359.tar.gz"
    URL_HASH

    "SHA256=623fe05d283020f37a579a8985ba338091ac7229da62b5591ff40acdf05b95e2"
  )

  FetchContent_MakeAvailable(googletest)

  add_executable(bencode_tests
    tests/bencode_tests.cpp
  )

  target_link_libraries(bencode_tests PRIVATE torrnado GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(bencode_tests)
endif()
