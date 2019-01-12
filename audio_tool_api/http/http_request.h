//
//  http_request.h
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//
//  This serves as a wrapper for boost::beast::http::request in case that needs to be changed later
//

#ifndef http_request_h
#define http_request_h

#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>
#include <vector>
#include "../utilities/json.hpp"
#include "fields_alloc.h"
#include "multipart_parser.h"

using namespace std;
namespace http = boost::beast::http;

class HTTPRequest {
    using request_body_t = http::string_body;
    using alloc_t = fields_alloc<char>;
public:
    HTTPRequest(http::request<request_body_t, http::basic_fields<alloc_t>> const& req);
    ~HTTPRequest();
    
    string path;
    unordered_map<string, string> url_params;
    unordered_map<string, string> headers;
    http::verb verb;
    http::request<request_body_t, http::basic_fields<alloc_t>> const& _req;
    
    vector<string> files;
    
    bool has_header(string header_name);
    bool has_param(string param_name);
    
    nlohmann::json json();
    
private:
    string not_found_response = "404 Route not found";
    int not_found_status = 404;
    
    multipart_parser* m_parser;
    multipart_parser_settings m_callbacks;
    int current_file_index = 0;
    string current_file_data;
    
    
};

#endif /* http_request_h */
