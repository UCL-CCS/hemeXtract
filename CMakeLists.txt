cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(hemeXtract
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(argp REQUIRED)

add_executable(hemeXtract hemeXtract.cc)

target_include_directories(hemeXtract PRIVATE ${argp_INCLUDE_DIRS})
target_link_libraries(hemeXtract PRIVATE ${argp_LIBRARIES})
