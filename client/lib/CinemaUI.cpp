#include "CinemaUI.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

CinemaUI::CinemaUI() {
}

void CinemaUI::displayWelcome() {
    std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << "CINEMA BOOKING CLIENT" << std::endl;
    std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
}

int CinemaUI::showMainMenu() {
    std::cout << UIConstants::CLEAR_SCREEN;
    
    std::cout << "\n" << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << "CINEMA BOOKING SYSTEM" << std::endl;
    std::cout << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << "1. View all movies and theaters" << std::endl;
    std::cout << "2. Book seats (Step-by-step)" << std::endl;
    std::cout << "3. Show booking help" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    
    return getIntInput("Choose option (1-4): ");
}

void CinemaUI::viewMovies(const std::string& cinemaData) {
    std::cout << UIConstants::CLEAR_SCREEN;
    
    if (!cinemaData.empty()) {
        std::cout << "\n" << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
        std::cout << "CINEMA SHOWTIMES AND AVAILABILITY" << std::endl;
        std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
        std::cout << cinemaData << std::endl;
        std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
    } else {
        std::cout << "No cinema data available." << std::endl;
    }
    
    waitForEnter();
}

std::string CinemaUI::performBooking(const std::vector<Shows>& shows) {
    std::cout << "\nBOOKING PROCESS" << std::endl;
    std::cout << std::string(UIConstants::NARROW_SEPARATOR_WIDTH, '=') << std::endl;
    
    if (shows.empty()) {
        std::cout << "No shows available for booking." << std::endl;
        return "";
    }
    
    std::string selectedTheater = selectTheater(shows);
    if (selectedTheater.empty()) {
        std::cout << "Booking cancelled." << std::endl;
        return "";
    }
    
    Shows selectedShow = selectShow(selectedTheater, shows);
    if (selectedShow.theater.empty()) {  
        std::cout << "Booking cancelled." << std::endl;
        return "";
    }
    
    std::vector<uint8_t> selectedSeats = selectSeats(selectedShow);
    if (selectedSeats.empty()) {
        std::cout << "Booking cancelled." << std::endl;
        return "";
    }
    
    
    if (!confirmBooking(selectedShow, selectedSeats)) {
        std::cout << "Booking cancelled." << std::endl;
        return "";
    }
    
    
    std::string booking = selectedShow.theater + "," + selectedShow.movie;
    for (uint8_t seat : selectedSeats) {
        booking += "," + std::to_string(seat);
    }
    
    return booking;
}

std::string CinemaUI::selectTheater(const std::vector<Shows>& shows) {
    std::cout << UIConstants::CLEAR_SCREEN;
    
    std::vector<std::string> theaters;
    for (const auto& show : shows) {
        if (std::find(theaters.begin(), theaters.end(), show.theater) == theaters.end()) {
            theaters.push_back(show.theater);
        }
    }
    
    std::cout << "\nAVAILABLE THEATERS:" << std::endl;
    
    for (size_t i = 0; i < theaters.size(); ++i) {
        std::cout << (i + 1) << ". " << theaters[i] << std::endl;
    }
    std::cout << "0. Cancel booking" << std::endl;
    
    int choice = getIntInput("\nSelect theater (0-" + std::to_string(theaters.size()) + "): ");
    
    if (choice == 0) {
        return "";
    } else if (choice >= 1 && choice <= static_cast<int>(theaters.size())) {
        return theaters[choice - 1];
    } else {
        std::cout << "Invalid selection!" << std::endl;
        return "";
    }
}

Shows CinemaUI::selectShow(const std::string& theater, const std::vector<Shows>& shows) {
    std::cout << UIConstants::CLEAR_SCREEN;
    
    std::vector<Shows> theaterShows;
    for (const auto& show : shows) {
        if (show.theater == theater) {
            theaterShows.push_back(show);
        }
    }
    
    std::cout << "\nAVAILABLE SHOWS AT " << theater << ":" << std::endl;
    
    for (size_t i = 0; i < theaterShows.size(); ++i) {
        std::cout << (i + 1) << ". " << theaterShows[i].movie 
                  << " (" << theaterShows[i].dateTime << ")" << std::endl;
    }
    std::cout << "0. Cancel booking" << std::endl;
    
    int choice = getIntInput("\nSelect show (0-" + std::to_string(theaterShows.size()) + "): ");
    
    if (choice == 0) {
        return Shows("", "", "");  
    } else if (choice >= 1 && choice <= static_cast<int>(theaterShows.size())) {
        return theaterShows[choice - 1];
    } else {
        std::cout << "Invalid selection!" << std::endl;
        return Shows("", "", ""); 
    }
}

std::vector<uint8_t> CinemaUI::selectSeats(const Shows& selectedShow) {
    std::cout << UIConstants::CLEAR_SCREEN;
    std::cout << "\nSEAT SELECTION FOR " << selectedShow.movie << " AT " << selectedShow.theater << std::endl;
    std::cout << "Show time: " << selectedShow.dateTime << std::endl;
    
    displaySpecificShow(selectedShow);
    
    std::cout << "\nAvailable seats are numbered " << UIConstants::MIN_SEAT_NUMBER << "-" << UIConstants::MAX_SEAT_NUMBER << std::endl;
    std::cout << "Enter seat numbers separated by spaces OR commas (e.g., 1 2 3 4 OR 1,2,3,4)" << std::endl;
    std::cout << "Enter 0 to cancel booking" << std::endl;
    std::cout << "\nSelect your seats: ";
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty() || input == "0") {
        return {};
    }
    
    std::vector<uint8_t> seats;
    
    std::string normalizedInput = input;
    std::replace(normalizedInput.begin(), normalizedInput.end(), ',', ' ');
    
    std::stringstream ss(normalizedInput);
    std::string seatStr;
    
    while (ss >> seatStr) {
        try {
            int seat = std::stoi(seatStr);
            if (seat >= UIConstants::MIN_SEAT_NUMBER && seat <= UIConstants::MAX_SEAT_NUMBER) {
                seats.push_back(static_cast<uint8_t>(seat));
            } else {
                std::cout << "Invalid seat number: " << seat << " (must be " << UIConstants::MIN_SEAT_NUMBER << "-" << UIConstants::MAX_SEAT_NUMBER << ")" << std::endl;
                return {};
            }
        } catch (const std::exception& e) {
            std::cout << "Invalid seat format: " << seatStr << std::endl;
            return {};
        }
    }
    
    if (seats.empty()) {
        std::cout << "No valid seats selected!" << std::endl;
        return {};
    }
    
    std::vector<uint8_t> availableSeats = selectedShow.getAvailableSeats();
    for (uint8_t seat : seats) {
        if (std::find(availableSeats.begin(), availableSeats.end(), seat) == availableSeats.end()) {
            std::cout << "Seat " << static_cast<int>(seat) << " is not available!" << std::endl;
            return {};
        }
    }
    
    return seats;
}

bool CinemaUI::confirmBooking(const Shows& selectedShow, const std::vector<uint8_t>& seats) {
    std::cout << "\nBOOKING CONFIRMATION" << std::endl;
    std::cout << std::string(UIConstants::NARROW_SEPARATOR_WIDTH, '-') << std::endl;
    std::cout << "Theater: " << selectedShow.theater << std::endl;
    std::cout << "Movie: " << selectedShow.movie << std::endl;
    std::cout << "Show time: " << selectedShow.dateTime << std::endl;
    std::cout << "Seats: ";
    for (size_t i = 0; i < seats.size(); ++i) {
        std::cout << static_cast<int>(seats[i]);
        if (i < seats.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << std::string(UIConstants::NARROW_SEPARATOR_WIDTH, '-') << std::endl;
    std::cout << "Confirm booking? (y/n): ";
    
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        return true;
    } else {
        return false;
    }
}

void CinemaUI::displaySpecificShow(const Shows& selectedShow) {
    std::cout << "\n" << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << selectedShow.movie << " at " << selectedShow.theater << " (" << selectedShow.dateTime << ")" << std::endl;
    std::cout << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    
    std::vector<uint8_t> availableSeats = selectedShow.getAvailableSeats();
    
    std::cout << "Available seats: ";
    if (availableSeats.empty()) {
        std::cout << "SOLD OUT";
    } else {
        for (size_t i = 0; i < availableSeats.size(); ++i) {
            std::cout << static_cast<int>(availableSeats[i]);
            if (i < availableSeats.size() - 1) std::cout << ", ";
        }
        std::cout << " (Total: " << availableSeats.size() << ")";
    }
    std::cout << std::endl;
    
    showSeatLayout(availableSeats);
    
    std::cout << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
}

void CinemaUI::showSeatLayout(const std::vector<uint8_t>& availableSeats) {
    std::cout << "\nSCREEN" << std::endl;
    std::cout << "Seat Layout:" << std::endl;
    
    for (int row = 1; row <= UIConstants::CINEMA_ROWS; ++row) {
        std::cout << "Row " << row << ": ";
        for (int seat = 1; seat <= UIConstants::SEATS_PER_ROW; ++seat) {
            int seatNumber = (row - 1) * UIConstants::SEATS_PER_ROW + seat;
            bool isAvailable = std::find(availableSeats.begin(), availableSeats.end(), static_cast<uint8_t>(seatNumber)) != availableSeats.end();
            
            if (seatNumber < 10) {
                std::cout << "[" << (isAvailable ? std::to_string(seatNumber) : "X") << " ] ";
            } else {
                std::cout << "[" << (isAvailable ? std::to_string(seatNumber) : "X") << "] ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "\nAvailable = Available  |  X = Occupied" << std::endl;
}

void CinemaUI::showBookingHelp() {
    std::cout << "\n" << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << "STEP-BY-STEP BOOKING PROCESS" << std::endl;
    std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
    std::cout << "Our new booking process is easy and guided:" << std::endl;
    std::cout << "\nBooking Steps:" << std::endl;
    std::cout << "  1. Select a theater" << std::endl;
    std::cout << "  2. Choose a movie" << std::endl;
    std::cout << "  3. View available seats for your selection" << std::endl;
    std::cout << "  4. Pick your seats (1-20, multiple seats allowed)" << std::endl;
    std::cout << "  5. Confirm your booking" << std::endl;
    std::cout << "\nTips:" << std::endl;
    std::cout << "  • Enter 0 at any step to cancel" << std::endl;
    std::cout << "  • You can select multiple seats (e.g., 1 2 3 4)" << std::endl;
    std::cout << "  • All clients will see real-time updates" << std::endl;
    std::cout << "  • Seat availability is checked in real-time" << std::endl;
    std::cout << std::string(UIConstants::WIDE_SEPARATOR_WIDTH, '=') << std::endl;
    
    waitForEnter();
}

void CinemaUI::showBookingResult(bool success) {
    std::cout << "\n" << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
    if (success) {
        std::cout << "BOOKING SUCCESSFUL!" << std::endl;
        std::cout << "Your seats have been reserved successfully." << std::endl;
        std::cout << "You will receive a confirmation shortly." << std::endl;
    } else {
        std::cout << "BOOKING FAILED!" << std::endl;
        std::cout << "There was an issue processing your booking." << std::endl;
        std::cout << "Please try again or contact support." << std::endl;
    }
    std::cout << std::string(UIConstants::MEDIUM_SEPARATOR_WIDTH, '=') << std::endl;
}

void CinemaUI::displayGoodbye() {
    std::cout << "Thank you for using Cinema Booking System!" << std::endl;
}

int CinemaUI::getIntInput(const std::string& prompt) {
    std::cout << prompt;
    int choice;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input! Please enter a number." << std::endl;
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

void CinemaUI::waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
    std::cin.get(); 
}