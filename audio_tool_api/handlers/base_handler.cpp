//
//  base_handler.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "base_handler.h"


BaseHandler::BaseHandler () {
    // Defaults for handlers, not necessarily undefined routes
    response.body = "";
    response.status = 200;
    response.content_type = "application/json";
    response.empty_body = false;
    
}


// If any of these definitions get called, it is because they have not been overriden. They are undefined routes and should 404
void BaseHandler::get(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}

void BaseHandler::post(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}

void BaseHandler::put(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}

void BaseHandler::options(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}

void BaseHandler::delete_(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}

void BaseHandler::head(HTTPRequest request_data) {
    response.body = not_found_response;
    response.status = not_found_status;
}
