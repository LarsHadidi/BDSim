cmake_minimum_required(VERSION 3.3)
project(BDumper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB_RECURSE SRCS_MAIN "src/*.cpp")
set(INC_DIRS "include")

set(INCLUDE_DIRECTORIES ${INC_DIRS})
set(SOURCE_FILES ${SRCS_MAIN})

add_definitions(-DBOOST_LOG_DYN_LINK)
include_directories(${INC_DIRS})


add_executable(BDumper ${SRCS_MAIN})
target_link_libraries(BDumper pthread boost_filesystem boost_date_time boost_program_options boost_log boost_system boost_timer boost_log_setup boost_thread)