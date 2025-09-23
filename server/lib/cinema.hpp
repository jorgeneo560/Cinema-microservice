/**
 * @file cinema.hpp
 * @brief Core business logic and services for Cinema booking system
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
#include <sstream>
#include <algorithm>

/**
 * @class Shows
 * @brief Server-side movie show data structure with thread-safe seat management
 * 
 * Identical to client-side Shows class but used in server context.
 * Provides thread-safe operations for managing seat availability and bookings.
 * 
 * @par Thread Safety
 * All operations are thread-safe using shared_mutex for concurrent access.
 * 
 * @see Shows in cinema_Client.hpp for detailed documentation
 */
class Shows {
public:
	std::string movie;               ///< Movie title
	std::string dateTime;            ///< Show date and time
	std::string theater;             ///< Theater name
	std::vector<bool> seats;         ///< Seat availability (false=available, true=booked)
	
private:
	mutable std::shared_ptr<std::shared_mutex> seatsMutex; ///< Thread safety for seat operations
	
public:
	/**
	 * @brief Constructor
	 * @param m Movie title
	 * @param dt Date and time string
	 * @param t Theater name
	 * @post All 20 seats are initially available
	 */
	Shows(const std::string& m, const std::string& dt, const std::string& t);
	
	/**
	 * @brief Get list of available seat numbers
	 * @return Vector of available seat numbers (1-20)
	 * @note Thread-safe operation
	 */
	std::vector<uint8_t> getAvailableSeats() const;
	
	/**
	 * @brief Book specific seats atomically
	 * @param seatNumbers Vector of seat numbers to book (1-20)
	 * @return true if ALL seats were successfully booked
	 * @note Thread-safe operation
	 * @note All-or-nothing operation
	 */
	bool bookSeats(const std::vector<uint8_t>& seatNumbers);
};

/**
 * @class CinemaService
 * @brief Service for formatting cinema data for client communication
 * 
 * Provides static methods to format Shows data into protocol-compliant
 * strings for transmission to clients. Handles both initial data streams
 * and booking update notifications.
 * 
 * @par Output Format
 * Generates structured text output with theater grouping, movie listings,
 * and seat availability information in a human-readable format.
 */
class CinemaService {
public:
    /**
     * @brief Format complete cinema data for client transmission
     * @param shows Vector of all available shows
     * @return Formatted cinema data string with headers and theater grouping
     * @post Returns protocol-compliant cinema data stream
     * @note Groups shows by theater for organized display
     * @note Includes availability information for each show
     */
    static std::string formatCinemaData(const std::vector<Shows>& shows);
    
    /**
     * @brief Format booking update data for client notification
     * @param shows Vector of updated shows data
     * @return Formatted update data string with booking update headers
     * @post Returns protocol-compliant booking update stream
     * @note Used for real-time client synchronization after bookings
     * @note Similar format to formatCinemaData but with update headers
     */
    static std::string formatUpdateData(const std::vector<Shows>& shows);
};

/**
 * @class BookingService
 * @brief Service for processing seat booking requests
 * 
 * Handles parsing of booking requests, validation of seat numbers,
 * and execution of booking operations. Provides comprehensive error
 * handling and user feedback.
 */
class BookingService {
public:
    /**
     * @struct BookingResult
     * @brief Result of a booking operation
     * 
     * Encapsulates the outcome of a booking request including success status,
     * user message, and whether other clients should be notified.
     */
    struct BookingResult {
        bool success;           ///< Whether booking succeeded
        std::string message;    ///< Response message for client
        bool shouldBroadcast;   ///< Whether to notify other clients
    };
    
    /**
     * @brief Process a booking request message
     * @param message Raw booking request string from client
     * @param shows Reference to shows vector to modify
     * @return BookingResult with operation outcome
     * @pre message format: "theater,movie,seat1,seat2,..."
     * @pre shows vector contains valid show data
     * @post On success: specified seats are booked, shows vector updated
     * @post On failure: no changes made to shows vector
     * 
     * @par Message Format
     * Expected format: "TheaterName,MovieTitle,SeatNumber1,SeatNumber2,..."
     * Example: "PVR,Inception,1,2,3"
     * 
     * @par Validation
     * - Verifies show exists (theater + movie combination)
     * - Validates all seat numbers are in range [1,20]
     * - Checks all seats are available before booking
     * - Provides detailed error messages for failures
     */
    static BookingResult processBooking(const std::string& message, std::vector<Shows>& shows);
};

/**
 * @class MessageHandler
 * @brief High-level message routing and processing service
 * 
 * Provides unified interface for handling all types of client messages.
 * Routes messages to appropriate services and coordinates responses.
 * Acts as the main entry point for server message processing.
 */
class MessageHandler {
public:
    /**
     * @brief Handle any client message and generate appropriate response
     * @param received Raw message received from client
     * @param shows Reference to shows vector (may be modified)
     * @param shouldBroadcast Output parameter indicating if response should be broadcast
     * @return Response message to send back to client
     * @pre received contains valid client message
     * @post shouldBroadcast indicates whether other clients should receive response
     * @post shows vector may be modified for booking operations
     * 
     * @par Message Types Handled
     * - "get_data" - Returns formatted cinema data
     * - Booking requests - Processes seat bookings
     * - Unknown messages - Returns error response
     * 
     * @par Broadcast Logic
     * - Data requests: no broadcast needed
     * - Successful bookings: broadcast update to all clients
     * - Failed bookings: no broadcast needed
     */
    static std::string handleMessage(const std::string& received, std::vector<Shows>& shows, bool& shouldBroadcast);
};

