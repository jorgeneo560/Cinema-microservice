/**
 * @file main.cpp
 * @brief Cinema Server Application Entry Point
 * @author Jorge Royon
 * @date 2025-09-18
 * @version 1.0
 * 
 * @brief Main application for Cinema booking system server
 * 
 * This application provides the main entry point for the Cinema booking server.
 * It initializes the cinema data, sets up the WebSocket server infrastructure,
 * and manages real-time communication with multiple cinema booking clients.
 * 
 * @par Application Architecture
 * - Creates initial cinema data with predefined shows and seat patterns
 * - Configures WebSocket server with callback handlers
 * - Manages concurrent client connections
 * - Coordinates booking operations and real-time updates
 * 
 * @par Data Initialization
 * Creates 9 shows (3 movies × 3 theaters) with different seat availability patterns:
 * - Movies: Inception, Interstellar, Tenet
 * - Theaters: PVR, IMAX, Cinepolis
 * - Each show has 20 seats with predefined booking patterns
 * 
 * @par Server Features
 * - Multi-client WebSocket communication
 * - Real-time booking updates
 * - Thread-safe seat management
 * - Broadcast notifications for booking changes
 * - Comprehensive error handling
 * 
 * @par Architecture Role
 * Serves as the main business logic coordinator, integrating:
 * - Cinema data management (Shows)
 * - WebSocket communication (WebSocketServer)
 * - Business services (CinemaService, BookingService, MessageHandler)
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>
#include <boost/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include "lib/cinema.hpp"
#include "lib/websocket_server.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;

/**
 * @brief Main server application entry point
 * @return 0 on successful execution
 * 
 * @par Initialization Process
 * 1. Create cinema data with movies, theaters, and seat patterns
 * 2. Configure WebSocket server with callback handlers
 * 3. Start server on localhost:8080
 * 4. Display initial cinema state
 * 5. Enter serving loop for client connections
 * 
 * @par Callback Configuration
 * - Message Callback: Routes client messages to MessageHandler
 * - Initial Data Callback: Provides current cinema state to new clients
 * - Broadcast Callback: Supplies update data for booking notifications
 * 
 * @par Threading Model
 * - Main thread: Handles initialization and display
 * - WebSocket thread: Manages async I/O operations
 * - Server supports multiple concurrent client connections
 * 
 * @par Data Patterns
 * Uses three predefined seat patterns to simulate different booking states:
 * - Pattern 1: Mixed availability (50% occupied)
 * - Pattern 2: Sparse availability (45% occupied)  
 * - Pattern 3: High availability (25% occupied)
 */

int main() {

	std::vector<std::string> movieNames = {"Inception", "Interstellar", "Tenet"};
	std::vector<std::string> theaterNames = {"PVR", "IMAX", "Cinepolis"};

	std::vector<Shows> shows;
	
	std::vector<bool> pattern1 = {
		true, true, false, false, true,
		false, false, false, false, true,
		true, false, false, true, true,
		false, false, false, true, false
	};
	
	std::vector<bool> pattern2 = {
		false, true, false, true, false,
		true, false, true, true, false,
		false, true, true, false, false,
		true, true, false, false, true
	};
	
	std::vector<bool> pattern3 = {
		true, false, true, true, true,
		false, true, true, false, true,
		true, false, true, true, true,
		false, false, true, true, true
	};
	
	std::vector<std::vector<bool>> patterns = {pattern1, pattern2, pattern3};
	
	int patternIndex = 0;
	for (const auto& tName : theaterNames) {
		for (const auto& mName : movieNames) {
			Shows show(mName, "2025-09-11 19:30", tName);
			show.seats = patterns[patternIndex % patterns.size()];
			shows.push_back(show);
			patternIndex++;
		}
	}

	net::io_context ioc;
	auto const address = net::ip::make_address("0.0.0.0");
	auto const port = static_cast<unsigned short>(8080);
	
	std::cout << "Starting WebSocket server on " << address << ":" << port << std::endl;
	
	auto messageCallback = [&shows](const std::string& message, bool& shouldBroadcast) -> std::string {
		return MessageHandler::handleMessage(message, shows, shouldBroadcast);
	};
	
	auto initialDataCallback = [&shows]() -> std::string {
		return CinemaService::formatCinemaData(shows);
	};
	
	auto broadcastDataCallback = [&shows]() -> std::string {
		return CinemaService::formatUpdateData(shows);
	};
	
	WebSocketServer server(ioc, tcp::endpoint{address, port}, 
	                      messageCallback, initialDataCallback, broadcastDataCallback);
	server.run();
	
	std::thread websocket_thread([&ioc]() {
		ioc.run();
	});
	
	std::cout << "WebSocket server started! Connect to ws://localhost:8080" << std::endl << std::endl;

	for (const auto& theaterName : theaterNames) {
		std::cout << "Theater: " << theaterName << "\n";
		for (const auto& show : shows) {
			if (show.theater == theaterName) {
				std::cout << "  Movie: " << show.movie << "\n";
				std::cout << "    Free seats: ";
				for (auto seatNum : show.getAvailableSeats()) {
					std::cout << static_cast<int>(seatNum) << " ";
				}
				std::cout << "\n";
			}
		}
		std::cout << "\n";
	}
	
	std::cout << "Cinema data displayed. WebSocket server is running..." << std::endl;
	std::cout << "Press Ctrl+C to stop the server." << std::endl;
	
	websocket_thread.join();
	
	return 0;
}
