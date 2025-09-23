#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <sstream>

class SimpleTest {
private:
    static int tests_run;
    static int tests_passed;
    
public:
    static void EXPECT_TRUE(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            std::cout << "[FAIL] " << test_name << std::endl;
        }
    }
    
    static void EXPECT_FALSE(bool condition, const std::string& test_name) {
        EXPECT_TRUE(!condition, test_name);
    }
    
    template<typename T>
    static void EXPECT_EQ(T expected, T actual, const std::string& test_name) {
        tests_run++;
        if (expected == actual) {
            tests_passed++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            std::cout << "[FAIL] " << test_name << " (expected: " << expected << ", got: " << actual << ")" << std::endl;
        }
    }
    
    template<typename T>
    static void EXPECT_NE(T expected, T actual, const std::string& test_name) {
        tests_run++;
        if (expected != actual) {
            tests_passed++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            std::cout << "[FAIL] " << test_name << " (expected different from: " << expected << ")" << std::endl;
        }
    }
    
    static void EXPECT_CONTAINS(const std::string& haystack, const std::string& needle, const std::string& test_name) {
        EXPECT_TRUE(haystack.find(needle) != std::string::npos, test_name + " (should contain '" + needle + "')");
    }
    
    static void EXPECT_NOT_CONTAINS(const std::string& haystack, const std::string& needle, const std::string& test_name) {
        EXPECT_TRUE(haystack.find(needle) == std::string::npos, test_name + " (should not contain '" + needle + "')");
    }
    
    static void printResults() {
        std::cout << "\n=== TEST RESULTS ===" << std::endl;
        std::cout << "Tests run: " << tests_run << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << (tests_run - tests_passed) << std::endl;
        
        if (tests_passed == tests_run) {
            std::cout << "All tests PASSED! ✅" << std::endl;
        } else {
            std::cout << "Some tests FAILED! ❌" << std::endl;
        }
    }
    
    static int getFailedCount() {
        return tests_run - tests_passed;
    }
};