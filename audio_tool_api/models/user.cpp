//
//  user.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "user.h"

session user_model::create_session(db::Connection * db){
    current_session.create(db, id);
    return current_session;
}

void user_model::fill_by_id(db::Connection * db, int _id){
    auto query = db->query("SELECT id, email FROM users WHERE id=?", _id);
    if(query.row_count() < 1) {
        throw runtime_error("No user found for ID provided");
    }
    auto results = query.row();
    
    id = _id;
    email_address = string(results["email"]);
}

void user_model::fill_by_email(db::Connection * db, string _email_address){
    auto query = db->query("SELECT id FROM users WHERE email=?", _email_address);
    if(query.row_count() < 1) {
        throw runtime_error("No user found for ID provided");
    }
    auto results = query.row();
    
    id = int(results["id"]);
    email_address = _email_address;
}

void user_model::fill_by_token(db::Connection *db, string token) {
    auto token_query = db->query("SELECT user_id, created, expires FROM sessions WHERE token=? AND expires > UNIX_TIMESTAMP()", token);
    
    if(token_query.row_count() < 1) {
        throw runtime_error("No user by that token");
    }
    
    auto token_results = token_query.row();
    
    id = token_results["user_id"];
    fill_by_id(db, id);
    current_session.created = token_results["created"];
    current_session.expires = token_results["expires"];
    current_session.user_id = token_results["user_id"];
    current_session.token = token;
}

bool user_model::authenticate(db::Connection * db, string email, string pass){
    
    auto query = db->query("SELECT id, email, hash, nacl FROM users WHERE email=?", email);
    LOG(INFO) << "Rows found:" << query.row_count();
    auto results = query.row();
    
    LOG(INFO) << "Building Hash";
    int user_id = int(results["id"]);
    istream * hash_stream = results["hash"];
    unsigned char hash[128];
    hash_stream->read( (char *)&hash[0], 128);
    LOG(INFO) << "Building Salt";
    istream * salt_stream = results["nacl"];
    unsigned char salt[128];
    salt_stream->read( (char *)&salt[0], 128);
    LOG(INFO) << "Building Hash";
    unsigned char * test_hash = (unsigned char *) malloc(sizeof(unsigned char) * 128);
    int success = PKCS5_PBKDF2_HMAC_SHA1(pass.c_str(), pass.length(), salt, 128, 1000, 128, test_hash);
    
    if(success != 1){
        LOG(ERROR) << "Could not generate hash";
        throw std::runtime_error("Could not generate hash on authentication");
        return false;
    }
    LOG(INFO) << "Checking Hash";
    if(strcmp((const char *)&hash[0], (const char *)test_hash) == 0){
        LOG(INFO) << "Successfully Authenticated";
        fill_by_id(db, user_id);
        create_session(db);
        return true;
    }
    
    return false;
}

void user_model::create(db::Connection * db, string email, string password){
    unsigned char * hash = (unsigned char *) malloc(sizeof(unsigned char) * 128);
    unsigned char salt[128];
    RAND_bytes(salt,128);
    
    int success = PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.length(), salt, 128, 1000, 128, hash);
    
    if(success != 1) {
        LOG(ERROR) << "Could not generate hash";
        throw std::runtime_error("Could not generate hash on user creation");
        return;
    }
    
    string hash_str((const char *)hash, 128);
    string salt_str((const char *)salt, 128);
    
    auto query = db->query("INSERT INTO users (email, hash, nacl) VALUES (?, ?, ?)", email, hash_str, salt_str);
    query.execute();
    
    if(!authenticate(db, email, password)) {
        LOG(ERROR) << "User did not pass authentication check after creation";
    }
}

void user_model::refresh_session(db::Connection * db) {
    current_session.refresh(db);
}
