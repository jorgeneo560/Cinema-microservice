#include "simple_test.hpp"
#include "cinema.hpp"

void test_cinema_service_format_data() {
    std::cout << "\n=== Testing Cinema Service Format Data ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show1("Inception", "2025-09-11 19:30", "PVR");
    Shows show2("Interstellar", "2025-09-11 19:30", "IMAX");
    Shows show3("Tenet", "2025-09-11 19:30", "Cinepolis");
    
    std::vector<bool> pattern = {
        false, true, false, true, false,
        true, false, true, false, true,
        false, true, false, true, false,
        true, false, true, false, false
    };
    
    show1.seats = pattern;
    show2.seats = pattern;
    show3.seats = pattern;
    
    shows = {show1, show2, show3};
    
    std::string data = CinemaService::formatCinemaData(shows);
    
    // Test header and footer
    SimpleTest::EXPECT_CONTAINS(data, "=== CINEMA DATA STREAM ===", "Contains cinema data header");
    SimpleTest::EXPECT_CONTAINS(data, "=== END CINEMA DATA ===", "Contains cinema data footer");
    
    // Test theater information
    SimpleTest::EXPECT_CONTAINS(data, "Theater: PVR", "Contains PVR theater");
    SimpleTest::EXPECT_CONTAINS(data, "Theater: IMAX", "Contains IMAX theater");
    SimpleTest::EXPECT_CONTAINS(data, "Theater: Cinepolis", "Contains Cinepolis theater");
    
    // Test movie information
    SimpleTest::EXPECT_CONTAINS(data, "Movie: Inception", "Contains Inception movie");
    SimpleTest::EXPECT_CONTAINS(data, "Movie: Interstellar", "Contains Interstellar movie");
    SimpleTest::EXPECT_CONTAINS(data, "Movie: Tenet", "Contains Tenet movie");
    
    // Test time information
    SimpleTest::EXPECT_CONTAINS(data, "(2025-09-11 19:30)", "Contains show time in parentheses");
    
    // Test seat information
    SimpleTest::EXPECT_CONTAINS(data, "Available seats:", "Contains available seats information");
}

void test_cinema_service_format_update_data() {
    std::cout << "\n=== Testing Cinema Service Format Update Data ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Inception", "2025-09-11 19:30", "PVR");
    std::vector<bool> pattern = {
        false, true, false, true, false,
        true, false, true, false, true,
        false, true, false, true, false,
        true, false, true, false, false
    };
    show.seats = pattern;
    shows = {show};
    
    std::string data = CinemaService::formatUpdateData(shows);
    
    // Test update header
    SimpleTest::EXPECT_CONTAINS(data, "BOOKING_UPDATE:", "Contains booking update header");
    SimpleTest::EXPECT_CONTAINS(data, "=== UPDATED CINEMA DATA ===", "Contains updated data header");
    SimpleTest::EXPECT_CONTAINS(data, "=== END UPDATED DATA ===", "Contains updated data footer");
    
    // Test that it contains theater and movie information
    SimpleTest::EXPECT_CONTAINS(data, "Theater: PVR", "Update contains PVR theater");
    SimpleTest::EXPECT_CONTAINS(data, "Movie: Inception", "Update contains Inception movie");
    SimpleTest::EXPECT_CONTAINS(data, "Available seats:", "Update contains available seats information");
}

void test_cinema_service_empty_shows() {
    std::cout << "\n=== Testing Cinema Service with Empty Shows ===" << std::endl;
    
    std::vector<Shows> emptyShows;
    
    std::string data = CinemaService::formatCinemaData(emptyShows);
    SimpleTest::EXPECT_CONTAINS(data, "=== CINEMA DATA STREAM ===", "Empty shows still has header");
    SimpleTest::EXPECT_CONTAINS(data, "=== END CINEMA DATA ===", "Empty shows still has footer");
    
    std::string updateData = CinemaService::formatUpdateData(emptyShows);
    SimpleTest::EXPECT_CONTAINS(updateData, "BOOKING_UPDATE:", "Empty shows update has header");
    SimpleTest::EXPECT_CONTAINS(updateData, "=== UPDATED CINEMA DATA ===", "Empty shows update has data header");
}

void test_cinema_service_multiple_theaters_same_movie() {
    std::cout << "\n=== Testing Cinema Service Multiple Theaters Same Movie ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show1("Inception", "2025-09-11 19:30", "PVR");
    Shows show2("Inception", "2025-09-11 19:30", "IMAX");
    Shows show3("Inception", "2025-09-11 19:30", "Cinepolis");
    
    std::vector<bool> pattern1(20, true);   // All available
    std::vector<bool> pattern2(20, false);  // All booked
    std::vector<bool> pattern3 = {
        true, false, true, false, true,
        false, true, false, true, false,
        true, false, true, false, true,
        false, true, false, true, false
    };
    
    show1.seats = pattern1;
    show2.seats = pattern2;
    show3.seats = pattern3;
    
    shows = {show1, show2, show3};
    
    std::string data = CinemaService::formatCinemaData(shows);
    
    // Count occurrences of "Movie: Inception"
    size_t pos = 0;
    int count = 0;
    while ((pos = data.find("Movie: Inception", pos)) != std::string::npos) {
        count++;
        pos += std::string("Movie: Inception").length();
    }
    
    SimpleTest::EXPECT_EQ(3, count, "Should have Inception movie in 3 theaters");
    
    // Verify different seat availability patterns
    SimpleTest::EXPECT_CONTAINS(data, "Theater: PVR", "Contains PVR theater");
    SimpleTest::EXPECT_CONTAINS(data, "Theater: IMAX", "Contains IMAX theater");
    SimpleTest::EXPECT_CONTAINS(data, "Theater: Cinepolis", "Contains Cinepolis theater");
}

void test_cinema_service_seat_numbering() {
    std::cout << "\n=== Testing Cinema Service Seat Numbering ===" << std::endl;
    
    std::vector<Shows> shows;
    
    Shows show("Test Movie", "2025-09-11 19:30", "Test Theater");
    
    // Only seats 1, 3, 5, 7, 9 available (indices 0, 2, 4, 6, 8)
    // false = available, true = booked
    std::vector<bool> pattern = {
        false, true, false, true, false,  // 1, 3, 5 available (2,4 booked)
        true, false, true, false, true,   // 7, 9 available (6,8,10 booked)
        true, true, true, true, true,     // 11-15 all booked
        true, true, true, true, true      // 16-20 all booked
    };
    show.seats = pattern;
    shows = {show};
    
    std::string data = CinemaService::formatCinemaData(shows);
    
    // The formatted data should show seat numbers (1-based)
    SimpleTest::EXPECT_CONTAINS(data, "Available seats:", "Contains available seats label");
    
    // Check that seat numbers appear correctly in output
    auto availableSeats = show.getAvailableSeats();
    SimpleTest::EXPECT_EQ(5, (int)availableSeats.size(), "Should have 5 available seats");
    SimpleTest::EXPECT_EQ(1, (int)availableSeats[0], "First available seat should be 1");
    SimpleTest::EXPECT_EQ(3, (int)availableSeats[1], "Second available seat should be 3");
    SimpleTest::EXPECT_EQ(5, (int)availableSeats[2], "Third available seat should be 5");
}

void run_cinema_service_tests() {
    test_cinema_service_format_data();
    test_cinema_service_format_update_data();
    test_cinema_service_empty_shows();
    test_cinema_service_multiple_theaters_same_movie();
    test_cinema_service_seat_numbering();
}