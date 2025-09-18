#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <queue>
#include <functional>
#include "cinema.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

using MessageCallback = std::function<std::string(const std::string&, bool&)>;
using InitialDataCallback = std::function<std::string()>;
using BroadcastDataCallback = std::function<std::string()>;

namespace CinemaProtocol {
    constexpr const char* CINEMA_DATA_HEADER = "=== CINEMA DATA STREAM ===";
    constexpr const char* CINEMA_DATA_FOOTER = "=== END CINEMA DATA ===";
    constexpr const char* UPDATE_DATA_HEADER = "=== UPDATED CINEMA DATA ===";
    constexpr const char* UPDATE_DATA_FOOTER = "=== END UPDATED DATA ===";
    constexpr const char* BOOKING_UPDATE_PREFIX = "BOOKING_UPDATE:\n";
    constexpr int MAX_SEATS = 20;
}

class WebSocketServer;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
private:
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    WebSocketServer* server_;
    std::queue<std::string> message_queue_;
    bool writing_;

public:
    explicit WebSocketSession(tcp::socket&& socket, WebSocketServer* server);
    void run();
    void sendBroadcastMessage(const std::string& message);

private:
    void on_accept(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void do_read();
    void do_write();
    void send_message(const std::string& message);
};

class WebSocketServer {
private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::set<std::shared_ptr<WebSocketSession>> sessions_;
    MessageCallback messageCallback_;
    InitialDataCallback initialDataCallback_;
    BroadcastDataCallback broadcastDataCallback_;

public:
    WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint,
                   MessageCallback messageCallback,
                   InitialDataCallback initialDataCallback,
                   BroadcastDataCallback broadcastDataCallback);
    void run();
    void addSession(std::shared_ptr<WebSocketSession> session);
    void removeSession(std::shared_ptr<WebSocketSession> session);
    void broadcastUpdate();
    std::string handleMessage(const std::string& message, bool& shouldBroadcast);
    std::string getInitialData();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};