//
//  auth.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef auth_hpp
#define auth_hpp

#include <stdio.h>
#include <curl/curl.h>

#include "base_handler.h"
#include "../utilities/requests.h"
#include "../utilities/json.hpp"

class auth_handler : public base_handler {
public:
    auth_handler() { requires_authentication = false; }
    void post(http_request request_data) override;
    void put(http_request request_data) override;
    void get(http_request request_data) override;
};

#endif /* auth_hpp */
