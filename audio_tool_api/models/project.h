//
//  project.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef project_model_hpp
#define project_model_hpp

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

class project_model {
public:
    project_model(){};
    project_model(string _name, string _project_data, int _creator_id): name(_name), project_data(_project_data), creator_id(_creator_id) {};
    project_model(int _id, string _guid, string _name, string _project_data, int _creator_id, bool _is_deleted): id(_id), guid(_guid), name(_name), project_data(_project_data), creator_id(_creator_id), is_deleted(_is_deleted) {};
    
    int id;
    int creator_id;
    bool is_deleted;
    string guid;
    string name;
    string project_data;
    
    void fill_by_guid(db::Connection * db, string _guid);
    void update(db::Connection * db);
    void create(db::Connection * db);
    
    operator nlohmann::json(){
        return serialize::jsonify(
                                  KVP(guid),
                                  KVP(creator_id),
                                  KVP(is_deleted),
                                  KVP(name),
                                  KVP(project_data));
    }
};

vector<project_model> get_projects_for_user(db::Connection * db, int user_id, bool with_data = false);


#endif /* project_hpp */
