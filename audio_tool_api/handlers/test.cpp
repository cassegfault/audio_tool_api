//
//  test.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/5/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "test.h"
#include <chrono>
#include <fstream>
void test_handler::get(http_request & request_data) {
    const int readsize = 4096;
    char * data = (char *)malloc(readsize);
    ifstream urandom("/dev/urandom", ios::in|ios::binary);
    
    // read 4kb
    urandom.read(data, readsize);
    if(urandom){
        urandom.close();
    }
    nlohmann::json j;
    
    j["response"] = base64_encode((const unsigned char *)data, readsize);
    response.set_content(j);
}
