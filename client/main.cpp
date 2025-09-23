/**
 * @file main.cpp
 * @brief Cinema Client Application Entry Point
 * @author Jorge Royon
 * @date 2025-09-18
 * @version 1.0
 * 
 * @brief Main application for Cinema booking system client
 * 
 * This application provides the main entry point for the Cinema booking client.
 * It integrates the WebSocket communication layer (CinemaClient) with the user
 * interface (CinemaUI) to provide a complete booking experience.
 * 
 * @par Application Flow
 * 1. Initialize client and UI components
 * 2. Connect to Cinema server via WebSocket
 * 3. Fetch initial cinema data from server
 * 4. Present interactive menu to user
 * 5. Handle user selections (view movies, book seats, help)
 * 6. Process booking requests with server communication
 * 7. Display booking results and confirmations
 * 8. Graceful shutdown on exit
 * 
 * @par Key Features
 * - Real-time server communication
 * - Interactive booking interface
 * - Error handling and connection management
 * - Booking confirmation system
 * - Help and guidance for users
 * 
 * @par Architecture Role
 * Serves as the business logic coordinator in the client application,
 * orchestrating communication between UI and WebSocket layers.
 */

#include "lib/websocket_client.hpp"
#include "lib/CinemaUI.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

/**
 * @brief Main application entry point
 * @return 0 on successful execution, 1 on error
 * 
 * @par Execution Flow
 * 1. Creates CinemaClient and CinemaUI instances
 * 2. Establishes WebSocket connection to server
 * 3. Enters main interaction loop
 * 4. Handles user menu selections and booking operations
 * 5. Manages server communication and response processing
 * 
 * @par Error Handling
 * - Connection failures result in graceful exit with error message
 * - Server communication errors are handled with appropriate user feedback
 * - Exception handling provides error reporting
 * 
 * @par User Interaction
 * - Menu option 1: View current movies and showtimes
 * - Menu option 2: Book seats for available shows
 * - Menu option 3: Display booking help and instructions
 * - Menu option 4: Exit application gracefully
 */

int main() {
    try {
        CinemaClient client;
        CinemaUI ui;
        
        ui.displayWelcome();
        
        // Get server host and port from environment variables (for Docker)
        // Default to localhost:8080 for local development
        const char* server_host = std::getenv("SERVER_HOST");
        const char* server_port = std::getenv("SERVER_PORT");
        
        std::string host = server_host ? server_host : "localhost";
        std::string port = server_port ? server_port : "8080";
        
        std::cout << "Starting Cinema Client..." << std::endl;
        std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
        
        if (!client.connect(host, port)) {
            std::cout << "\nCannot connect to server at " << host << ":" << port << std::endl;
            std::cout << "Make sure the cinema server is running and try again." << std::endl;
            return 1;
        }
        
        std::cout << "Fetching initial cinema data..." << std::endl;
        client.sendMessage("get_data");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        while (client.isConnected()) {
            int choice = ui.showMainMenu();
            
            switch (choice) {
                case 1: {
                    std::cout << "Fetching current cinema data..." << std::endl;
                    client.sendMessage("get_data");
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    std::string cinemaData = client.getLastResponse();
                    ui.viewMovies(cinemaData);
                    break;
                }
                case 2: {
                    std::cout << "Getting current cinema data..." << std::endl;
                    client.sendMessage("get_data");
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    
                    std::vector<Shows> shows = client.getShows();

                    if (shows.empty()) {
                        std::cout << "No shows available for booking. Please try again." << std::endl;
                        break;
                    }
                    
                    
                    std::string bookingData = ui.performBooking(shows);
                    
                    if (!bookingData.empty()) {
                        std::cout << "Processing your booking..." << std::endl;
                        client.sendMessage(bookingData);
                        std::this_thread::sleep_for(std::chrono::milliseconds(300));
                        
                        std::string serverResponse = client.getLastBookingResponse();

                        bool success = false;
                        
                        if (serverResponse.find("SUCCESS:") != std::string::npos) {
                            success = true;
                        } else if (serverResponse.find("ERROR:") != std::string::npos) {
                            success = false;
                        } 
                        
                        ui.showBookingResult(success);
                        ui.waitForEnter();
                    }
                    break;
                }
                case 3:
                    ui.showBookingHelp();
                    break;
                case 4:
                    ui.displayGoodbye();
                    client.disconnect();
                    return 0; 
                default:
                    std::cout << "Invalid option! Please choose 1-4." << std::endl;
                    break;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}