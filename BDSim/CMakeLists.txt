# Project Settings ##########################################
project ("BDSim")
cmake_minimum_required(VERSION 2.6)

# Find Libraries ############################################
find_package(Boost REQUIRED)

# Compiler & Linker Flags ###################################
set(CMAKE_CXX_FLAGS "-std=c++11 -funroll-loops -flto")
add_definitions(-DBOOST_ALL_DYN_LINK)

# Recursively build list of sources #########################
file(GLOB_RECURSE SRCS_MAIN "src/main/*.cpp")

# Include paths #############################################
set(INC_DIRS "include" "boost-simd/include" "Voro++/include")

# Includes and library paths ################################
include_directories(${INC_DIRS})
link_directories("Voro++/lib")

# Target ####################################################
add_executable(BDSim ${SRCS_MAIN})
target_link_libraries(BDSim pthread boost_filesystem boost_date_time boost_program_options boost_log boost_system boost_timer boost_log_setup boost_thread voro++_2d)
