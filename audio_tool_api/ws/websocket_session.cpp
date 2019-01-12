//
//  websocket_session.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/7/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "websocket_session.h"

void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

websocket_session::websocket_session(tcp::socket socket): ws_(std::move(socket)), strand_(ws_.get_executor()) {
}

// Start the asynchronous operation
void websocket_session::run() {
    // Accept the websocket handshake
    ws_.async_accept(
                     net::bind_executor(
                                        strand_,
                                        std::bind(
                                                  &websocket_session::on_accept,
                                                  shared_from_this(),
                                                  std::placeholders::_1)));
}

void websocket_session::on_accept(beast::error_code ec) {
    if(ec)
        return fail(ec, "accept");
    
    // Read a message
    do_read();
}

void websocket_session::do_read() {
    // Read a message into our buffer
    ws_.async_read(
                   buffer_,
                   net::bind_executor(
                                      strand_,
                                      std::bind(
                                                &websocket_session::on_read,
                                                shared_from_this(),
                                                std::placeholders::_1,
                                                std::placeholders::_2)));
}

void websocket_session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    // This indicates that the session was closed
    if(ec == websocket::error::closed)
        return;
    
    if(ec)
        fail(ec, "read");
    
    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write(
                    buffer_.data(),
                    net::bind_executor(
                                       strand_,
                                       std::bind(
                                                 &websocket_session::on_write,
                                                 shared_from_this(),
                                                 std::placeholders::_1,
                                                 std::placeholders::_2)));
}

void websocket_session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if(ec)
        return fail(ec, "write");
    
    // Clear the buffer
    buffer_.consume(buffer_.size());
    
    // Do another read
    do_read();
}
