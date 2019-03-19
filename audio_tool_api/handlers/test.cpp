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
void test_handler::get(http_request request_data) {
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    /*stringstream s("str");
    ifstream urandom("/dev/urandom", ios::in|ios::binary);
    int random_value = 0, new_random = 0;
    size_t size = sizeof(random_value);
    
    for(int x = 0; x < 1024 * 1024 * 10; x++){
        urandom.read(reinterpret_cast<char*>(&new_random), size);
        
    }
    if(urandom){
        urandom.close();
    }*/
    response.set_content(string("slept"));
}
