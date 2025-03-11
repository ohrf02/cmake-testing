cmake_minimum_required(VERSION 3.10)
project(https_lib)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)

# Define the library
add_library(https_lib STATIC https_client.cpp)

# Link with WinHTTP library (Windows only)
target_link_libraries(https_lib PRIVATE Winhttp)
