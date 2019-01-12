//
//  websocket_listener.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/7/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef websocket_listener_hpp
#define websocket_listener_hpp

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
#include "websocket_session.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class websocket_listener : public std::enable_shared_from_this<websocket_listener>{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    
public:
    websocket_listener(net::io_context& ioc, tcp::endpoint endpoint);
    
    void run();
    
    void do_accept();
    
    void on_accept(beast::error_code ec);
};

#endif /* websocket_listener_hpp */
