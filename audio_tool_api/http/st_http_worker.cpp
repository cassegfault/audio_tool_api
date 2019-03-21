//
//  worker.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "st_http_worker.h"

void st_http_worker::accept() {
    // Clean up any previous connection.
    boost::beast::error_code ec;
    socket_.close(ec);
    buffer_.consume(buffer_.size());
    
    acceptor_.async_accept(socket_, [this](boost::beast::error_code ec) {
        if (ec) {
            accept();
        } else {
            // Request must be fully processed within 60 seconds.
            request_deadline_.expires_after(std::chrono::seconds(60));
            
            read_request();
        }
    });
}
void st_http_worker::sync_accept() {
    // Clean up any previous connection.
    boost::beast::error_code ec;
    socket_.close(ec);
    buffer_.consume(buffer_.size());
    
    acceptor_.accept(socket_);
    sync_read();
}

void st_http_worker::read_request() {
    // On each read the parser needs to be destroyed and
    // recreated. We store it in a boost::optional to
    // achieve that.
    //
    // Arguments passed to the parser constructor are
    // forwarded to the message object. A single argument
    // is forwarded to the body constructor.
    //
    // We construct the dynamic body with a 1MB limit
    // to prevent vulnerability to buffer attacks.
    //
    parser_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    parser_->body_limit(1024 * 1024 * 1024);
    http::async_read(socket_, buffer_, *parser_, [this](boost::beast::error_code ec, std::size_t) {
        
        if (ec){
            accept();
        }
        else
            process_request(parser_->get());
    });
}
void st_http_worker::sync_read(){
    parser_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    parser_->body_limit(1024 * 1024 * 1024);
    http::read(socket_, buffer_, *parser_);
    process_request(parser_->get());
}

unique_ptr<base_handler> st_http_worker::find_route(string path) {
    auto found_route = application_routes.find(path);
    if (found_route != application_routes.end()) {
        return unique_ptr<base_handler>(found_route->second());
    }
    return nullptr;
}

void st_http_worker::process_request(http::request<request_body_t, http::basic_fields<alloc_t>> const& req) {
    LOG(INFO) << req.target();
    stats()->increment("requests");
    timer request_timer;
    request_timer.start();
    http_request req_data(req);
    
    std::unique_ptr<base_handler> found_handler(find_route(req_data.path));
    
    if (found_handler == nullptr) {
        send_bad_response(http::int_to_status(404), "Route not found");
        stats()->time("request_length", (int)request_timer.microseconds());
        return;
    }
    
    if(found_handler->requires_authentication){
        if(req.find("session-token") == req.end()) {
            send_bad_response(http::status::unauthorized, "Unauthorized");
            return;
        }
        string token = req.at("session-token").to_string();
        
        try {
            found_handler->user.fill_by_token(found_handler->db, token);
        } catch (runtime_error e){
            send_bad_response(http::status::unauthorized, string("Unauthorized: ") + e.what());
            return;
        }
        try {
            found_handler->user.refresh_session(found_handler->db);
        } catch (runtime_error e) {
            // Sentry call here
            LOG(WARNING) << "Could not refresh session";
        }
    }
    
    // Run the handler
    //try {
        // we need to use a req_data pointer if
        found_handler->setup(&req_data);
        switch (req.method()) {
            case http::verb::head:
                found_handler->head(req_data);
                break;
            case http::verb::options:
                found_handler->options(req_data);
                break;
            case http::verb::get:
                found_handler->get(req_data);
                break;
            case http::verb::post:
                found_handler->post(req_data);
                break;
            case http::verb::put:
                found_handler->put(req_data);
                break;
            case http::verb::delete_:
                found_handler->delete_(req_data);
                break;
                
            default:
                stats()->time("request_length", (int)request_timer.microseconds());
                return send_bad_response(http::status::bad_request, "Invalid request-method '" + req.method_string().to_string() + "'\r\n");
                break;
        }
        found_handler->finish(req_data);
    /*} catch (http_exception e) {
        return send_bad_response(http::int_to_status(e.http_code), e.what());
    }catch (std::exception& e) {
        string error_message = "There was an error processing your request – Please wait and try again: ";
        error_message += e.what();
        return send_bad_response(http::status::internal_server_error,  error_message );
    } catch (...) {
        return send_bad_response(http::status::internal_server_error, "Unhandled exception caught by the server – Please wait and try again");
    }*/
    
    
    
    // TODO: swap out string response for empty response if empty_body flag is set
    string_response_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    string_response_->result(found_handler->response.status);
    
    string_response_->set(http::field::server, "Audio Tool API");
    string_response_->set(http::field::content_type, found_handler->response.content_type);
    
    // Set any headers
    for(auto header : found_handler->response.headers){
        string_response_->set(header.first, header.second);
    }
    
    string_response_->body() = found_handler->response.get_content();
    string_response_->prepare_payload();
    
    string_serializer_.emplace(*string_response_);
    
    stats()->time("request_length", (int)request_timer.microseconds());
    write();
}

void st_http_worker::write(){
    http::async_write(socket_, *string_serializer_, [this](boost::beast::error_code ec, std::size_t) {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        string_serializer_.reset();
        string_response_.reset();
        accept();
    });
}

void st_http_worker::sync_write(){
    http::write(socket_, *string_serializer_);
    socket_.shutdown(tcp::socket::shutdown_send);
    string_serializer_.reset();
    string_response_.reset();
    sync_accept();
}

void st_http_worker::send_bad_response(http::status status, std::string const& error) {
    string_response_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    
    string_response_->result(status);
    string_response_->keep_alive(false);
    string_response_->set(http::field::server, "Audio Tool API");
    string_response_->set(http::field::content_type, "application/json");
    json j;
    j["error"] = error;
    string_response_->body() = j.dump();
    string_response_->prepare_payload();
    
    string_serializer_.emplace(*string_response_);
    
    http::async_write(socket_, *string_serializer_, [this](boost::beast::error_code ec, std::size_t) {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        string_serializer_.reset();
        string_response_.reset();
        accept();
    });
}

void st_http_worker::check_deadline() {
    // The deadline may have moved, so check it has really passed.
    if (request_deadline_.expiry() <= std::chrono::steady_clock::now()) {
        // Close socket to cancel any outstanding operation.
        boost::beast::error_code ec;
        socket_.close();
        
        // Sleep indefinitely until we're given a new deadline.
        request_deadline_.expires_at(
                                     std::chrono::steady_clock::time_point::max());
    }
    
    request_deadline_.async_wait([this](boost::beast::error_code) { check_deadline(); });
}
