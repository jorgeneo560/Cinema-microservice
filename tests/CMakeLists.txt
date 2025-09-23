# Tests CMakeLists.txt

add_executable(
    cinema_tests
    test_main.cpp
    test_shows.cpp
    test_cinema_service.cpp
    test_booking_service.cpp
    simple_test.cpp
)

target_include_directories(cinema_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/server/lib
    ${CMAKE_SOURCE_DIR}/tests
)

target_link_libraries(cinema_tests 
    cinema_lib
    ${Boost_LIBRARIES}
    pthread
)

add_custom_target(run_tests
    COMMAND cinema_tests
    DEPENDS cinema_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    COMMENT "Running Cinema Microservice Unit Tests"
)