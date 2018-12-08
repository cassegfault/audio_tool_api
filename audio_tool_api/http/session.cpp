//
//  session.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "session.h"
#include "routes.h"

void Session::fail(boost::system::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

void Session::run() {
    // Perform the SSL handshake
    stream_.async_handshake(ssl::stream_base::server, boost::asio::bind_executor (strand_, std::bind( &Session::on_handshake, shared_from_this(), std::placeholders::_1)));
}

template<class Body, class Allocator, class Send>
void Session::handle_request(boost::beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
// Respond to GET request
/*http::response<http::file_body> res{
    std::piecewise_construct,
    std::make_tuple(std::move(body)),
    std::make_tuple(http::status::ok, req.version())};
res.set(http::field::server, "homebrew");
res.set(http::field::content_type, "application/javascript");
res.content_length(0);
res.keep_alive(req.keep_alive());
    auto s = req.target();
    http::string_body
    auto route = application_routes["/"]();
    http::file_body::value_type body;
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
return send(std::move(res));*/
}

//////
///// Callback Functions
//////

void Session::on_handshake(boost::system::error_code ec) {
    if(ec)
        return fail(ec, "handshake");
        
    do_read();
}

void Session::do_read() {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};
    
    // Read a request
    /*http::request_parser<boost::beast::http::string_body, fields_alloc<char>> parser_;
    parser_.emplace(std::piecewise_construct,
                    std::make_tuple(),
                    std::make_tuple(alloc_));*/
    
    http::async_read(stream_, buffer_, req_, boost::asio::bind_executor(strand_, std::bind(&Session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    
}

void Session::on_read( boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    // This means they closed the connection
    if(ec == http::error::end_of_stream)
        return do_close();
    
    if(ec)
        return ; //fail(ec, "read");
    
    // Send the response
    handle_request(std::move(req_), lambda_);
}


void Session::on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
    boost::ignore_unused(bytes_transferred);
    
    if(ec)
        return fail(ec, "write");
    
    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }
    
    // We're done with the response so delete it
    res_ = nullptr;
    
    // Read another request
    do_read();
}


void Session::do_close(){
    // Perform the SSL shutdown
    stream_.async_shutdown( boost::asio::bind_executor(strand_, std::bind( &Session::on_shutdown, shared_from_this(), std::placeholders::_1) ) );
}

void Session::on_shutdown(boost::system::error_code ec) {
    if(ec)
        return fail(ec, "shutdown");
    
    // At this point the connection is closed gracefully
}
