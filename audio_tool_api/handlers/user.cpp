//
//  user.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "user.h"

void user_handler::get(http_request request_data){
    response_data = user;
}
