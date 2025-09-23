cmake_minimum_required(VERSION 3.10.0)
project(cinema VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

# Create a shared library for cinema functionality (for testing)
add_library(cinema_lib STATIC
    server/lib/cinema.cpp
    server/lib/websocket_server.cpp
)

target_include_directories(cinema_lib PUBLIC 
    server/lib
)

target_link_libraries(cinema_lib 
    boost::boost
)

# Create the main server executable
add_executable(cinema 
    server/main.cpp
)

target_link_libraries(cinema 
    cinema_lib
    boost::boost
)

# Add client subdirectory
add_subdirectory(client)

# Enable testing and add tests subdirectory
enable_testing()
add_subdirectory(tests)
