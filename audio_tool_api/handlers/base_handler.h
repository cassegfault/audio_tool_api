//
//  base_handler.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef base_handler_h
#define base_handler_h

#include <stdio.h>
#include <unordered_map>
#include "../http/http_request.h"
#include "../http/http_response.h"

class BaseHandler {
public:
    
    BaseHandler();
    ~BaseHandler(){ cout << "Handler Destructed" << endl; };
    
    virtual void get(HTTPRequest request_data);
    virtual void post(HTTPRequest request_data);
    virtual void put(HTTPRequest request_data);
    virtual void delete_(HTTPRequest request_data);
    virtual void options(HTTPRequest request_data);
    virtual void head(HTTPRequest request_data);
    
    HTTPResponse response;

private:
    string not_found_response = "404 Route not found";
    int not_found_status = 404;
};

#endif /* base_handler_h */
