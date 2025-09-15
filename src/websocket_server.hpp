#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include "cinema.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// WebSocket session class
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
private:
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::vector<Shows>* shows_; // Non-const pointer to shows data for booking

public:
    explicit WebSocketSession(tcp::socket&& socket, std::vector<Shows>* shows);
    void run();

private:
    std::string generateCinemaData();
    std::string handleBookingRequest(const std::string& message);
    void on_accept(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
};

// WebSocket server class
class WebSocketServer {
private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::vector<Shows>* shows_; // Non-const pointer to shows data for booking

public:
    WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint, std::vector<Shows>* shows);
    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};