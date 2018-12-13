//
//  http_request.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "http_request.h"
#include <iostream>
HTTPRequest::HTTPRequest(http::request<request_body_t, http::basic_fields<alloc_t>> const& req): _req(req) {
    string current_param_name;
    string current_param_value;
    bool in_params = false;
    bool in_value = false;
    
    verb = _req.method();
    
    for (const char& c : _req.target()) {
        if (c == '?') {
            path = current_param_name;
            current_param_name.clear();
            in_params = true;
            continue;
        } else if (c == '=' && in_params && !in_value) {
            in_value = true;
            continue;
        } else if (c == '&' && in_params) {
            url_params[current_param_name] = current_param_value;
            current_param_name.clear();
            current_param_value.clear();
            in_value = false;
            continue;
        } else if (in_params && in_value) {
            current_param_value += c;
            continue;
        } else {
            current_param_name += c;
            continue;
        }
    }
    
    // when we reach the end of the string we need to handle the last piece
    if (current_param_name.length() > 0) {
        if (in_params) {
            url_params.emplace(current_param_name, current_param_value);
        } else {
            path = current_param_name;
        }
    }
    
    // Store the headers (not as efficient, but much better accessibility)
    for(auto const& header : req) {
        headers.emplace(header.name_string(), header.value());
    }
    cout << "content-type: " << req.at("content-type") << endl;
    //if (req.at("content-type").find("multipart") > -1){
        memset(&m_callbacks, 0, sizeof(multipart_parser_settings));
        m_callbacks.on_part_data = [](multipart_parser* p, const char *at, size_t length){
            HTTPRequest * self = (HTTPRequest *)multipart_parser_get_data(p);
            self->current_file_data.append(at,length);
            return 0;
        };
        m_callbacks.on_part_data_end = [](multipart_parser* p){
            HTTPRequest * self = (HTTPRequest *)multipart_parser_get_data(p);
            self->files.emplace_back(self->current_file_data);
            self->current_file_data.clear();
            return 0;
        };
    
        std::string boundary = "";
        if (req.find("content-type") != req.end()) {
            boundary = req.at("content-type").to_string();
            auto index = boundary.find("boundary=");
            
            if(index != -1) {
                boundary = "--" + boundary.substr(index + 9);
            } else {
                cerr << "Invalid boundary:" << boundary << endl;
            }
        }
    
        m_parser = multipart_parser_init(boundary.c_str(), &m_callbacks);
        multipart_parser_set_data(m_parser, this);
        multipart_parser_execute(m_parser, req.body().c_str(), req.body().length());
    
    //}
    
}

HTTPRequest::~HTTPRequest(){
    
}
