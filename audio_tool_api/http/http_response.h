//
//  http_result.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef http_result_h
#define http_result_h

#include <string>
#include <unordered_map>

using namespace std;

class HTTPResponse {
public:
    HTTPResponse() {}
    
    unordered_map<string, string> headers;
    string body;
    bool empty_body;
    int status;
    string content_type;
};

#endif /* http_result_h */
