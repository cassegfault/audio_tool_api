//
//  http_exception.h
//  audio_tool_api
//
//  Created by Chris Pauley on 1/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_exception_h
#define http_exception_h

#include <stdexcept>
#include <string>
#include <unordered_map>

using namespace std;

// From the HTTP Spec at https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
static unordered_map<int, const char *> error_map{
    { 100, "Continue" },
    { 101, "Switching Protocols" },
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 203, "Non-Authoritative Information" },
    { 204, "No Content" },
    { 205, "Reset Content" },
    { 206, "Partial Content" },
    { 300, "Multiple Choices" },
    { 301, "Moved Permanently" },
    { 302, "Found" },
    { 303, "See Other" },
    { 304, "Not Modified" },
    { 305, "Use Proxy" },
    { 306, "(Unused)" },
    { 307, "Temporary Redirect" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 402, "Payment Required" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 406, "Not Acceptable" },
    { 407, "Proxy Authentication Required" },
    { 408, "Request Timeout" },
    { 409, "Conflict" },
    { 410, "Gone" },
    { 411, "Length Required" },
    { 412, "Precondition Failed" },
    { 413, "Request Entity Too Large" },
    { 414, "Request-URI Too Long" },
    { 415, "Unsupported Media Type" },
    { 416, "Requested Range Not Satisfiable" },
    { 417, "Expectation Failed" },
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Unavailable" },
    { 504, "Gateway Timeout" },
    { 505, "HTTP Version Not Supported" }
};

const char * get_http_error(int code) {
    return error_map[code];
}

class http_exception : public runtime_error {
public:
    http_exception(int code, string error): http_code(code), runtime_error(error) {}
    http_exception(int code): http_code(code), runtime_error(get_http_error(code)) {};
    int http_code;
};

#endif /* http_exception_h */
