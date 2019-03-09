//
//  test.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/5/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef test_handler_hpp
#define test_handler_hpp

#include <stdio.h>
#include <thread>
#include "base_handler.h"

class test_handler : public base_handler {
public:
    test_handler() { requires_authentication = false; }
    void get(http_request request_data) override;
};

#endif /* test_hpp */
