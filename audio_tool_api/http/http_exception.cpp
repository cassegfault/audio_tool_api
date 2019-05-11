//
//  http_exception.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_exception.h"
const char * get_http_status(int code) {
    return error_map[code];
}
