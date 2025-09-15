#include "websocket_server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

WebSocketSession::WebSocketSession(tcp::socket&& socket, std::vector<Shows>* shows)
    : ws_(std::move(socket)), shows_(shows) {}

void WebSocketSession::run() {
    // Accept the WebSocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &WebSocketSession::on_accept,
            shared_from_this()));
}

std::string WebSocketSession::generateCinemaData() {
    std::stringstream ss;
    ss << "=== CINEMA DATA STREAM ===\n";
    
    // Group shows by theater for display
    std::vector<std::string> theaters;
    for (const auto& show : *shows_) {
        // Check if we've already seen this theater
        if (std::find(theaters.begin(), theaters.end(), show.theater) == theaters.end()) {
            theaters.push_back(show.theater);
        }
    }
    
    // Display shows grouped by theater
    for (const auto& theaterName : theaters) {
        ss << "Theater: " << theaterName << "\n";
        for (const auto& show : *shows_) {
            if (show.theater == theaterName) {
                ss << "  Movie: " << show.movie << " (" << show.dateTime << ")\n";
                ss << "    Available seats: ";
                auto availableSeats = show.Seats_Availables();
                if (availableSeats.empty()) {
                    ss << "SOLD OUT";
                } else {
                    for (size_t i = 0; i < availableSeats.size(); ++i) {
                        ss << static_cast<int>(availableSeats[i]);
                        if (i < availableSeats.size() - 1) ss << ", ";
                    }
                }
                ss << " (Total: " << availableSeats.size() << "/20)\n";
            }
        }
        ss << "\n";
    }
    
    ss << "=== END CINEMA DATA ===\n";
    return ss.str();
}

std::string WebSocketSession::handleBookingRequest(const std::string& message) {
    // Parse message format: "theater,movie,seat1,seat2,seat3..."
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        parts.push_back(item);
    }
    
    if (parts.size() < 3) {
        return "ERROR: Invalid booking format. Use: theater,movie,seat1,seat2,...";
    }
    
    std::string theaterName = parts[0];
    std::string movieName = parts[1];
    
    // Convert seat numbers from string to uint8_t
    std::vector<uint8_t> seatNumbers;
    for (size_t i = 2; i < parts.size(); ++i) {
        try {
            int seatNum = std::stoi(parts[i]);
            if (seatNum > 0 && seatNum <= 20) {
                seatNumbers.push_back(static_cast<uint8_t>(seatNum));
            } else {
                return "ERROR: Invalid seat number " + parts[i] + ". Must be 1-20.";
            }
        } catch (const std::exception& e) {
            return "ERROR: Invalid seat number format: " + parts[i];
        }
    }
    
    // Find the matching show
    for (auto& show : *shows_) {
        if (show.theater == theaterName && show.movie == movieName) {
            // Attempt to book the seats
            if (show.bookSeats(seatNumbers)) {
                std::string response = "SUCCESS: Booked seats ";
                for (size_t i = 0; i < seatNumbers.size(); ++i) {
                    response += std::to_string(seatNumbers[i]);
                    if (i < seatNumbers.size() - 1) response += ", ";
                }
                response += " for " + movieName + " at " + theaterName + "\n\n";
                response += generateCinemaData();
                return response;
            } else {
                return "ERROR: One or more seats are already booked or invalid\n\n" + generateCinemaData();
            }
        }
    }
    
    return "ERROR: Show not found - " + movieName + " at " + theaterName + "\n\n" + generateCinemaData();
}

void WebSocketSession::on_accept(beast::error_code ec) {
    if (ec) {
        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
        return;
    }

    // Send cinema data instead of "Hello World"
    std::string cinemaData = generateCinemaData();
    ws_.async_write(
        net::buffer(cinemaData),
        beast::bind_front_handler(
            &WebSocketSession::on_write,
            shared_from_this()));
}

void WebSocketSession::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec) {
        std::cerr << "WebSocket write error: " << ec.message() << std::endl;
        return;
    }

    // Read a message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketSession::on_read,
            shared_from_this()));
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec == websocket::error::closed) {
        return;
    }

    if (ec) {
        std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        return;
    }

    // Get the received message
    std::string received = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());
    
    std::cout << "WebSocket received: " << received << std::endl;
    
    std::string response;
    if (received == "get_data" || received == "refresh") {
        // Send fresh cinema data
        response = generateCinemaData();
    } else if (received.find(',') != std::string::npos) {
        // Handle booking request: theater,movie,seat1,seat2,seat3...
        response = handleBookingRequest(received);
    } else {
        // Echo the message back with cinema data
        response = "Echo: " + received + "\n\n" + generateCinemaData();
    }
    
    // Send response
    ws_.async_write(
        net::buffer(response),
        beast::bind_front_handler(
            &WebSocketSession::on_write,
            shared_from_this()));
}

WebSocketServer::WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint, std::vector<Shows>* shows)
    : ioc_(ioc), acceptor_(ioc), shows_(shows) {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Failed to open acceptor: " << ec.message() << std::endl;
        return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        std::cerr << "Failed to set reuse_address: " << ec.message() << std::endl;
        return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Failed to bind: " << ec.message() << std::endl;
        return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Failed to listen: " << ec.message() << std::endl;
        return;
    }
}

void WebSocketServer::run() {
    do_accept();
}

void WebSocketServer::do_accept() {
    acceptor_.async_accept(
        beast::bind_front_handler(
            &WebSocketServer::on_accept,
            this));
}

void WebSocketServer::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    } else {
        std::make_shared<WebSocketSession>(std::move(socket), shows_)->run();
    }

    // Accept another connection
    do_accept();
}