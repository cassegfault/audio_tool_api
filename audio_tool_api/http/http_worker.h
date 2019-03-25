//
//  http_worker.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_worker_async_hpp
#define http_worker_async_hpp

#include <stdio.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/optional/optional.hpp>
#include <boost/bind.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast/http.hpp>

#include "http/fields_alloc.h"
#include "http/http_connection.h"
#include "http/http_exception.h"
#include "http/routes.h"
#include "handlers/base_handler.h"
#include "utilities/stats_client.h"
#include "utilities/timer.h"

namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;
using namespace std;

class http_worker {
public:
    http_worker(boost::asio::io_context & _work_thread_context): work_thread_context(_work_thread_context), parser_(boost::none) {};
    http_worker(const http_worker& other): work_thread_context(other.work_thread_context), parser_(boost::none) {}
    ~http_worker(){};
    
    void start(shared_ptr<http_connection> _conn);
    bool has_finished() { return _has_finished; }
private:
    shared_ptr<http_connection> conn;
    boost::asio::io_context & work_thread_context;
    bool _has_finished = true;
    
    // I would like to replace the http stuff with either a smaller http parser or my own
        using alloc_t = fields_alloc<char>;
        using body_t = http::string_body;
        using fields_t = http::basic_fields<alloc_t>;
        // For reads
        boost::beast::flat_static_buffer<8192> buffer_;
        boost::optional<http::request_parser<body_t, alloc_t>> parser_;
        alloc_t alloc_{8192};
    
        // For writes
        boost::optional<http::response<body_t, fields_t>> response;
        boost::optional<http::response_serializer<body_t, fields_t>> serializer;
    
    unique_ptr<base_handler> find_route(string path);
    
    void read();
    void read_handler(boost::beast::error_code & ec);
    void write_handler(boost::beast::error_code & ec);
    void process(http::request<body_t, fields_t> const & req);
    void build_response(HTTPResponse & handler_result);
    void build_response(http::status error_code, string body);
    void write();
    void close_socket(boost::beast::error_code & ec);
};

#endif /* http_worker_hpp */
