/**
 * @file websocket_client.hpp
 * @brief WebSocket client for Cinema booking system communication
 * @author Jorge Royon
 * @date 2025-09-18
 * @version 1.0
 */

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <mutex>
#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include "cinema_Client.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/**
 * @namespace CinemaProtocol
 * @brief Protocol constants for Cinema server communication
 * 
 * Defines the string patterns and prefixes used in the Cinema booking
 * protocol for parsing server responses and identifying message types.
 */
namespace CinemaProtocol {
    constexpr const char* THEATER_PREFIX = "Theater: ";           ///< Prefix for theater lines
    constexpr const char* MOVIE_PREFIX = "  Movie: ";            ///< Prefix for movie lines  
    constexpr const char* CINEMA_DATA_STREAM = "=== CINEMA DATA STREAM ===";     ///< Cinema data header
    constexpr const char* UPDATED_CINEMA_DATA = "=== UPDATED CINEMA DATA ===";  ///< Update data header
    constexpr const char* BOOKING_UPDATE = "BOOKING_UPDATE:";    ///< Booking update prefix
    constexpr size_t THEATER_PREFIX_LEN = 9;                     ///< Length of theater prefix
    constexpr size_t MOVIE_PREFIX_LEN = 9;                       ///< Length of movie prefix
}

/**
 * @class CinemaClient
 * @brief WebSocket client for Cinema booking system
 * 
 * Provides communication layer between Cinema client application and server.
 * Handles connection management, message sending/receiving, and protocol parsing.
 * Maintains local cache of Shows data synchronized with server.
 * 
 * @par Thread Safety
 * This class is thread-safe. All public methods can be called from multiple threads.
 * Internal message processing runs on a separate background thread.
 * 
 * @par Connection Lifecycle
 * 1. Create CinemaClient instance
 * 2. Call connect() to establish WebSocket connection
 * 3. Send messages via sendMessage()
 * 4. Receive responses via getLastResponse()/getLastBookingResponse()
 * 5. Get synchronized Shows data via getShows()
 * 6. Call disconnect() or destructor handles cleanup
 * 
 * @par Protocol Support
 * - Cinema data streams (theater/movie listings)
 * - Booking requests and responses  
 * - Real-time booking updates
 * - Automatic Shows data synchronization
 */
class CinemaClient {
public:
    /**
     * @brief Constructor
     * @post Client is created but not connected
     * @post Background thread is not started
     */
    CinemaClient();
    
    /**
     * @brief Destructor
     * @post Stops message listener thread
     * @post Disconnects from server if connected
     * @post All resources are cleaned up
     */
    ~CinemaClient();
    
    /**
     * @brief Connect to Cinema server
     * @param host Server hostname or IP address
     * @param port Server port number  
     * @return true if connection successful, false otherwise
     * @pre host and port must not be empty
     * @post On success: connected_ is true, message listener started
     * @post On failure: connected_ remains false
     * @note Thread-safe operation
     */
    bool connect(const std::string& host, const std::string& port);
    
    /**
     * @brief Disconnect from server
     * @post connected_ is false
     * @post Message listener thread is stopped
     * @post WebSocket connection is closed gracefully
     * @note Thread-safe operation
     * @note Safe to call multiple times
     */
    void disconnect();
    
    /**
     * @brief Check connection status
     * @return true if connected to server, false otherwise
     * @note Thread-safe operation
     */
    bool isConnected() const;
    
    /**
     * @brief Send message to server
     * @param message Message string to send
     * @pre Must be connected to server
     * @post Message is sent via WebSocket
     * @note Thread-safe operation
     * @note If not connected, error is logged and no action taken
     */
    void sendMessage(const std::string& message);
    
    /**
     * @brief Get last server response
     * @return Last received server message
     * @note Thread-safe operation
     * @note Returns copy of last response for safety
     */
    std::string getLastResponse() const;
    
    /**
     * @brief Get last booking response  
     * @return Last booking-specific server response (SUCCESS/ERROR messages)
     * @note Thread-safe operation
     * @note Only returns responses containing SUCCESS: or ERROR:
     */
    std::string getLastBookingResponse() const;
    
    /**
     * @brief Parse server response and update Shows data
     * @param response Server response string to parse
     * @post Shows data is updated with parsed information
     * @note Thread-safe operation
     * @note Automatically called by message listener
     */
    void parseAndUpdateShows(const std::string& response);
    
    /**
     * @brief Get current Shows data
     * @return Copy of current Shows vector
     * @note Thread-safe operation
     * @note Returns snapshot of current data
     * @note Data is automatically synchronized with server
     */
    std::vector<Shows> getShows() const;

private:
    net::io_context ioc_;                          ///< Boost.Asio I/O context
    websocket::stream<tcp::socket> ws_;            ///< WebSocket stream
    std::atomic<bool> connected_{false};           ///< Connection status flag

    std::unique_ptr<std::thread> listenerThread_;  ///< Background message listener thread
    std::atomic<bool> shouldStop_{false};          ///< Thread stop flag

    std::string lastResponse_;                      ///< Last server response
    std::string lastBookingResponse_;               ///< Last booking response
    mutable std::mutex responseMutex_;              ///< Protects response data

    std::vector<Shows> shows_;                      ///< Cached Shows data
    mutable std::mutex showsMutex_;                 ///< Protects Shows data

    /**
     * @brief Start background message listener thread
     * @post Background thread is running and processing messages
     * @note Called automatically by connect()
     */
    void startMessageListener();
    
    /**
     * @brief Stop background message listener thread
     * @post Background thread is stopped and joined
     * @note Called automatically by disconnect() and destructor
     */
    void stopMessageListener();
    
    /**
     * @brief Process incoming server message
     * @param response Server message to process
     * @post Appropriate handler is called based on message type
     * @note Called by background message listener thread
     */
    void processMessage(const std::string& response);
    
    /**
     * @brief Check if response contains cinema data
     * @param response Server response to check
     * @return true if response contains cinema data stream
     * @note Used to identify parseable cinema data
     */
    bool isCinemaDataStream(const std::string& response) const;
    
    /**
     * @brief Handle booking update messages
     * @param response Booking update response
     * @post Response is stored and Shows data updated
     * @note Handles real-time booking notifications
     */
    void handleBookingUpdate(const std::string& response);
    
    /**
     * @brief Handle general server messages
     * @param response General server message
     * @post Message is displayed to user
     * @note Handles non-data server communications
     */
    void handleServerMessage(const std::string& response);
};