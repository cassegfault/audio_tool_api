//
//  listener.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef listener_h
#define listener_h

#include "session.h"
#include <stdio.h>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ip/tcp.hpp>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class Listener : public std::enable_shared_from_this<Listener> {
    ssl::context& ctx_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    
public:
    Listener(boost::asio::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint);
    
    // Failure reporting
    void fail(boost::system::error_code ec, char const* what);
    
    // Start accepting incoming connections
    void run();
    
    void do_accept();
    
    void on_accept(boost::system::error_code ec);
};

#endif /* listener_h */
