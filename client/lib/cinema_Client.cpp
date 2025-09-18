#include "cinema_Client.hpp"
#include <thread>
#include <chrono>

Shows::Shows(const std::string& m, const std::string& dt, const std::string& t) 
    : movie(m), dateTime(dt), theater(t), seatsMutex(std::make_shared<std::shared_mutex>()) {
    seats.resize(20, false); 
}

std::vector<uint8_t> Shows::getAvailableSeats() const {
    std::shared_lock<std::shared_mutex> lock(*seatsMutex);
    std::vector<uint8_t> availableSeats;
    for (size_t i = 0; i < seats.size(); ++i) {
        if (!seats[i]) {
            availableSeats.push_back(static_cast<uint8_t>(i + 1));
        }
    }
    return availableSeats;
}

void Shows::updateSeatAvailability(const std::vector<bool>& seatStatus) {
    std::unique_lock<std::shared_mutex> lock(*seatsMutex);
    if (seatStatus.size() == seats.size()) {
        seats = seatStatus;
    }
}