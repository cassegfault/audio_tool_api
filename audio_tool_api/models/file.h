//
//  file.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/15/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef file_model_hpp
#define file_model_hpp

#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <vector>
#include "../utilities/database.h"
#include "../utilities/serialize.h"

using namespace std;
using namespace serialize;

class file_model {
public:
    file_model() {};
    file_model(int _id, size_t _size, string _guid, string _name, int _user_id, int _sample_rate): id(_id), size(_size), guid(_guid), name(_name), user_id(_user_id), sample_rate(_sample_rate) {};
    file_model(size_t _size, string _name, int _user_id, int _sample_rate): size(_size), name(_name), user_id(_user_id), sample_rate(_sample_rate) {};
    void create(db::Connection * db);
    
    void fill_by_guid(db::Connection * db, string _guid, bool with_data = false);
    void fill_by_id(db::Connection * db, int _id, bool with_data = false);
    
    int id;
    string data; // Only use this property when necessary
    size_t size;
    string guid;
    string name;
    int sample_rate;
    int user_id;
    
    // Do not send data, return on different endpoint as arraybuffer
    operator nlohmann::json(){
        return jsonify(
                    KVP(id),
                    KVP(size),
                    KVP(guid),
                    KVP(name),
                    KVP(sample_rate),
                    KVP(user_id));
    }
};

vector<file_model> get_files_for_user(db::Connection * db, int user_id);

#endif /* file_hpp */
