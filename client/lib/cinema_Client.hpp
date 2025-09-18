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
	std::string dateTime; 
	std::string theater;  
	std::vector<bool> seats;
	
private:
	mutable std::shared_ptr<std::shared_mutex> seatsMutex;
	
public:
	Shows(const std::string& m, const std::string& dt, const std::string& t);
	std::vector<uint8_t> getAvailableSeats() const;
	bool bookSeats(const std::vector<uint8_t>& seatNumbers);
	void updateSeatAvailability(const std::vector<bool>& seatStatus);
};

