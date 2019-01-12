//
//  user.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef user_hpp
#define user_hpp

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <glog/logging.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "session.h"
#include "../utilities/database.h"

using namespace std;

class user {
public:
    user(){};
    
    int id;
    string email_address;
    session current_session;
    
    void create(db::Connection * db, string email, string password);
    void create_session(db::Connection * db);
    bool authenticate(db::Connection * db, string email, string pass);
};

#endif /* user_hpp */
