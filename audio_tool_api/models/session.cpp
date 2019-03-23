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

// Creates 128 bytes of random data, encodes it in base64
string session::create_token(){
    unsigned char token[128];
    RAND_bytes(token, 128);
    return base64_encode(token, 128);;
}

void session::create(db::Connection * db, int _user_id) {
    // Generate a token
    string new_token = create_token();
    
    // Create a session with that token
    auto query = db->query("INSERT INTO sessions (user_id, token, created, expires) VALUES (?,?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP() + ?)", _user_id, new_token, session_length);
    query.execute();
    
    // Confirm that it saved properly and load the created / expired timestamps
    auto confirmation_query = db->query("SELECT created, expires FROM sessions WHERE token=?", new_token);
    if(confirmation_query.row_count() < 1){
        throw runtime_error("Session did not insert properly into the database");
    }
    auto confirmation_results = confirmation_query.row();
    
    // Fill the class
    user_id = _user_id;
    token = new_token;
    created = confirmation_results["created"];
    expires = confirmation_results["expires"];
}

void session::refresh(db::Connection * db) {
    auto query = db->query("UPDATE sessions SET expires = UNIX_TIMESTAMP() + ? WHERE token=? AND user_id=?", session_length, token, user_id);
    query.execute();
}

void session::lookup(db::Connection * db, string _token){
    auto query = db->query("SELECT id, created, expires, user_id FROM sessions WHERE token=? AND expires > UNIX_TIMESTAMP()", token);
    if(query.row_count() < 1){
        throw runtime_error("No sessions found for this token");
    }
    
    auto results = query.row();
    created = results["created"];
    expires = results["expires"];
    user_id = results["user_id"];
    token = _token;
}
