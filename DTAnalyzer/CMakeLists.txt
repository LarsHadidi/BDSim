# Project Settings ##########################################
project ("DTAnalyzer")
cmake_minimum_required(VERSION 2.6)

# Find Libraries ############################################
find_package(Boost REQUIRED)

# Compiler & Linker Flags ###################################
set(CMAKE_CXX_FLAGS "-std=c++11 -msse2 -funroll-loops -flto -mtune=native -fopenmp")
add_definitions(-DBOOST_ALL_DYN_LINK)

# Recursively build list of sources #########################
file(GLOB_RECURSE SRCS_MAIN "src/main/*.cpp")

# Include paths #############################################
set(INC_DIRS "include" "boost-simd/include")

# Includes and library paths ################################
include_directories(${INC_DIRS})

# Target ####################################################
add_executable(DTAnalyzer ${SRCS_MAIN})
target_link_libraries(DTAnalyzer pthread boost_filesystem boost_date_time boost_program_options boost_log boost_system boost_timer boost_log_setup boost_thread)