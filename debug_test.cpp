#include <iostream>
#include "cinema.hpp"

void debug_shows_behavior() {
    std::cout << "=== DEBUGGING SHOWS BEHAVIOR ===" << std::endl;
    
    // Create a show with default initialization
    Shows show("Test Movie", "2025-09-11 19:30", "Test Theater");
    
    std::cout << "Default seats (after constructor):" << std::endl;
    for (int i = 0; i < show.seats.size(); ++i) {
        std::cout << "Seat " << (i+1) << ": " << (show.seats[i] ? "BOOKED" : "AVAILABLE") << std::endl;
    }

    std::cout << "\nAvailable seats from getAvailableSeats():" << std::endl;
    auto available = show.getAvailableSeats();
    for (auto seat : available) {
        std::cout << "Seat " << (int)seat << " ";
    }
    std::cout << "\nTotal available: " << available.size() << std::endl;
    
    // Test booking
    std::vector<uint8_t> seatsToBook = {1, 2, 3};
    bool result = show.bookSeats(seatsToBook);
    std::cout << "\nBooking seats 1,2,3: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "After booking:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Seat " << (i+1) << ": " << (show.seats[i] ? "BOOKED" : "AVAILABLE") << std::endl;
    }

    auto available2 = show.getAvailableSeats();
    std::cout << "Available count after booking: " << available2.size() << std::endl;
}

int main() {
    debug_shows_behavior();
    return 0;
}