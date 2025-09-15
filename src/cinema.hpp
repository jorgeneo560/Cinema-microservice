#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <shared_mutex>
#include <mutex>
#include <memory>

class Shows {
public:
	std::string movie;
	std::string dateTime; // e.g., "2025-09-11 19:30"
	std::string theater;  // Theater name
	std::vector<bool> seats;
	
private:
	mutable std::shared_ptr<std::shared_mutex> seatsMutex; // Shared pointer to shared_mutex
	
public:
	Shows(const std::string& m, const std::string& dt, const std::string& t);
	std::vector<uint8_t> Seats_Availables() const;
	bool bookSeats(const std::vector<uint8_t>& seatNumbers);
};

