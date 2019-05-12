//
//  http_worker.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_worker.h"


void http_worker::start(shared_ptr<http_connection> _conn) {
    if (!_has_finished) {
        throw new runtime_error("Starting on a worker that isn't finished");
    }
    //conn.reset(_conn.get());
    //conn = std::move(_conn);
    conn.swap(_conn);
    conn->started_time = chrono::steady_clock::now();
    _has_finished = false;
    _has_started = true;
    response.reset();
    request_.reset();
    response.emplace();
    request_.emplace();
    if(conn->is_raw){
        conn->build_socket(work_thread_context);
    }
    read();
}

void http_worker::read(){
    //parser_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    http::async_read(*conn->socket, buffer_, *request_, boost::bind(&http_worker::read_handler,this, boost::asio::placeholders::error, 0));
}
void http_worker::read_handler(boost::beast::error_code & ec, size_t bytes_transferred){
    if(ec){
        LOG(ERROR) << ec.message();
        stats()->increment("read_errors");
        close_socket(ec);
    } else {
        process();
    }
}
void http_worker::process(){
    stats()->increment("requests");
    timer request_timer;
    request_timer.start();
    http_request req_data(*request_);
    LOG(INFO) << request_->target();
    DLOG(INFO) << req_data.path;
    
    std::unique_ptr<base_handler> found_handler(find_route(req_data.path));
    
    if (found_handler == nullptr) {
        return build_response(http::int_to_status(404), "Route not found");
    }
    
    if(found_handler->requires_authentication){
        if(request_->find("session-token") == request_->end()) {
            return build_response(http::status::unauthorized, "Unauthorized");
        }
        string token = request_->at("session-token").to_string();
        
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
        found_handler->init(work_thread_context);
        // we need to use a req_data pointer if
        found_handler->setup(&req_data);
        switch (request_->method()) {
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
                stats()->time("request_length", (int)request_timer.milliseconds());
                return build_response(http::status::bad_request, "Invalid request-method '" + request_->method_string().to_string() + "'\r\n");
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
    request_timer.stop();
    if(request_timer.milliseconds() > 10000) {
        DLOG(WARNING) << "Handler took " << request_timer.milliseconds() << "ms";
    }
    auto ms = request_timer.milliseconds();
    stats()->time("handler_length", (int)ms);
    build_response(found_handler->response);
}

void http_worker::build_response(HTTPResponse & handler_result){
    //response.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple());
    //response.reset();
    response->result(handler_result.status);
    
    response->set(http::field::server, "Audio Tool API");
    response->set(http::field::content_type, handler_result.content_type);
    response->keep_alive(false);
    
    // Set any headers
    for(auto header : handler_result.headers){
        response->set(header.first, header.second);
    }
    
    response->body() = handler_result.get_content();
    response->prepare_payload();
    
    //serializer.emplace(*response);
    write();
}

void http_worker::build_response(http::status status, string body){
    //response.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple());
    //response.reset();
    
    response->result(status);
    response->keep_alive(false);
    response->set(http::field::server, "Audio Tool API");
    response->set(http::field::content_type, "application/json");
    json j;
    j["message"] = body;
    j["status"] = status;
    response->body() = j.dump();
    response->prepare_payload();
    
    //serializer.emplace(*response);
    write();
}

void http_worker::write() {
    http::async_write(*conn->socket,*response,boost::bind(&http_worker::write_handler, this,boost::asio::placeholders::error, 0));
}
void http_worker::write_handler(boost::beast::error_code & ec, size_t unused){
    if(ec){
        LOG(ERROR) << ec.message();
        stats()->increment("write_errors");
    }
    
    close_socket(ec);
}

void http_worker::close_socket(boost::beast::error_code & ec){
    conn->close(work_thread_context, ec);
    /*
     created               started
        |----------|----------|--------|
                accepted            closed
     closed - created is total time
     started - accepted is time in queue
     */
    int diff = (int)chrono::duration_cast<chrono::milliseconds>(conn->closed_time - conn->accepted_time).count();
    int queue_diff = (int)chrono::duration_cast<chrono::milliseconds>(conn->started_time - conn->created_time).count();
    int total_diff = (int)chrono::duration_cast<chrono::milliseconds>(conn->closed_time - conn->created_time).count();
    int run_diff = (int)chrono::duration_cast<chrono::milliseconds>(conn->closed_time - conn->started_time).count();
    LOG_IF(ERROR, total_diff > 30 * 1000) << "Connection lasted longer than 30s";
    stats()->time("request_length", diff);
    stats()->time("long_request_length", total_diff);
    stats()->time("queue_length", queue_diff);
    stats()->time("run_length", run_diff);
    //serializer.reset();
    response.reset();
    _has_finished = true;
    conn.reset();
}

unique_ptr<base_handler> http_worker::find_route(string path) {
    string prefix = config()->path_prefix;
    if(prefix.length() > 0 && path.find(prefix) == 0) {
        path = path.substr(0,prefix.length());
    }
    auto found_route = application_routes.find(path);
    if (found_route != application_routes.end()) {
        return unique_ptr<base_handler>(found_route->second());
    }
    return nullptr;
}
