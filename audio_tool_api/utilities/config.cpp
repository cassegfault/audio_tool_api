//
//  config.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/8/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "config.h"
#include <fstream>

config_holder * global_config = nullptr;

config_holder::config_holder(string filename){
    std::ifstream json_file(filename.c_str());
    json j = json::parse(json_file);
    
    if (!j.is_object()){
        LOG(FATAL) << "Incorrect JSON format, root should be object";
    }
    
    for (auto& el : j.items()) {
        string key = el.key();
        //auto value = el.value();
        cout << key << ": "<< j[key] << " - " << j[key].type_name() << endl;
        if (strcmp(key.c_str(),"server_host") == 0) {
            server_host = j[key];
        } else if (strcmp(key.c_str(),"server_port") == 0) {
            
                server_port  = el.value();
            
        } else if (strcmp(key.c_str(),"statsd_host") == 0) {
            statsd_host = el.value();
        } else if (strcmp(key.c_str(),"statsd_port") == 0) {
            
                statsd_port = el.value();
            
        } else if (strcmp(key.c_str(),"db_host") == 0) {
            db_host = j[key];
        } else if (strcmp(key.c_str(),"db_port") == 0) {
            
                db_port = el.value();
            
        } else if (strcmp(key.c_str(),"db_user") == 0) {
            db_user = el.value();
        } else if (strcmp(key.c_str(),"db_password") == 0) {
            db_password = el.value();
        } else if (strcmp(key.c_str(),"db_database") == 0) {
            db_database = el.value();
        } else if (strcmp(key.c_str(),"google_auth_client_id") == 0) {
            google_auth_client_id = el.value();
        } else if (strcmp(key.c_str(),"google_auth_secret") == 0) {
            google_auth_secret = el.value();
        } else {
            DLOG(INFO) << "Unknown config parameter: " << key << " : ";
        }
    }
    
    if (db_host.length() == 0 || db_port == 0 || db_database.length() == 0) {
        LOG(FATAL) << "Config did not include database connection parameters";
    }
    
    db_connection_string = "tcp://" + db_host + ':' + to_string(db_port);
};

void load_config(string config_filename) {
    global_config = new config_holder(config_filename);
};

config_holder * config() {
    return global_config;
}
