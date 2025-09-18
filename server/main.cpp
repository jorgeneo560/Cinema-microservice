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
