#include "lib/websocket_client.hpp"
#include "lib/CinemaUI.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        CinemaClient client;
        CinemaUI ui;
        
        ui.displayWelcome();
        
        std::cout << "Starting Cinema Client..." << std::endl;
        if (!client.connect("localhost", "8080")) {
            std::cout << "\nCannot connect to server at localhost:8080" << std::endl;
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