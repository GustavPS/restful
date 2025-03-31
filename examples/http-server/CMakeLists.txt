cmake_minimum_required(VERSION 3.10)
project(example)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)

# Add executable
add_executable(example main.cpp)

# Link against the restful library
target_link_libraries(example PRIVATE restful)
