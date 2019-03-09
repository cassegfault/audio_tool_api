//
//  session.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef session_hpp
#define session_hpp
#include "../utilities/database.h"
#include "../utilities/base64.h"

#include <stdio.h>
#include <string>
#include <time.h>
#include <openssl/rand.h>

using namespace std;

class session {
public:
    string token;
    int expires;
    int created;
    int user_id;
    
    bool is_valid();
    
    void create(db::Connection * db, int _user_id);
    void refresh(db::Connection * db);
    void lookup(db::Connection * db, string _token);
private:
    string create_token();
    
    // Default session length in seconds
    int session_length = 60 * 60;
};

#endif /* session_hpp */
