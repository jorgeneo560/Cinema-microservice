#include "websocket_server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

WebSocketSession::WebSocketSession(tcp::socket&& socket, WebSocketServer* server)
    : ws_(std::move(socket)), server_(server), writing_(false) {}

void WebSocketSession::run() {
    server_->addSession(shared_from_this());
    
    ws_.async_accept(
        beast::bind_front_handler(
            &WebSocketSession::on_accept,
            shared_from_this()));
}

void WebSocketSession::sendBroadcastMessage(const std::string& message) {
    send_message(message);
}

void WebSocketSession::send_message(const std::string& message) {
    message_queue_.push(message);
    
    if (!writing_) {
        do_write();
    }
}

void WebSocketSession::do_write() {
    if (message_queue_.empty()) {
        writing_ = false;
        return;
    }
    
    writing_ = true;
    std::string message = message_queue_.front();
    message_queue_.pop();
    
    ws_.async_write(
        net::buffer(message),
        beast::bind_front_handler(
            &WebSocketSession::on_write,
            shared_from_this()));
}

void WebSocketSession::do_read() {
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketSession::on_read,
            shared_from_this()));
}

void WebSocketSession::on_accept(beast::error_code ec) {
    if (ec) {
        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
        server_->removeSession(shared_from_this());
        return;
    }

    send_message(server_->getInitialData());
    do_read();
}

void WebSocketSession::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec) {
        std::cerr << "WebSocket write error: " << ec.message() << std::endl;
        server_->removeSession(shared_from_this());
        return;
    }

    do_write();
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec == websocket::error::closed) {
        server_->removeSession(shared_from_this());
        return;
    }

    if (ec) {
        std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        server_->removeSession(shared_from_this());
        return;
    }

    std::string received = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());
    
    std::cout << "WebSocket received: " << received << std::endl;
    
    bool shouldBroadcast = false;
    std::string response = server_->handleMessage(received, shouldBroadcast);
    
    send_message(response);
    
    if (shouldBroadcast) {
        server_->broadcastUpdate();
    }
    
    do_read();
}

WebSocketServer::WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint,
                               MessageCallback messageCallback,
                               InitialDataCallback initialDataCallback,
                               BroadcastDataCallback broadcastDataCallback)
    : ioc_(ioc), acceptor_(ioc), 
      messageCallback_(messageCallback),
      initialDataCallback_(initialDataCallback),
      broadcastDataCallback_(broadcastDataCallback) {
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
        std::make_shared<WebSocketSession>(std::move(socket), this)->run();
    }

    do_accept();
}

void WebSocketServer::addSession(std::shared_ptr<WebSocketSession> session) {
    sessions_.insert(session);
    std::cout << "WebSocket client connected. Total clients: " << sessions_.size() << std::endl;
}

void WebSocketServer::removeSession(std::shared_ptr<WebSocketSession> session) {
    sessions_.erase(session);
    std::cout << "WebSocket client disconnected. Total clients: " << sessions_.size() << std::endl;
}

void WebSocketServer::broadcastUpdate() {
    std::string updateMessage = broadcastDataCallback_();
    
    std::cout << "Broadcasting update to " << sessions_.size() << " clients" << std::endl;
    for (auto session : sessions_) {
        session->sendBroadcastMessage(updateMessage);
    }
}

std::string WebSocketServer::handleMessage(const std::string& message, bool& shouldBroadcast) {
    return messageCallback_(message, shouldBroadcast);
}

std::string WebSocketServer::getInitialData() {
    return initialDataCallback_();
}