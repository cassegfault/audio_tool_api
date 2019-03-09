//
//  base_handler.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "base_handler.h"


base_handler::base_handler () {
    db = new db::Connection();
    db->open(config()->db_connection_string, config()->db_user, config()->db_password, config()->db_database);
    
    // Defaults for handlers, not necessarily undefined routes
    response.status = 200;
    response.content_type = "application/json";
    response.empty_body = false;
    response.headers.emplace("Access-Control-Allow-Origin","*");
    
}


// If any of these definitions get called, it is because they have not been overriden. They are undefined routes and should 404
void base_handler::get(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}

void base_handler::post(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}

void base_handler::put(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}

void base_handler::options(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}

void base_handler::delete_(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}

void base_handler::head(http_request request_data) {
    response.set_content(not_found_response);
    response.status = not_found_status;
}
