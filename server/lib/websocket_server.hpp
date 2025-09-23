/**
 * @file websocket_server.hpp
 * @brief WebSocket server infrastructure for Cinema booking system
 * @author Jorge Royon
 * @date 2025-09-18
 * @version 1.0
 */

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <queue>
#include <functional>
#include "cinema.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/**
 * @brief Callback for handling client messages
 * @param message The received message
 * @param shouldBroadcast Output parameter indicating if response should be broadcast
 * @return Response message to send back
 */
using MessageCallback = std::function<std::string(const std::string&, bool&)>;

/**
 * @brief Callback for getting initial data to send to new clients
 * @return Initial data string
 */
using InitialDataCallback = std::function<std::string()>;

/**
 * @brief Callback for getting broadcast data after updates
 * @return Broadcast data string
 */
using BroadcastDataCallback = std::function<std::string()>;

/**
 * @namespace CinemaProtocol
 * @brief Server-side protocol constants for Cinema communication
 * 
 * Defines headers, footers, and constants used in server-client communication.
 */
namespace CinemaProtocol {
    constexpr const char* CINEMA_DATA_HEADER = "=== CINEMA DATA STREAM ===";   ///< Cinema data stream header
    constexpr const char* CINEMA_DATA_FOOTER = "=== END CINEMA DATA ===";     ///< Cinema data stream footer
    constexpr const char* UPDATE_DATA_HEADER = "=== UPDATED CINEMA DATA ==="; ///< Update data header
    constexpr const char* UPDATE_DATA_FOOTER = "=== END UPDATED DATA ===";    ///< Update data footer
    constexpr const char* BOOKING_UPDATE_PREFIX = "BOOKING_UPDATE:\n";        ///< Booking update prefix
    constexpr int MAX_SEATS = 20;                                            ///< Maximum seats per show
}

class WebSocketServer;

/**
 * @class WebSocketSession
 * @brief Represents an individual client WebSocket connection
 * 
 * Manages the lifecycle of a single WebSocket connection from client acceptance
 * through message handling to disconnection. Each connected client gets its own
 * WebSocketSession instance.
 * 
 * @par Connection Lifecycle
 * 1. Created when client connects to server
 * 2. WebSocket handshake performed
 * 3. Initial cinema data sent to client
 * 4. Continuous message processing loop
 * 5. Cleanup on disconnection or error
 * 
 * @par Message Flow
 * - Receives messages from client asynchronously
 * - Forwards messages to server for processing
 * - Sends responses back to client
 * - Participates in broadcast messages to all clients
 * 
 * @par Thread Safety
 * Uses Boost.Beast's async operations for thread-safe message handling.
 * Write operations are queued to prevent overlapping sends.
 */
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
private:
    websocket::stream<tcp::socket> ws_;          ///< WebSocket stream for this connection
    beast::flat_buffer buffer_;                  ///< Buffer for incoming messages
    WebSocketServer* server_;                    ///< Reference to parent server
    std::queue<std::string> message_queue_;      ///< Queue for outgoing messages
    bool writing_;                               ///< Flag to prevent overlapping writes

public:
    /**
     * @brief Constructor
     * @param socket TCP socket from accepted connection
     * @param server Pointer to parent WebSocket server
     * @post Session is ready to begin WebSocket handshake
     */
    explicit WebSocketSession(tcp::socket&& socket, WebSocketServer* server);
    
    /**
     * @brief Start the WebSocket session
     * @post WebSocket handshake initiated
     * @post On success: initial data sent, read loop started
     * @post On failure: session cleans up automatically
     */
    void run();
    
    /**
     * @brief Send broadcast message to this client
     * @param message Message to broadcast
     * @post Message is queued for transmission
     * @note Thread-safe operation
     * @note Used for notifying client of booking updates
     */
    void sendBroadcastMessage(const std::string& message);

private:
    /**
     * @brief Handle WebSocket handshake completion
     * @param ec Error code from handshake operation
     * @post On success: initial data sent, read operations begin
     * @post On failure: session terminates
     */
    void on_accept(beast::error_code ec);
    
    /**
     * @brief Handle message write completion
     * @param ec Error code from write operation
     * @param bytes_transferred Number of bytes written
     * @post Next queued message sent if available
     * @post On error: session cleanup initiated
     */
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    
    /**
     * @brief Handle message read completion
     * @param ec Error code from read operation
     * @param bytes_transferred Number of bytes read
     * @post Message processed and response sent
     * @post Next read operation initiated
     * @post On error: session cleanup initiated
     */
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    
    /**
     * @brief Initiate asynchronous read operation
     * @post Read operation started for next client message
     */
    void do_read();
    
    /**
     * @brief Initiate asynchronous write operation
     * @post Next queued message is sent
     * @note Only called when no write operation is in progress
     */
    void do_write();
    
    /**
     * @brief Queue message for transmission
     * @param message Message to send
     * @post Message added to send queue
     * @post Write operation initiated if not already in progress
     */
    void send_message(const std::string& message);
};

/**
 * @class WebSocketServer
 * @brief Main WebSocket server for Cinema booking system
 * 
 * Manages multiple client connections, handles message routing, and coordinates
 * broadcast notifications. Provides the main server infrastructure for real-time
 * communication with Cinema booking clients.
 * 
 * @par Server Architecture
 * - Accepts new client connections continuously
 * - Maintains set of active sessions
 * - Routes messages to business logic via callbacks
 * - Coordinates broadcast updates to all clients
 * 
 * @par Callback System
 * Uses function callbacks to decouple server infrastructure from business logic:
 * - MessageCallback: Handle individual client messages
 * - InitialDataCallback: Get data for new clients
 * - BroadcastDataCallback: Get update data for broadcasts
 */
class WebSocketServer {
private:
    net::io_context& ioc_;                           ///< Boost.Asio I/O context
    tcp::acceptor acceptor_;                         ///< TCP acceptor for new connections
    std::set<std::shared_ptr<WebSocketSession>> sessions_; ///< Active client sessions
    MessageCallback messageCallback_;                ///< Handler for client messages
    InitialDataCallback initialDataCallback_;       ///< Provider of initial data
    BroadcastDataCallback broadcastDataCallback_;   ///< Provider of broadcast data

public:
    /**
     * @brief Constructor
     * @param ioc Boost.Asio I/O context for async operations
     * @param endpoint TCP endpoint to bind and listen on
     * @param messageCallback Function to handle client messages
     * @param initialDataCallback Function to get initial data for new clients
     * @param broadcastDataCallback Function to get broadcast update data
     * @post Server is configured but not yet accepting connections
     */
    WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint,
                   MessageCallback messageCallback,
                   InitialDataCallback initialDataCallback,
                   BroadcastDataCallback broadcastDataCallback);
    
    /**
     * @brief Start accepting client connections
     * @post Server begins accepting connections on configured endpoint
     * @post New sessions created for each client connection
     */
    void run();
    
    /**
     * @brief Add new session to active sessions set
     * @param session Shared pointer to new WebSocket session
     * @post Session is tracked and will receive broadcast messages
     * @note Called automatically when new clients connect
     */
    void addSession(std::shared_ptr<WebSocketSession> session);
    
    /**
     * @brief Remove session from active sessions set
     * @param session Shared pointer to session being removed
     * @post Session no longer receives broadcast messages
     * @note Called automatically when clients disconnect
     */
    void removeSession(std::shared_ptr<WebSocketSession> session);
    
    /**
     * @brief Send update to all connected clients
     * @post Broadcast message sent to all active sessions
     * @note Uses broadcastDataCallback_ to get current data
     * @note Called after successful booking operations
     */
    void broadcastUpdate();
    
    /**
     * @brief Handle message from client (delegates to callback)
     * @param message Message received from client
     * @param shouldBroadcast Output indicating if update should be broadcast
     * @return Response message for client
     * @post Delegates to messageCallback_ for business logic
     */
    std::string handleMessage(const std::string& message, bool& shouldBroadcast);
    
    /**
     * @brief Get initial data for new client (delegates to callback)
     * @return Initial cinema data string
     * @post Delegates to initialDataCallback_ for current data
     */
    std::string getInitialData();

private:
    /**
     * @brief Initiate asynchronous accept operation
     * @post Accept operation started for next client connection
     */
    void do_accept();
    
    /**
     * @brief Handle new client connection acceptance
     * @param ec Error code from accept operation
     * @param socket TCP socket for new client
     * @post On success: new WebSocketSession created and started
     * @post Next accept operation initiated
     */
    void on_accept(beast::error_code ec, tcp::socket socket);
};