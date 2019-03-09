//
//  http_work_thread.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_work_thread.h"

void http_work_thread::start() {
    _is_running = true;
}

void http_work_thread::join() {
    if(_thread.joinable()) {
        if(!_thread.join()){
            LOG(ERROR) << "Could not join worker thread!";
        }
    }
    _is_running = false;
}
