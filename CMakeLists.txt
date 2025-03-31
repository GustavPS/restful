cmake_minimum_required(VERSION 3.16)
project(restful LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the library (static or shared)
add_library(restful STATIC
    src/http_server.cpp
    src/http_response.cpp
    src/http_request.cpp
    src/http_header.cpp
    src/http_route.cpp
    src/http_path.cpp
    src/http_router.cpp
)

find_package(nlohmann_json REQUIRED)
target_link_libraries(restful PRIVATE nlohmann_json::nlohmann_json)

target_sources(restful PRIVATE src/utils.cpp)

# Specify public include directories
target_include_directories(restful
PRIVATE src
PUBLIC include)

# Optional: Install rules
install(TARGETS restful DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

add_subdirectory(examples)