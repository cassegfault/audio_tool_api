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
    
    int CountHeaders(const std::string& body) {
        multipart_parser_execute(m_parser, body.c_str(), body.size());
     
        return m_headers;
    }
    
private:
    static int ReadHeaderName(multipart_parser* p, const char *at, size_t length) {
        HTTPRequest* me = (HTTPRequest*)multipart_parser_get_data(p);
        string datastr ((char *)at, length);
        cout << "Header Name " << datastr << endl;
        me->m_headers++;
        return me->m_headers;
    }
    static int ReadHeaderValue(multipart_parser* p, const char *at, size_t length) {
        HTTPRequest* me = (HTTPRequest*)multipart_parser_get_data(p);
        me->m_headers++;
        return me->m_headers;
    }
    
    string not_found_response = "404 Route not found";
    int not_found_status = 404;
    
    multipart_parser* m_parser;
    multipart_parser_settings m_callbacks;
    int m_headers;
    
};

#endif /* http_request_h */
