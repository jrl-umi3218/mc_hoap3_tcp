cmake_minimum_required(VERSION 3.1)

project(mc_hoap3_tcp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

find_package(mc_rtc REQUIRED)

add_subdirectory(src)

option(DISABLE_TESTS "Disable unit tests" ON)
if(NOT ${DISABLE_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif()
