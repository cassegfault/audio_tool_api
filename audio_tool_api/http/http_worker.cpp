//
//  http_worker.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_worker.h"


void http_worker::start(http_connection &_conn) {
    if (!_has_finished) {
        throw new runtime_error("Starting on a worker that isn't finished");
    }
    conn = _conn;
    _has_finished = false;
    read();
}

void http_worker::read(){
    //parser_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    http::request_parser<body_t, alloc_t> parser_(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    http::async_read(*conn->socket, buffer_, parser_, [&,this](boost::beast::error_code ec, std::size_t) {
        if(ec){
            _has_finished = true;
        } else {
            process(parser_.get());
        }
    });
}

void http_worker::process(http::request<body_t, fields_t> const & req){
    LOG(INFO) << req.target();
    stats()->increment("requests");
    timer request_timer;
    request_timer.start();
    http_request req_data(req);
    
    std::unique_ptr<base_handler> found_handler(find_route(req_data.path));
    
    if (found_handler == nullptr) {
        return build_response(http::int_to_status(404), "Route not found");
    }
    
    if(found_handler->requires_authentication){
        if(req.find("session-token") == req.end()) {
            return build_response(http::status::unauthorized, "Unauthorized");
        }
        string token = req.at("session-token").to_string();
        
        try {
            found_handler->user.fill_by_token(found_handler->db, token);
        } catch (runtime_error e){
            return build_response(http::status::unauthorized, string("Unauthorized: ") + e.what());
        }
        try {
            found_handler->user.refresh_session(found_handler->db);
        } catch (runtime_error e) {
            // Sentry call here
            LOG(WARNING) << "Could not refresh session";
        }
    }
    
    // Run the handler
    try {
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
                return build_response(http::status::bad_request, "Invalid request-method '" + req.method_string().to_string() + "'\r\n");
                break;
        }
        found_handler->finish(req_data);
    } catch (http_exception e) {
        return build_response(http::int_to_status(e.http_code), e.what());
    }catch (std::exception& e) {
        string error_message = "There was an error processing your request – Please wait and try again: ";
        error_message += e.what();
        return build_response(http::status::internal_server_error,  error_message );
    } catch (...) {
        return build_response(http::status::internal_server_error, "Unhandled exception caught by the server – Please wait and try again");
    }
    
    build_response(found_handler->response);
}

void http_worker::build_response(HTTPResponse & handler_result){
    response.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    response->result(handler_result.status);
    
    response->set(http::field::server, "Audio Tool API");
    response->set(http::field::content_type, handler_result.content_type);
    
    // Set any headers
    for(auto header : handler_result.headers){
        response->set(header.first, header.second);
    }
    
    response->body() = handler_result.get_content();
    response->prepare_payload();
    
    serializer.emplace(*response);
}

void http_worker::build_response(http::status status, string body){
    response.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    
    response->result(status);
    response->keep_alive(false);
    response->set(http::field::server, "Audio Tool API");
    response->set(http::field::content_type, "application/json");
    json j;
    j["message"] = body;
    j["status"] = status;
    response->body() = j.dump();
    response->prepare_payload();
    
    serializer.emplace(*response);
}

void http_worker::write() {
    http::async_write(*conn->socket, *serializer, [this](boost::beast::error_code ec, std::size_t) {
        conn->socket->shutdown(tcp::socket::shutdown_send, ec);
        serializer.reset();
        response.reset();
        _has_finished = true;
    });
}

unique_ptr<base_handler> http_worker::find_route(string path) {
    auto found_route = application_routes.find(path);
    if (found_route != application_routes.end()) {
        return unique_ptr<base_handler>(found_route->second());
    }
    return nullptr;
}
