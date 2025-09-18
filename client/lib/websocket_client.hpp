#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <mutex>
#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include "cinema_Client.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace CinemaProtocol {
    constexpr const char* THEATER_PREFIX = "Theater: ";
    constexpr const char* MOVIE_PREFIX = "  Movie: ";
    constexpr const char* CINEMA_DATA_STREAM = "=== CINEMA DATA STREAM ===";
    constexpr const char* UPDATED_CINEMA_DATA = "=== UPDATED CINEMA DATA ===";
    constexpr const char* BOOKING_UPDATE = "BOOKING_UPDATE:";
    constexpr size_t THEATER_PREFIX_LEN = 9;
    constexpr size_t MOVIE_PREFIX_LEN = 9;
}

class CinemaClient {
public:
    CinemaClient();
    ~CinemaClient();
    bool connect(const std::string& host, const std::string& port);
    void disconnect();
    bool isConnected() const;
    void sendMessage(const std::string& message);
    std::string getLastResponse() const;
    std::string getLastBookingResponse() const;
    void parseAndUpdateShows(const std::string& response);
    std::vector<Shows> getShows() const;

private:
    net::io_context ioc_;
    websocket::stream<tcp::socket> ws_;
    std::atomic<bool> connected_{false};

    std::unique_ptr<std::thread> listenerThread_;
    std::atomic<bool> shouldStop_{false};

    std::string lastResponse_;
    std::string lastBookingResponse_;
    mutable std::mutex responseMutex_;

    std::vector<Shows> shows_;
    mutable std::mutex showsMutex_;

    void startMessageListener();
    void stopMessageListener();
    void processMessage(const std::string& response);
    bool isCinemaDataStream(const std::string& response) const;
    void handleBookingUpdate(const std::string& response);
    void handleServerMessage(const std::string& response);
};