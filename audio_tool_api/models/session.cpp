//
//  session.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "session.h"

bool session::is_valid(){
    time_t now = time(nullptr);
    return created < now && expires > now && token.length() > 0;
}
