#include "cinema.hpp"
#include <thread>
#include <chrono>

Shows::Shows(const std::string& m, const std::string& dt, const std::string& t) 
    : movie(m), dateTime(dt), theater(t), seatsMutex(std::make_shared<std::shared_mutex>()) {
    seats.resize(20, false); // 20 seats, all initially available
}

std::vector<uint8_t> Shows::Seats_Availables() const {
    std::shared_lock<std::shared_mutex> lock(*seatsMutex); // SHARED lock - multiple readers allowed
    std::vector<uint8_t> availableSeats;
    for (int i = 0; i < seats.size(); ++i) {
        if (!seats[i]) { // if seat is free (false means free)
            availableSeats.push_back(static_cast<uint8_t>(i + 1)); // return 1-based seat numbers
        }
    }
    return availableSeats;
}

bool Shows::bookSeats(const std::vector<uint8_t>& seatNumbers) {
    std::unique_lock<std::shared_mutex> lock(*seatsMutex); // EXCLUSIVE lock - single writer

    // Check if all requested seats are available
    for (uint8_t seat : seatNumbers) {
        if (seat < 1 || seat > seats.size() || seats[seat - 1]) {
            return false; // Seat number out of range or already booked
        }
    }
    // Book the seats
    for (uint8_t seat : seatNumbers) {
        seats[seat - 1] = true; // Mark seat as booked
    }
    return true;
}