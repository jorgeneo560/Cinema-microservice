/**
 * @file cinema_Client.hpp
 * @brief Data structures for Cinema booking system
 * @author Jorge Royon
 * @date 2025-09-18
 * @version 1.0
 */

#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <shared_mutex>
#include <mutex>
#include <memory>

/**
 * @class Shows
 * @brief Represents a movie show with thread-safe seat management
 * 
 * This class serves as the core data structure for the Cinema booking system.
 * It provides thread-safe operations for managing seat availability and bookings
 * for individual movie shows.
 * 
 * @par Thread Safety
 * All operations are thread-safe using shared_mutex for concurrent access.
 * Multiple readers can access seat data simultaneously, but write operations
 * are exclusive.
 * 
 * @par Seat Numbering
 * Seats are numbered 1-20 inclusive. Internal storage uses 0-based indexing.
 * 
 * @invariant Total seats is always 20
 * @invariant Seat numbers are 1-20 inclusive
 * @invariant seats vector size is always 20
 */
class Shows {
public:
	std::string movie;                ///< Movie title
	std::string dateTime;             ///< Show date and time (e.g., "2025-09-11 19:30")
	std::string theater;              ///< Theater name (e.g., "PVR", "IMAX")
	std::vector<bool> seats;          ///< Seat availability (false=available, true=booked)
	
private:
	mutable std::shared_ptr<std::shared_mutex> seatsMutex; ///< Thread safety for seat operations
	
public:
	/**
	 * @brief Constructor
	 * @param m Movie title
	 * @param dt Date and time string
	 * @param t Theater name
	 * @post All 20 seats are initially available (false)
	 * @post seatsMutex is initialized and ready for use
	 */
	Shows(const std::string& m, const std::string& dt, const std::string& t);
	
	/**
	 * @brief Get list of available seat numbers
	 * @return Vector of available seat numbers (1-20)
	 * @note Thread-safe operation (shared lock)
	 * @note Returns 1-based seat numbers for user display
	 */
	std::vector<uint8_t> getAvailableSeats() const;
	
	/**
	 * @brief Book specific seats atomically
	 * @param seatNumbers Vector of seat numbers to book (1-20)
	 * @return true if ALL seats were successfully booked
	 * @retval false if ANY seat is already booked, invalid, or operation fails
	 * @note Thread-safe operation (exclusive lock)
	 * @note All-or-nothing operation - either all seats book or none
	 * @pre All seat numbers must be in range [1,20]
	 * @post On success: specified seats are marked as booked
	 * @post On failure: no seats are modified
	 */
	bool bookSeats(const std::vector<uint8_t>& seatNumbers);
	
	/**
	 * @brief Update seat availability from external source
	 * @param seatStatus New seat status vector (false=available, true=booked)
	 * @pre seatStatus.size() must equal 20
	 * @post If precondition met: seats vector is updated with new status
	 * @post If precondition not met: no changes made
	 * @note Thread-safe operation (exclusive lock)
	 * @note Used for synchronizing with server updates
	 */
	void updateSeatAvailability(const std::vector<bool>& seatStatus);
};

