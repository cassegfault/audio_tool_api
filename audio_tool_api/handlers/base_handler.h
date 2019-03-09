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
#include <glog/logging.h>

#include "../http/http_request.h"
#include "../http/http_response.h"
#include "../http/http_exception.h"

#include "../utilities/config.h"
#include "../utilities/database.h"
#include "../utilities/serialize.h"
#include "../models/user.h"

class base_handler {
public:
    base_handler();
    ~base_handler(){
        db->close();
    };
    db::Connection *db;
    
    virtual void get(http_request request_data);
    virtual void post(http_request request_data);
    virtual void put(http_request request_data);
    virtual void delete_(http_request request_data);
    virtual void options(http_request request_data);
    virtual void head(http_request request_data);
    
    virtual void setup(http_request *request_data) {};
    virtual void finish(http_request request_data) {};
    
    HTTPResponse response;
    bool requires_authentication = true;
    user_model user;

private:
    string not_found_response = "404 Route not found";
    int not_found_status = 404;
};

template<typename model_type>
class base_model_handler : public base_handler {
public:
    void setup(http_request * request_data) override {
        request_data->parse_json();
    }
    
    void finish(http_request request_data) override {
        nlohmann::json json_response;
        json_response["output"] = response_data;
        json_response["status"] = response.status;
        response.set_content(json_response);
    }
    model_type response_data;
};

#endif /* base_handler_h */
