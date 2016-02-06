cmake_minimum_required(VERSION 2.6)
project ("DTracker")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -flto -mtune=native")
add_definitions(-DBOOST_ALL_DYN_LINK)

file(GLOB_RECURSE SOURCE_FILES "src/main/*.cpp")
set(INC_DIRS "include")

include_directories(${INC_DIRS})
add_executable(DTracker ${SOURCE_FILES})
target_link_libraries(DTracker pthread boost_filesystem boost_date_time boost_program_options boost_log boost_system boost_timer boost_log_setup boost_thread igraph gsl gslcblas)
