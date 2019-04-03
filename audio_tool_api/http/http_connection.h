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
#include <boost/optional/optional.hpp>
#include <glog/logging.h>
#include <chrono>

using namespace std;
using tcp = boost::asio::ip::tcp;

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    http_connection(boost::asio::io_context & context, atomic<int> & _request_counter): socket(context), has_socket(true), created_time(chrono::steady_clock::now()), request_counter(_request_counter) {
        request_counter++;};
    http_connection(int _raw_socket, atomic<int> & _request_counter): raw_socket(_raw_socket), has_socket(true), created_time(chrono::steady_clock::now()), request_counter(_request_counter), is_raw(true) {
        request_counter++;};
    
    ~http_connection(){
        DLOG_IF(ERROR,!safe_to_destruct) << "HTTP Connection closed improperly";
        request_counter--;
    }
    int raw_socket = -1;
    bool is_raw=false;
    boost::optional<tcp::socket> socket;
    shared_ptr<http_connection> get_ptr() { return shared_from_this(); }
    void build_socket(boost::asio::io_context & context) {
        socket.emplace(context);
        socket->assign(boost::asio::ip::tcp::v4(), raw_socket);
    }
    void close(boost::asio::io_context & work_thread_context, boost::beast::error_code & ec){
        socket->shutdown(tcp::socket::shutdown_both, ec);
        boost::system::error_code close_err;
        socket->close(close_err);
        if(close_err) {
            LOG(ERROR) << close_err.message();
        }
        closed_time = chrono::steady_clock::now();
        safe_to_destruct = true;
    }
    chrono::steady_clock::time_point created_time;
    chrono::steady_clock::time_point accepted_time;
    chrono::steady_clock::time_point started_time;
    chrono::steady_clock::time_point closed_time;
    bool safe_to_destruct = false;
    atomic<int> & request_counter;
private:
    bool has_socket = false;
    
    
};

#endif /* http_connection_hpp */
