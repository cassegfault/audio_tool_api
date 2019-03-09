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
#include <glog/logging.h>
#include "../utilities/json.hpp"

using namespace std;

class HTTPResponse {
public:
    HTTPResponse() {}
    
    unordered_map<string, string> headers;
    bool empty_body;
    int status;
    string content_type;
    
    string get_content();
    void set_content(nlohmann::json j);
    void set_content(string j);
    
    template<typename T>
    void set_content(const T & content){
        use_json_body = false;
        body_content = to_string(content);
    };
private:
    bool use_json_body = false;
    nlohmann::json json_content;
    string body_content;
};

#endif /* http_result_h */
