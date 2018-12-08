//
//  session.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef session_h
#define session_h

#include <stdio.h>
#include <iostream>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <thread>
#include "routes.h"

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class Session : public std::enable_shared_from_this<Session>
{
    struct send_lambda {
        Session& self_;
        
        explicit send_lambda(Session& self): self_(self) { }
        
        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const {
            auto sp = std::make_shared< http::message<isRequest, Body, Fields> >(std::move(msg));
            
            self_.res_ = sp;
            
            http::async_write(self_.stream_, *sp, boost::asio::bind_executor(self_.strand_, std::bind(&Session::on_write, self_.shared_from_this(), std::placeholders::_1, std::placeholders::_2, sp->need_eof())));
        }
    };
    
    tcp::socket socket_;
    ssl::stream<tcp::socket&> stream_;
    boost::asio::strand<
    boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;
    
public:
    // Take ownership of the socket
    explicit Session(tcp::socket socket, ssl::context& ctx): socket_(std::move(socket)), stream_(socket_, ctx), strand_(socket_.get_executor()), lambda_(*this) { }
    
    // Failure reporting
    void fail(boost::system::error_code ec, char const* what);
    
    // Request Handling
    template<class Body, class Allocator, class Send>
    void handle_request(boost::beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
    
    // Start the asynchronous operation
    void run();
    
    // Callbacks
    void on_handshake(boost::system::error_code ec);
    
    void do_read();
    
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
    
    void on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close);
    
    void do_close();
    
    void on_shutdown(boost::system::error_code ec);
};

#endif /* session_h */
