//
//  session.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef session_hpp
#define session_hpp

#include <stdio.h>
#include <string>
#include <time.h>

using namespace std;

class session {
    string token;
    int expires;
    int created;
    
    bool is_valid();
};

#endif /* session_hpp */
