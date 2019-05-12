//
//  user.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef user_handler_hpp
#define user_handler_hpp

#include "base_handler.h"
#include "../models/user.h"

class user_handler : public base_model_handler<user_model> {
public:
    void get(http_request & request_data) override;
};

#endif /* user_hpp */
