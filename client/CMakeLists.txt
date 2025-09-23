cmake_minimum_required(VERSION 3.15)
project(CinemaClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Create CinemaClientLib library
add_library(CinemaClientLib STATIC
    lib/cinema_Client.cpp
    lib/websocket_client.cpp
    lib/CinemaUI.cpp
)

# Include directories for the library
target_include_directories(CinemaClientLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Link libraries to the library target
target_link_libraries(CinemaClientLib
    Boost::system
    Threads::Threads
)

# Create the main executable
add_executable(cinema_client main.cpp)

# Link the library to the executable
target_link_libraries(cinema_client
    CinemaClientLib
)

# Include directories for the executable
target_include_directories(cinema_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set output directory
set_target_properties(cinema_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/client
)