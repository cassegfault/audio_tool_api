//
//  config.h
//  audio_tool_api
//
//  Created by Chris Pauley on 1/8/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef config_h
#define config_h

#include <string>
#include <iostream>
#include <glog/logging.h>
#include "json.hpp"

using namespace nlohmann;
using namespace std;

class config_holder {
public:
    config_holder(string filename);
    
    string server_host;
    int server_port;
    
    string statsd_host;
    int statsd_port;
    
    string db_connection_string; // Built from db_host, db_port
    string db_database;
    string db_user;
    string db_password;
    
    string google_auth_secret;
    string google_auth_client_id;
    
    // For testing server setups
    string server_type;
    int num_workers;
    
private:
    string db_host = "";
    int db_port = 0;
};

void load_config(string config_filename);
config_holder * config();

#endif /* config_h */
