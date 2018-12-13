//
//  worker.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef http_worker_h
#define http_worker_h

#include "fields_alloc.h"
#include "routes.h"
#include "http_worker.h"

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

namespace ip = boost::asio::ip;         // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class HTTPWorker {
public:
    HTTPWorker(HTTPWorker const&) = delete;
    HTTPWorker& operator=(HTTPWorker const&) = delete;
    
    HTTPWorker(tcp::acceptor& acceptor) : acceptor_(acceptor) { }
    
    void start() {
        accept();
        check_deadline();
    }
    
private:
    using alloc_t = fields_alloc<char>;
    using request_body_t = http::string_body;
    
    // The acceptor used to listen for incoming connections.
    tcp::acceptor& acceptor_;
    
    // The socket for the currently connected client.
    tcp::socket socket_{acceptor_.get_executor().context()};
    
    // The buffer for performing reads
    boost::beast::flat_static_buffer<8192> buffer_;
    
    // The allocator used for the fields in the request and reply.
    alloc_t alloc_{8192};
    
    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> parser_;
    
    // The timer putting a time limit on requests.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> request_deadline_{
        acceptor_.get_executor().context(), (std::chrono::steady_clock::time_point::max)()};
    
    // The string-based response message.
    boost::optional<http::response<http::string_body, http::basic_fields<alloc_t>>> string_response_;
    
    // The string-based response serializer.
    boost::optional<http::response_serializer<http::string_body, http::basic_fields<alloc_t>>> string_serializer_;
    
    void accept();
    
    void read_request();
    
    void process_request(http::request<request_body_t, http::basic_fields<alloc_t>> const& req);
    
    void send_bad_response(http::status status, std::string const& error);
    
    void check_deadline();
    
    unique_ptr<BaseHandler> find_route(string path);
};

#endif /* http_worker_h */
