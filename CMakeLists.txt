cmake_minimum_required(VERSION 4.4.2)

project(torrnado LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(torrnado
  src/main.cpp
  src/bencode.cpp
)

target_compile_features(torrnado PRIVATE cxx_std_26)
