//
//  user.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "user.h"


#define KEY_LEN      32
#define KEK_KEY_LEN  20
#define ITERATION     1

void keygen(string password){
    size_t i;
    unsigned char *out;
    const char pwd[] = "password";
    unsigned char salt_value[] = {'s','a','l','t'};
    
    out = (unsigned char *) malloc(sizeof(unsigned char) * KEK_KEY_LEN);
    
    printf("pass: %s\n", pwd);
    printf("ITERATION: %u\n", ITERATION);
    printf("salt: "); for(i=0;i<sizeof(salt_value);i++) { printf("%02x", salt_value[i]); } printf("\n");
    
    if( PKCS5_PBKDF2_HMAC_SHA1(pwd, strlen(pwd), salt_value, sizeof(salt_value), ITERATION, KEK_KEY_LEN, out) != 0 )
    {
        printf("out: "); for(i=0;i<KEK_KEY_LEN;i++) { printf("%02x", out[i]); } printf("\n");
    }
    else
    {
        fprintf(stderr, "PKCS5_PBKDF2_HMAC_SHA1 failed\n");
    }
    
    free(out);
}

void user::create_session(db::Connection * db){
    for(auto row: db->query("SELECT password FROM users WHERE email=?", string("chrisp3d@gmail.com"))) {
        cout << string(row["password"]) << endl;
    }
    
}

bool user::authenticate(db::Connection * db, string email, string pass){
    auto query = db->query("SELECT id, email, hash, nacl FROM users WHERE email=?", email);
    auto results = query.row();
    istream * hash_stream = results["hash"];
    unsigned char hash[128];
    hash_stream->read( (char *)&hash[0], 128);
    
    istream * salt_stream = results["nacl"];
    unsigned char salt[128];
    salt_stream->read( (char *)&salt[0], 128);
    
    unsigned char * test_hash = (unsigned char *) malloc(sizeof(unsigned char) * 128);
    int error = PKCS5_PBKDF2_HMAC_SHA1(pass.c_str(), pass.length(), salt, 128, 1000, 128, test_hash);
    
    if(error != 1){
        LOG(ERROR) << "Could not generate hash";
    }
    
    if(strcmp((const char *)&hash[0], (const char *)test_hash) == 0){
        cout << "User authenticated: " << email << endl;
    } else {
        LOG(ERROR) << "Did not authenticate";
    }
    
    
    
    return false;
}

void user::create(db::Connection * db, string email, string password){
    unsigned char * hash = (unsigned char *) malloc(sizeof(unsigned char) * 128);
    unsigned char salt[128];
    RAND_bytes(salt,128);
    
    int error = PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.length(), salt, 128, 1000, 128, hash);
    
    if(error != 1) {
        LOG(ERROR) << "Could not generate hash";
        return;
    }
    
    string hash_str((const char *)hash, 128);
    string salt_str((const char *)salt, 128);
    auto query = db->query("INSERT INTO users (email, hash, nacl) VALUES (?, ?, ?)", email, hash_str, salt_str);
    query.execute();
    cout << "Created User" << endl;
    authenticate(db, email, password);
}
