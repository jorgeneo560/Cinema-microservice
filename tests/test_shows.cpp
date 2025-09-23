#include "simple_test.hpp"
#include "cinema.hpp"

void test_shows_basic_functionality() {
    std::cout << "\n=== Testing Shows Basic Functionality ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    SimpleTest::EXPECT_EQ(std::string("Inception"), show.movie, "Movie name initialization");
    SimpleTest::EXPECT_EQ(std::string("2025-09-11 19:30"), show.dateTime, "Show date time initialization");
    SimpleTest::EXPECT_EQ(std::string("PVR"), show.theater, "Theater name initialization");
    SimpleTest::EXPECT_EQ(20, (int)show.seats.size(), "Default seats size should be 20");
}

void test_shows_seat_availability() {
    std::cout << "\n=== Testing Shows Seat Availability ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    
    // Set pattern: false=available, true=booked
    std::vector<bool> pattern = {
        true, false, true, false, true,   // Seats 1-5: 1,3,5 booked, 2,4 available
        false, true, false, true, true,   // Seats 6-10: 6,8 available, 7,9,10 booked
        true, false, true, false, true,   // Seats 11-15: 11,13,15 booked, 12,14 available
        false, true, false, true, true    // Seats 16-20: 16,18 available, 17,19,20 booked
    };
    show.seats = pattern;
    
    auto available = show.getAvailableSeats();
    SimpleTest::EXPECT_EQ(8, (int)available.size(), "Should have 8 available seats");
    
    SimpleTest::EXPECT_EQ(2, (int)available[0], "First available seat should be 2");
    SimpleTest::EXPECT_EQ(4, (int)available[1], "Second available seat should be 4");
    SimpleTest::EXPECT_EQ(6, (int)available[2], "Third available seat should be 6");
    SimpleTest::EXPECT_EQ(8, (int)available[3], "Fourth available seat should be 8");
}

void test_shows_booking_success() {
    std::cout << "\n=== Testing Shows Successful Booking ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    
    std::vector<bool> pattern(20, false);  // All seats available (false = available)
    show.seats = pattern;
    
    std::vector<uint8_t> seatsToBook = {1, 5, 10};
    bool result = show.bookSeats(seatsToBook);
    
    SimpleTest::EXPECT_TRUE(result, "Booking available seats should succeed");
    SimpleTest::EXPECT_TRUE(show.seats[0], "Seat 1 should be booked (true)");
    SimpleTest::EXPECT_TRUE(show.seats[4], "Seat 5 should be booked (true)");
    SimpleTest::EXPECT_TRUE(show.seats[9], "Seat 10 should be booked (true)");
    
    auto available = show.getAvailableSeats();
    SimpleTest::EXPECT_EQ(17, (int)available.size(), "Should have 17 available seats after booking 3");
}

void test_shows_booking_failures() {
    std::cout << "\n=== Testing Shows Booking Failures ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    
    std::vector<bool> pattern = {
        true, false, true, false, false,   // Seats 1,3 booked, 2,4,5 available
        false, false, false, false, false, // Seats 6-10: all available
        false, false, false, false, false, // Seats 11-15: all available
        false, false, false, false, false  // Seats 16-20: all available
    };
    show.seats = pattern;
    
    // Test booking already occupied seat
    std::vector<uint8_t> occupiedSeat = {1};
    bool result1 = show.bookSeats(occupiedSeat);
    SimpleTest::EXPECT_FALSE(result1, "Booking occupied seat should fail");
    
    // Test booking invalid seat number (0)
    std::vector<uint8_t> invalidSeat1 = {0};
    bool result2 = show.bookSeats(invalidSeat1);
    SimpleTest::EXPECT_FALSE(result2, "Booking seat 0 should fail");
    
    // Test booking invalid seat number (21)
    std::vector<uint8_t> invalidSeat2 = {21};
    bool result3 = show.bookSeats(invalidSeat2);
    SimpleTest::EXPECT_FALSE(result3, "Booking seat 21 should fail");
    
    // Test booking mix of valid and invalid seats
    std::vector<uint8_t> mixedSeats = {2, 25};
    bool result4 = show.bookSeats(mixedSeats);
    SimpleTest::EXPECT_FALSE(result4, "Booking mix of valid and invalid seats should fail");
    SimpleTest::EXPECT_FALSE(show.seats[1], "Seat 2 should remain available after failed booking");
}

void test_shows_thread_safety_simulation() {
    std::cout << "\n=== Testing Shows Thread Safety Simulation ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    
    std::vector<bool> pattern(20, false);  // All seats available (false = available)
    show.seats = pattern;
    
    // Simulate concurrent bookings
    std::vector<uint8_t> booking1 = {1, 2, 3};
    std::vector<uint8_t> booking2 = {3, 4, 5};  // Seat 3 conflicts
    
    bool result1 = show.bookSeats(booking1);
    SimpleTest::EXPECT_TRUE(result1, "First booking should succeed");
    
    bool result2 = show.bookSeats(booking2);
    SimpleTest::EXPECT_FALSE(result2, "Second booking should fail due to seat 3 conflict");
    
    // Verify seat 3 is booked from first booking (false = available, true = booked)
    SimpleTest::EXPECT_TRUE(show.seats[2], "Seat 3 should be booked from first booking");
    // Verify seats 4 and 5 are still available
    SimpleTest::EXPECT_FALSE(show.seats[3], "Seat 4 should still be available");
    SimpleTest::EXPECT_FALSE(show.seats[4], "Seat 5 should still be available");
}

void test_shows_edge_cases() {
    std::cout << "\n=== Testing Shows Edge Cases ===" << std::endl;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    
    // Test empty booking
    std::vector<uint8_t> emptyBooking = {};
    bool result1 = show.bookSeats(emptyBooking);
    SimpleTest::EXPECT_TRUE(result1, "Empty booking should succeed (no operation)");
    
    // Test booking when all seats are occupied
    std::vector<bool> allOccupied(20, true);  // All seats booked (true = booked)
    show.seats = allOccupied;
    
    std::vector<uint8_t> anySeats = {1, 5, 10};
    bool result2 = show.bookSeats(anySeats);
    SimpleTest::EXPECT_FALSE(result2, "Booking when all seats occupied should fail");
    
    auto available = show.getAvailableSeats();
    SimpleTest::EXPECT_EQ(0, (int)available.size(), "No seats should be available when all occupied");
}

void run_shows_tests() {
    test_shows_basic_functionality();
    test_shows_seat_availability();
    test_shows_booking_success();
    test_shows_booking_failures();
    test_shows_thread_safety_simulation();
    test_shows_edge_cases();
}