#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <thread>
#include <boost/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include "cinema.hpp"
#include "websocket_server.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
	// Print Boost version
	std::cout << "Using Boost version: " << BOOST_VERSION / 100000 << "."
			  << BOOST_VERSION / 100 % 1000 << "."
			  << BOOST_VERSION % 100 << std::endl << std::endl;

	// Hardcoded theaters and movies with random seat availability
	std::vector<std::string> movieNames = {"Inception", "Interstellar", "Tenet"};
	std::vector<std::string> theaterNames = {"PVR", "IMAX", "Cinepolis"};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::bernoulli_distribution seatDist(0.5); // 50% chance seat is free

	// Generate shows with random seat availability
	std::vector<Shows> shows;
	for (const auto& tName : theaterNames) {
		for (const auto& mName : movieNames) {
			Shows show(mName, "2025-09-11 19:30", tName);
			for (size_t i = 0; i < show.seats.size(); ++i) {
				show.seats[i] = !seatDist(gen); // true = occupied, false = free
			}
			shows.push_back(show);
		}
	}

	// Start WebSocket server in a separate thread
	net::io_context ioc;
	auto const address = net::ip::make_address("0.0.0.0");
	auto const port = static_cast<unsigned short>(8080);
	
	std::cout << "Starting WebSocket server on " << address << ":" << port << std::endl;
	
	WebSocketServer server(ioc, tcp::endpoint{address, port}, &shows);
	server.run();
	
	// Run the WebSocket server in a separate thread
	std::thread websocket_thread([&ioc]() {
		ioc.run();
	});
	
	std::cout << "WebSocket server started! Connect to ws://localhost:8080" << std::endl << std::endl;

	// Group shows by theater and display
	for (const auto& theaterName : theaterNames) {
		std::cout << "Theater: " << theaterName << "\n";
		for (const auto& show : shows) {
			if (show.theater == theaterName) {
				std::cout << "  Movie: " << show.movie << "\n";
				std::cout << "    Free seats: ";
				for (auto seatNum : show.Seats_Availables()) {
					std::cout << static_cast<int>(seatNum) << " ";
				}
				std::cout << "\n";
			}
		}
		std::cout << "\n";
	}
	
	std::cout << "Cinema data displayed. WebSocket server is running..." << std::endl;
	std::cout << "Press Ctrl+C to stop the server." << std::endl;
	
	// Keep the main thread alive while WebSocket server runs
	websocket_thread.join();
	
	return 0;
}
