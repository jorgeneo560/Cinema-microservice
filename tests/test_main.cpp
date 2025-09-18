#include "simple_test.hpp"
#include <iostream>

// Function declarations from test files
void run_shows_tests();
void run_cinema_service_tests();
void run_booking_service_tests();

int main() {
    std::cout << "Cinema service testing" << std::endl;
    std::cout << std::endl;
    
    try {
        // Run all test suites
        std::cout << "Running Shows Class Tests..." << std::endl;
        run_shows_tests();
        
        std::cout << "\nRunning Cinema Service Tests..." << std::endl;
        run_cinema_service_tests();
        
        std::cout << "\nRunning Booking Service Tests..." << std::endl;
        run_booking_service_tests();
        
        // Print final results
        std::cout << "\n" << std::string(60, '=') << std::endl;
        SimpleTest::printResults();
        std::cout << std::string(60, '=') << std::endl;
        
        // Return appropriate exit code
        int failedTests = SimpleTest::getFailedCount();
        if (failedTests == 0) {
            std::cout << "\nAll tests passed! Your Cinema Microservice is working perfectly!" << std::endl;
            std::cout << "Ready for production deployment!" << std::endl;
            return 0;
        } else {
            std::cout << "\n " << failedTests << " test(s) failed. Please review and fix the issues." << std::endl;
            std::cout << "Check the failed tests above for details." << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "\n Exception during testing: " << e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cout << "\n Unknown exception during testing!" << std::endl;
        return 3;
    }
}