cmake_minimum_required(VERSION 4.4.2)

project(torrnado LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(torrnado
  lib/bencode.cpp
)

target_include_directories(torrnado
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

target_compile_features(torrnado PUBLIC cxx_std_26)

add_executable(torrnado_cli
  app/main.cpp
)

set_target_properties(torrnado_cli PROPERTIES
  OUTPUT_NAME torrnado
)

target_link_libraries(torrnado_cli PRIVATE torrnado)

