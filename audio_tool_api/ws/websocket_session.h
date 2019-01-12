//
//  websocket_session.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/7/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef websocket_session_hpp
#define websocket_session_hpp

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class websocket_session : public std::enable_shared_from_this<websocket_session> {
    websocket::stream<tcp::socket> ws_;
    net::strand<
    net::io_context::executor_type> strand_;
    beast::multi_buffer buffer_;
    
public:
    // Take ownership of the socket
    explicit websocket_session(tcp::socket socket);
    
    // Start the asynchronous operation
    void run();
    
    void on_accept(beast::error_code ec);
    
    void do_read();
    
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
};

#endif /* websocket_session_hpp */
