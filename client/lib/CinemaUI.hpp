#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include "cinema_Client.hpp"

/**
 * @brief UI constants for consistent formatting and layout
 */
namespace UIConstants {
    constexpr int WIDE_SEPARATOR_WIDTH = 60;
    constexpr int MEDIUM_SEPARATOR_WIDTH = 50;
    constexpr int NARROW_SEPARATOR_WIDTH = 40;
    constexpr const char* CLEAR_SCREEN = "\033[2J\033[1;1H";
    constexpr int CINEMA_ROWS = 4;
    constexpr int SEATS_PER_ROW = 5;
    constexpr int MAX_SEAT_NUMBER = 20;
    constexpr int MIN_SEAT_NUMBER = 1;
}

/**
 * @brief User interface class for Cinema booking system
 * 
 * Pure UI class that handles display and user input collection only.
 * All data is passed via parameters, no direct client communication.
 */
class CinemaUI {
public:
    /**
     * @brief Constructor
     */
    CinemaUI();

    /**
     * @brief Display welcome message
     */
    void displayWelcome();

    /**
     * @brief Display main menu and get user choice
     * @return User's menu selection (1-4)
     */
    int showMainMenu();

    /**
     * @brief View all movies and theaters
     * @param cinemaData The complete cinema data string from server
     */
    void viewMovies(const std::string& cinemaData);

    /**
     * @brief Perform complete booking process
     * @param shows Vector of available shows
     * @return Booking data string if successful, empty string if cancelled
     */
    std::string performBooking(const std::vector<Shows>& shows);

    /**
     * @brief Show booking help
     */
    void showBookingHelp();

    /**
     * @brief Display booking result to user
     * @param success Whether the booking was successful
     */
    void showBookingResult(bool success);

    /**
     * @brief Display goodbye message
     */
    void displayGoodbye();

    /**
     * @brief Wait for user to press Enter
     */
    void waitForEnter();

private:
    /**
     * @brief Select a theater
     * @param shows Vector of available shows
     * @return Selected theater name, empty if cancelled
     */
    std::string selectTheater(const std::vector<Shows>& shows);

    /**
     * @brief Select a show for the given theater
     * @param theater Theater name
     * @param shows Vector of available shows for this theater
     * @return Selected show, or Shows with empty fields if cancelled
     */
    Shows selectShow(const std::string& theater, const std::vector<Shows>& shows);

    /**
     * @brief Select seats for the given show
     * @param selectedShow The selected show
     * @return Vector of selected seat numbers, empty if cancelled
     */
    std::vector<uint8_t> selectSeats(const Shows& selectedShow);

    /**
     * @brief Confirm booking details
     * @param selectedShow The selected show
     * @param seats Vector of seat numbers
     * @return true if user confirmed booking, false if cancelled
     */
    bool confirmBooking(const Shows& selectedShow, const std::vector<uint8_t>& seats);

    /**
     * @brief Display information for a specific show
     * @param selectedShow The selected show
     */
    void displaySpecificShow(const Shows& selectedShow);

    /**
     * @brief Show visual seat layout
     * @param availableSeats Vector of available seat numbers
     */
    void showSeatLayout(const std::vector<uint8_t>& availableSeats);

    /**
     * @brief Get user input with validation
     * @param prompt Prompt to display
     * @return User input as integer, -1 if invalid
     */
    int getIntInput(const std::string& prompt);
};