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
#include "base_handler.h"
#include "../utilities/requests.h"
#include "../utilities/json.hpp"
#include "../models/user.h"

class AuthHandler : public BaseHandler {
    void post(HTTPRequest request_data) override;
    void get(HTTPRequest request_data) override;
};

#endif /* auth_hpp */
