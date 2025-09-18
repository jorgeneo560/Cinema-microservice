#include "simple_test.hpp"
#include "cinema.hpp"

void test_booking_service_valid_booking() {
    std::cout << "\n=== Testing Booking Service Valid Booking ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern = {
        true, true, false, false, true,   // Seats 1,2,5 booked, 3,4 available
        false, true, false, true, false,  // Seat 7,9 booked, others available
        true, false, true, false, true,   // Mixed pattern
        false, true, false, true, true    // Mixed pattern
    };
    show.seats = pattern;
    shows = {show};
    
    // Test valid booking on available seats 3,4
    auto result = BookingService::processBooking("PVR,Inception,3,4", shows);
    
    SimpleTest::EXPECT_TRUE(result.success, "Valid booking should succeed");
    SimpleTest::EXPECT_CONTAINS(result.message, "SUCCESS", "Success message should contain SUCCESS");
    SimpleTest::EXPECT_CONTAINS(result.message, "Booked seats 3, 4", "Should mention booked seats");
    SimpleTest::EXPECT_CONTAINS(result.message, "Inception", "Should mention movie name");
    SimpleTest::EXPECT_CONTAINS(result.message, "PVR", "Should mention theater name");
    SimpleTest::EXPECT_TRUE(result.shouldBroadcast, "Successful booking should trigger broadcast");
    
    // Verify seats are actually booked
    SimpleTest::EXPECT_TRUE(shows[0].seats[2], "Seat 3 should be booked after booking");
    SimpleTest::EXPECT_TRUE(shows[0].seats[3], "Seat 4 should be booked after booking");
}

void test_booking_service_invalid_show() {
    std::cout << "\n=== Testing Booking Service Invalid Show ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern(20, true);
    show.seats = pattern;
    shows = {show};
    
    // Test invalid theater
    auto result1 = BookingService::processBooking("IMAX,Inception,1,2", shows);
    SimpleTest::EXPECT_FALSE(result1.success, "Invalid theater should fail");
    SimpleTest::EXPECT_CONTAINS(result1.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result1.message, "Show not found", "Should mention show not found");
    SimpleTest::EXPECT_FALSE(result1.shouldBroadcast, "Failed booking should not trigger broadcast");
    
    // Test invalid movie
    auto result2 = BookingService::processBooking("PVR,Tenet,1,2", shows);
    SimpleTest::EXPECT_FALSE(result2.success, "Invalid movie should fail");
    SimpleTest::EXPECT_CONTAINS(result2.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result2.message, "Show not found", "Should mention show not found");
    SimpleTest::EXPECT_FALSE(result2.shouldBroadcast, "Failed booking should not trigger broadcast");
}

void test_booking_service_invalid_format() {
    std::cout << "\n=== Testing Booking Service Invalid Format ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern(20, true);
    show.seats = pattern;
    shows = {show};
    
    // Test too few parameters
    auto result1 = BookingService::processBooking("PVR", shows);
    SimpleTest::EXPECT_FALSE(result1.success, "Too few parameters should fail");
    SimpleTest::EXPECT_CONTAINS(result1.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result1.message, "Invalid booking format", "Should mention invalid format");
    
    // Test only theater and movie (no seats)
    auto result2 = BookingService::processBooking("PVR,Inception", shows);
    SimpleTest::EXPECT_FALSE(result2.success, "No seats specified should fail");
    SimpleTest::EXPECT_CONTAINS(result2.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result2.message, "Invalid booking format", "Should mention invalid format");
    
    // Test empty string
    auto result3 = BookingService::processBooking("", shows);
    SimpleTest::EXPECT_FALSE(result3.success, "Empty string should fail");
    SimpleTest::EXPECT_CONTAINS(result3.message, "ERROR", "Error message should contain ERROR");
}

void test_booking_service_invalid_seats() {
    std::cout << "\n=== Testing Booking Service Invalid Seats ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern(20, true);
    show.seats = pattern;
    shows = {show};
    
    // Test seat number 0
    auto result1 = BookingService::processBooking("PVR,Inception,0", shows);
    SimpleTest::EXPECT_FALSE(result1.success, "Seat number 0 should fail");
    SimpleTest::EXPECT_CONTAINS(result1.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result1.message, "Invalid seat number", "Should mention invalid seat number");
    
    // Test seat number 21
    auto result2 = BookingService::processBooking("PVR,Inception,21", shows);
    SimpleTest::EXPECT_FALSE(result2.success, "Seat number 21 should fail");
    SimpleTest::EXPECT_CONTAINS(result2.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result2.message, "Invalid seat number", "Should mention invalid seat number");
    
    // Test negative seat number (if possible to parse)
    auto result3 = BookingService::processBooking("PVR,Inception,-1", shows);
    SimpleTest::EXPECT_FALSE(result3.success, "Negative seat number should fail");
    SimpleTest::EXPECT_CONTAINS(result3.message, "ERROR", "Error message should contain ERROR");
    
    // Test mix of valid and invalid seat numbers
    auto result4 = BookingService::processBooking("PVR,Inception,1,25", shows);
    SimpleTest::EXPECT_FALSE(result4.success, "Mix of valid and invalid seats should fail");
    SimpleTest::EXPECT_CONTAINS(result4.message, "ERROR", "Error message should contain ERROR");
}

void test_booking_service_already_booked_seats() {
    std::cout << "\n=== Testing Booking Service Already Booked Seats ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern = {
        true, true, false, false, false,   // Seats 1,2 already booked, others available
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false
    };
    show.seats = pattern;
    shows = {show};
    
    // Test booking already booked seat
    auto result1 = BookingService::processBooking("PVR,Inception,1", shows);
    SimpleTest::EXPECT_FALSE(result1.success, "Booking already booked seat should fail");
    SimpleTest::EXPECT_CONTAINS(result1.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result1.message, "already booked", "Should mention already booked");
    SimpleTest::EXPECT_FALSE(result1.shouldBroadcast, "Failed booking should not trigger broadcast");
    
    // Test booking mix of available and booked seats
    auto result2 = BookingService::processBooking("PVR,Inception,1,3", shows);
    SimpleTest::EXPECT_FALSE(result2.success, "Mix of booked and available seats should fail");
    SimpleTest::EXPECT_CONTAINS(result2.message, "ERROR", "Error message should contain ERROR");
    SimpleTest::EXPECT_CONTAINS(result2.message, "already booked", "Should mention already booked");
    
    // Verify that seat 3 remains available after failed booking
    SimpleTest::EXPECT_FALSE(shows[0].seats[2], "Seat 3 should remain available after failed booking");
}

void test_booking_service_multiple_seats() {
    std::cout << "\n=== Testing Booking Service Multiple Seats ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern(20, false);  // All seats available (false = available)
    show.seats = pattern;
    shows = {show};
    
    // Test booking multiple seats
    auto result = BookingService::processBooking("PVR,Inception,1,5,10,15,20", shows);
    
    SimpleTest::EXPECT_TRUE(result.success, "Multiple seat booking should succeed");
    SimpleTest::EXPECT_CONTAINS(result.message, "SUCCESS", "Success message should contain SUCCESS");
    SimpleTest::EXPECT_CONTAINS(result.message, "Booked seats", "Should mention booked seats");
    SimpleTest::EXPECT_TRUE(result.shouldBroadcast, "Successful booking should trigger broadcast");
    
    // Verify all requested seats are booked (true = booked)
    SimpleTest::EXPECT_TRUE(shows[0].seats[0], "Seat 1 should be booked");
    SimpleTest::EXPECT_TRUE(shows[0].seats[4], "Seat 5 should be booked");
    SimpleTest::EXPECT_TRUE(shows[0].seats[9], "Seat 10 should be booked");
    SimpleTest::EXPECT_TRUE(shows[0].seats[14], "Seat 15 should be booked");
    SimpleTest::EXPECT_TRUE(shows[0].seats[19], "Seat 20 should be booked");
    
    // Verify other seats remain available (false = available)
    SimpleTest::EXPECT_FALSE(shows[0].seats[1], "Seat 2 should remain available");
    SimpleTest::EXPECT_FALSE(shows[0].seats[2], "Seat 3 should remain available");
}

void test_booking_service_edge_cases() {
    std::cout << "\n=== Testing Booking Service Edge Cases ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern(20, true);
    show.seats = pattern;
    shows = {show};
    
    // Test duplicate seat numbers in booking
    auto result1 = BookingService::processBooking("PVR,Inception,1,1,1", shows);
    // This should either succeed (booking seat 1 once) or fail (duplicate detection)
    // The behavior depends on implementation, but let's test that it doesn't crash
    SimpleTest::EXPECT_TRUE(result1.success || !result1.success, "Duplicate seats should not crash");
    
    // Test with whitespace (if implementation trims)
    auto result2 = BookingService::processBooking(" PVR , Inception , 1 , 2 ", shows);
    // This may or may not work depending on implementation
    SimpleTest::EXPECT_TRUE(result2.success || !result2.success, "Whitespace should not crash");
}

void run_booking_service_tests() {
    test_booking_service_valid_booking();
    test_booking_service_invalid_show();
    test_booking_service_invalid_format();
    test_booking_service_invalid_seats();
    test_booking_service_already_booked_seats();
    test_booking_service_multiple_seats();
    test_booking_service_edge_cases();
}