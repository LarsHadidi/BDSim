cmake_minimum_required(VERSION 2.6)
project(BDumper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -flto -mtune=native")
add_definitions(-DBOOST_LOG_DYN_LINK)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
set(INC_DIRS "include")

include_directories(${INC_DIRS})
add_executable(BDumper ${SOURCE_FILES})
target_link_libraries(BDumper pthread boost_filesystem boost_date_time boost_program_options boost_log boost_system boost_timer boost_log_setup boost_thread)
