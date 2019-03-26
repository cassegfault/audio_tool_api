//
//  http_connection.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_connection_h
#define http_connection_h

#include <stdio.h>
#include <iostream>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/asio/strand.hpp>
#include <glog/logging.h>
#include <chrono>

using namespace std;
using tcp = boost::asio::ip::tcp;

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    http_connection(boost::asio::io_context & context): socket(context), has_socket(true), created_time(chrono::steady_clock::now()) {};
    ~http_connection(){}
    tcp::socket socket;
    shared_ptr<http_connection> get_ptr() { return shared_from_this(); }
    
    void close(boost::asio::io_context & work_thread_context, boost::beast::error_code & ec){
        closed_time = chrono::steady_clock::now();
    }
    chrono::steady_clock::time_point created_time;
    chrono::steady_clock::time_point accepted_time;
    chrono::steady_clock::time_point closed_time;
private:
    bool has_socket = false;
    
};

#endif /* http_connection_hpp */
