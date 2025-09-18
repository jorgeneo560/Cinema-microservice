#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <shared_mutex>
#include <mutex>
#include <memory>
#include <sstream>
#include <algorithm>

class Shows {
public:
	std::string movie;
	std::string dateTime;
	std::string theater;
	std::vector<bool> seats;
	
private:
	mutable std::shared_ptr<std::shared_mutex> seatsMutex;
	
public:
	Shows(const std::string& m, const std::string& dt, const std::string& t);
	std::vector<uint8_t> getAvailableSeats() const;
	bool bookSeats(const std::vector<uint8_t>& seatNumbers);
};

class CinemaService {
public:
    static std::string formatCinemaData(const std::vector<Shows>& shows);
    static std::string formatUpdateData(const std::vector<Shows>& shows);
};

class BookingService {
public:
    struct BookingResult {
        bool success;
        std::string message;
        bool shouldBroadcast;
    };
    
    static BookingResult processBooking(const std::string& message, std::vector<Shows>& shows);
};

class MessageHandler {
public:
    static std::string handleMessage(const std::string& received, std::vector<Shows>& shows, bool& shouldBroadcast);
};

