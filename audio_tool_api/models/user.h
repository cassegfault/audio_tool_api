//
//  user.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef user_model_hpp
#define user_model_hpp

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdexcept>
#include <glog/logging.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "session.h"
#include "../utilities/database.h"
#include "../utilities/serialize.h"

using namespace std;

class user_model {
public:
    user_model(){};
    
    int id;
    string email_address;
    session current_session;
    
    void create(db::Connection * db, string email, string password);
    session create_session(db::Connection * db);
    
    void fill_by_id(db::Connection * db, int _id);
    void fill_by_token(db::Connection * db, string token);
    void fill_by_email(db::Connection * db, string email);
    
    void refresh_session(db::Connection * db);
    
    bool authenticate(db::Connection * db, string email, string pass);
    
    operator nlohmann::json(){
        return serialize::jsonify(
                KVP(email_address),
                KVP(id));
    }
};

#endif /* user_hpp */
