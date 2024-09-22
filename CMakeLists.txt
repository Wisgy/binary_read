cmake_minimum_required(VERSION 3.10)

# Set the project name
project(BinaryRead)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory for header files
include_directories(include)

# find the Boost package
find_package(Boost REQUIRED COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost is required but not found")
endif()

add_subdirectory(src)