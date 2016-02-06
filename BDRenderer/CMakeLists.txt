cmake_minimum_required(VERSION 2.6)
project(BDRenderer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -flto -mtune=native")
add_definitions(-DBOOST_LOG_DYN_LINK)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
set(INC_DIRS "src")

include_directories(${INC_DIRS})
add_executable(BDRenderer ${SOURCE_FILES})
target_link_libraries(BDRenderer opencv_imgproc opencv_highgui opencv_core pthread boost_program_options boost_log boost_system boost_filesystem gsl gslcblas)
