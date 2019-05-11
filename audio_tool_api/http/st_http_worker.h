//
//  worker.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef http_worker_h
#define http_worker_h

#include "routes.h"
#include "http_worker.h"
#include "http_exception.h"
#include "utilities/stats_client.h"
#include "utilities/timer.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <chrono>


namespace ip = boost::asio::ip;         // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class st_http_worker {
public:
    st_http_worker(st_http_worker const&) = delete;
    st_http_worker& operator=(st_http_worker const&) = delete;
    
    st_http_worker(tcp::acceptor& acceptor) : acceptor_(acceptor) { }
    
    void start() {
        accept();
        check_deadline();
    }
    
    void start_sync(){
        sync_accept();
    }
private:
    using request_body_t = http::string_body;
    
    // The acceptor used to listen for incoming connections.
    tcp::acceptor& acceptor_;
    
    // The socket for the currently connected client.
    tcp::socket socket_{acceptor_.get_executor().context()};
    
    // The buffer for performing reads
    boost::beast::flat_static_buffer<8192> buffer_;
    boost::optional<http::request<http::string_body>> request_;
    
    
    // The timer putting a time limit on requests.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> request_deadline_{
        acceptor_.get_executor().context(), (std::chrono::steady_clock::time_point::max)()};
    
    // The string-based response message.
    boost::optional<http::response<http::string_body>> string_response_;
    
    void accept();
    void sync_accept();
    
    void read_request();
    void sync_read();
    
    void process_request();
    
    void write();
    void sync_write();
    
    void send_bad_response(http::status status, std::string const& error);
    
    void check_deadline();
    
    unique_ptr<base_handler> find_route(string path);
};

#endif /* http_worker_h */
