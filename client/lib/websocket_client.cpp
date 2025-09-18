#include "websocket_client.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>

CinemaClient::CinemaClient() 
    : ws_(ioc_) {
}

CinemaClient::~CinemaClient() {
    stopMessageListener();
    disconnect();
}

bool CinemaClient::connect(const std::string& host, const std::string& port) {
    if (host.empty() || port.empty()) {
        std::cerr << "Invalid host or port: host and port cannot be empty" << std::endl;
        return false;
    }
    
    if (connected_) {
        std::cerr << "Already connected to server" << std::endl;
        return true;
    }
    
    try {
        tcp::resolver resolver{ioc_};
        auto const results = resolver.resolve(host, port);
        
        auto ep = net::connect(ws_.next_layer(), results);
        std::string target = "/";
        ws_.handshake(host + ":" + std::to_string(ep.port()), target);
        
        connected_ = true;
        std::cout << "Connected to Cinema Server at " << host << ":" << port << "!\n" << std::endl;
        
        startMessageListener();
        
        return true;
    } catch (std::exception const& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        std::cerr << "Make sure the cinema server is running on " << host << ":" << port << std::endl;
        return false;
    }
}

void CinemaClient::disconnect() {
    if (connected_) {
        connected_ = false;
        shouldStop_ = true;
        
        try {
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            
            if (ws_.is_open()) {
                ws_.close(websocket::close_code::normal);
            }
            std::cout << "Disconnected from server." << std::endl;
        } catch (std::exception const& e) {
            std::string errorMsg = e.what();
            if (errorMsg.find("gracefully closed") == std::string::npos &&
                errorMsg.find("Operation canceled") == std::string::npos) {
                std::cerr << "Error during disconnect: " << e.what() << std::endl;
            }
        }
    }
}

bool CinemaClient::isConnected() const {
    return connected_;
}

void CinemaClient::sendMessage(const std::string& message) {
    if (!connected_) {
        std::cerr << "Not connected to server!" << std::endl;
        return;
    }
    
    try {
        ws_.write(net::buffer(message));
    } catch (std::exception const& e) {
        std::cerr << "Failed to send message: " << e.what() << std::endl;
        connected_ = false;
    }
}

std::string CinemaClient::getLastResponse() const {
    std::lock_guard<std::mutex> lock(responseMutex_);
    return lastResponse_;
}

std::string CinemaClient::getLastBookingResponse() const {
    std::lock_guard<std::mutex> lock(responseMutex_);
    return lastBookingResponse_;
}


void CinemaClient::startMessageListener() {
    shouldStop_ = false;
    listenerThread_ = std::make_unique<std::thread>([this]() {
        while (connected_ && !shouldStop_) {
            try {
                beast::flat_buffer buffer;
                ws_.read(buffer);
                std::string response = beast::buffers_to_string(buffer.data());
                
                
                {
                    std::lock_guard<std::mutex> lock(responseMutex_);
                    lastResponse_ = response;
                }
                
                
                processMessage(response);
                
            } catch (std::exception const& e) {
                if (connected_ && !shouldStop_) {
                    std::string errorMsg = e.what();
                    if (errorMsg.find("gracefully closed") == std::string::npos &&
                        errorMsg.find("Operation canceled") == std::string::npos) {
                        std::cerr << "Connection lost: " << e.what() << std::endl;
                    }
                    connected_ = false;
                }
                break;
            }
        }
    });
}

void CinemaClient::stopMessageListener() {
    shouldStop_ = true;
    if (listenerThread_ && listenerThread_->joinable()) {
        listenerThread_->join();
        listenerThread_.reset();
    }
}

void CinemaClient::processMessage(const std::string& response) {
    
    if (response.find("SUCCESS:") != std::string::npos || response.find("ERROR:") != std::string::npos) {
        {
            std::lock_guard<std::mutex> lock(responseMutex_);
            lastBookingResponse_ = response;
        }
        
        if (isCinemaDataStream(response)) {
             parseAndUpdateShows(response);
        }
        return;
    }
    
    if (isCinemaDataStream(response)) {
        parseAndUpdateShows(response);
        
        if (response.find(CinemaProtocol::BOOKING_UPDATE) != std::string::npos) {
            handleBookingUpdate(response);
        }
        return;
    }
    
    handleServerMessage(response);
}

bool CinemaClient::isCinemaDataStream(const std::string& response) const {
    return response.find(CinemaProtocol::CINEMA_DATA_STREAM) != std::string::npos ||
           response.find(CinemaProtocol::UPDATED_CINEMA_DATA) != std::string::npos;
}

void CinemaClient::handleBookingUpdate(const std::string& response) {
     {
        std::lock_guard<std::mutex> lock(responseMutex_);
        lastResponse_ = response;
    }
    
    
}

void CinemaClient::handleServerMessage(const std::string& response) {
    std::cout << "\nServer Update:\n" << response << std::endl;
}

void CinemaClient::parseAndUpdateShows(const std::string& response) {
    std::lock_guard<std::mutex> lock(showsMutex_);
    shows_.clear(); 
    

    
    std::stringstream ss(response);
    std::string line;
    std::string currentTheater;
    Shows* currentShow = nullptr; 
    
    while (std::getline(ss, line)) {
        if (line.find(CinemaProtocol::THEATER_PREFIX) == 0) {
            currentTheater = line.substr(CinemaProtocol::THEATER_PREFIX_LEN);
            currentShow = nullptr; 
        }
        else if (line.find(CinemaProtocol::MOVIE_PREFIX) == 0 && !currentTheater.empty()) {
            std::string movieLine = line.substr(CinemaProtocol::MOVIE_PREFIX_LEN);
            
            size_t datePos = movieLine.find(" (");
            if (datePos != std::string::npos) {
                std::string movieName = movieLine.substr(0, datePos);
                
                size_t startDate = movieLine.find("(") + 1;
                size_t endDate = movieLine.find(")");
                if (startDate < endDate) {
                    std::string dateTime = movieLine.substr(startDate, endDate - startDate);
                    
                    shows_.emplace_back(movieName, dateTime, currentTheater);
                    currentShow = &shows_.back(); 
                }
            }
        }
        else if (line.find("    Available seats:") != std::string::npos && currentShow != nullptr) {
            std::string seatsLine = line.substr(line.find(":") + 1);
            
            size_t totalPos = seatsLine.find("(Total:");
            if (totalPos != std::string::npos) {
                seatsLine = seatsLine.substr(0, totalPos);
            }
            
            std::vector<bool> seatStatus(20, true);
            
            std::stringstream seatStream(seatsLine);
            std::string token;
            
            std::vector<int> parsedSeats; 
            
            while (seatStream >> token) {
                if (!token.empty() && token.back() == ',') {
                    token.pop_back();
                }
                
                try {
                    int seatNum = std::stoi(token);
                    if (seatNum >= 1 && seatNum <= 20) {
                        seatStatus[seatNum - 1] = false; 
                        parsedSeats.push_back(seatNum); 
                    }
                } catch (const std::exception&) {
                    continue;
                }
            }
            
            currentShow->updateSeatAvailability(seatStatus);
        }
    }
}

std::vector<Shows> CinemaClient::getShows() const {
    std::lock_guard<std::mutex> lock(showsMutex_);
    return shows_;
}