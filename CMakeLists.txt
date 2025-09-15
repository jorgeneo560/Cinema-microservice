cmake_minimum_required(VERSION 3.10.0)
project(cinema VERSION 0.1.0 LANGUAGES C CXX)


find_package(Boost REQUIRED)
add_executable(cinema 
    src/main.cpp
    src/cinema.cpp
    src/websocket_server.cpp
)
target_link_libraries(cinema boost::boost)
