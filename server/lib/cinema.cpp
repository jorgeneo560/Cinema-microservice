#include "cinema.hpp"
#include <thread>
#include <chrono>
#include <iostream>

Shows::Shows(const std::string& m, const std::string& dt, const std::string& t) 
    : movie(m), dateTime(dt), theater(t), seatsMutex(std::make_shared<std::shared_mutex>()) {
    seats.resize(20, false);
}

std::vector<uint8_t> Shows::getAvailableSeats() const {
    std::shared_lock<std::shared_mutex> lock(*seatsMutex);
    std::vector<uint8_t> availableSeats;
    for (int i = 0; i < seats.size(); ++i) {
        if (!seats[i]) {
            availableSeats.push_back(static_cast<uint8_t>(i + 1));
        }
    }
    return availableSeats;
}

bool Shows::bookSeats(const std::vector<uint8_t>& seatNumbers) {
    std::unique_lock<std::shared_mutex> lock(*seatsMutex);

    for (uint8_t seat : seatNumbers) {
        if (seat < 1 || seat > seats.size() || seats[seat - 1]) {
            return false;
        }
    }
    for (uint8_t seat : seatNumbers) {
        seats[seat - 1] = true;
    }
    return true;
}

std::string CinemaService::formatCinemaData(const std::vector<Shows>& shows) {
    std::stringstream ss;
    ss << "=== CINEMA DATA STREAM ===\n";
    
    std::vector<std::string> theaters;
    for (const auto& show : shows) {
        if (std::find(theaters.begin(), theaters.end(), show.theater) == theaters.end()) {
            theaters.push_back(show.theater);
        }
    }
    
    for (const auto& theaterName : theaters) {
        ss << "Theater: " << theaterName << "\n";
        for (const auto& show : shows) {
            if (show.theater == theaterName) {
                ss << "  Movie: " << show.movie << " (" << show.dateTime << ")\n";
                ss << "    Available seats: ";
                auto availableSeats = show.getAvailableSeats();
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

std::string CinemaService::formatUpdateData(const std::vector<Shows>& shows) {
    std::stringstream ss;
    ss << "BOOKING_UPDATE:\n=== UPDATED CINEMA DATA ===\n";
    
    // Group shows by theater for display
    std::vector<std::string> theaters;
    for (const auto& show : shows) {
        if (std::find(theaters.begin(), theaters.end(), show.theater) == theaters.end()) {
            theaters.push_back(show.theater);
        }
    }
    
    for (const auto& theaterName : theaters) {
        ss << "Theater: " << theaterName << "\n";
        for (const auto& show : shows) {
            if (show.theater == theaterName) {
                ss << "  Movie: " << show.movie << " (" << show.dateTime << ")\n";
                ss << "    Available seats: ";
                auto availableSeats = show.getAvailableSeats();
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
    
    ss << "=== END UPDATED DATA ===\n";
    return ss.str();
}

BookingService::BookingResult BookingService::processBooking(const std::string& message, std::vector<Shows>& shows) {
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        parts.push_back(item);
    }
    
    if (parts.size() < 3) {
        return {false, "ERROR: Invalid booking format. Use: theater,movie,seat1,seat2,...\n\n" + CinemaService::formatCinemaData(shows), false};
    }
    
    std::string theaterName = parts[0];
    std::string movieName = parts[1];
    
    std::vector<uint8_t> seatNumbers;
    for (size_t i = 2; i < parts.size(); ++i) {
        try {
            int seatNum = std::stoi(parts[i]);
            if (seatNum > 0 && seatNum <= 20) {
                seatNumbers.push_back(static_cast<uint8_t>(seatNum));
            } else {
                return {false, "ERROR: Invalid seat number " + parts[i] + ". Must be 1-20.\n\n" + CinemaService::formatCinemaData(shows), false};
            }
        } catch (const std::exception& e) {
            return {false, "ERROR: Invalid seat number format: " + parts[i] + "\n\n" + CinemaService::formatCinemaData(shows), false};
        }
    }
    
    for (auto& show : shows) {
        if (show.theater == theaterName && show.movie == movieName) {
            if (show.bookSeats(seatNumbers)) {
                std::string response = "SUCCESS: Booked seats ";
                for (size_t i = 0; i < seatNumbers.size(); ++i) {
                    response += std::to_string(seatNumbers[i]);
                    if (i < seatNumbers.size() - 1) response += ", ";
                }
                response += " for " + movieName + " at " + theaterName + "\n\n";
                response += CinemaService::formatCinemaData(shows);
                return {true, response, true};
            } else {
                return {false, "ERROR: One or more seats are already booked or invalid\n\n" + CinemaService::formatCinemaData(shows), false};
            }
        }
    }
    
    return {false, "ERROR: Show not found - " + movieName + " at " + theaterName + "\n\n" + CinemaService::formatCinemaData(shows), false};
}

std::string MessageHandler::handleMessage(const std::string& received, std::vector<Shows>& shows, bool& shouldBroadcast) {
    shouldBroadcast = false;
    
    if (received == "get_data" || received == "refresh") {
        return CinemaService::formatCinemaData(shows);
    } else if (received.find(',') != std::string::npos) {
        auto result = BookingService::processBooking(received, shows);
        shouldBroadcast = result.shouldBroadcast;
        return result.message;
    } else {
        return "Echo: " + received + "\n\n" + CinemaService::formatCinemaData(shows);
    }
}