//
//  http_result.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "http_response.h"

string HTTPResponse::get_content(){
    if(use_json_body) {
        LOG(INFO) << "using JSON body";
        return json_content.dump();
    } else {
        LOG(INFO) << "using String body";
        return body_content;
    }
}

void HTTPResponse::set_content(nlohmann::json j){
    use_json_body = true;
    json_content = j;
}
void HTTPResponse::set_content(string content){
    use_json_body = false;
    body_content = content;
}
