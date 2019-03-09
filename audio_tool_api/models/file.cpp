//
//  file.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/15/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "file.h"

void file_model::create(db::Connection * db){
    auto query = db->query("INSERT INTO files (guid, name, data, size, created, user_id, sample_rate) VALUES (UUID(), ?, ?, ?, UNIX_TIMESTAMP(), ?, ?)",
                           name,
                           data,
                           size,
                           user_id,
                           sample_rate);
    query.execute();
    auto confirmation_query = db->raw_query("SELECT id, guid FROM files WHERE id=LAST_INSERT_ID()");
    if (confirmation_query.row_count() < 1){
        throw runtime_error("Failed to confirm file insert");
    }
    
    auto results = confirmation_query.row();
    id = results["id"];
    guid = string(results["guid"]);
}

void file_model::fill_by_guid(db::Connection * db, string _guid, bool with_data) {
    string query_string;
    if(with_data) {
        query_string = "SELECT id, guid, name, size, user_id, data, sample_rate FROM files WHERE guid=?";
    } else {
        query_string = "SELECT id, guid, name, size, user_id, sample_rate FROM files WHERE guid=?";
    }
    auto query = db->query(query_string, _guid);
    if (query.row_count() < 1) {
        throw runtime_error("No file by this guid");
    }
    
    auto results = query.row();
    id = results["id"];
    guid = _guid;
    size = results["size"];
    user_id = results["user_id"];
    sample_rate = results["sample_rate"];
    if(with_data) {
        data = string(results["data"]);
    }
}

void file_model::fill_by_id(db::Connection * db, int _id, bool with_data) {
    string query_string;
    if(with_data) {
        query_string = "SELECT id, guid, name, size, user_id, data, sample_rate FROM files WHERE id=?";
    } else {
        query_string = "SELECT id, guid, name, size, user_id, sample_rate FROM files WHERE id=?";
    }
    auto query = db->query(query_string, _id);
    if (query.row_count() < 1) {
        throw runtime_error("No file by this guid");
    }
    
    auto results = query.row();
    id = _id;
    guid = string(results["guid"]);
    size = results["size"];
    user_id = results["user_id"];
    sample_rate = results["sample_rate"];
    if(with_data) {
        data = string(results["data"]);
    }
}

vector<file_model> get_files_for_user(db::Connection * db, int user_id){
    vector<file_model> files;
    auto query = db->query("SELECT id, guid, name, size, user_id, sample_rate FROM files WHERE user_id=? AND is_deleted=0", user_id);
    
    if (query.row_count() > 0) {
        for(auto row : query){
            files.emplace_back(row["id"], row["size"], row["guid"], row["name"], row["user_id"], row["sample_rate"]);
        }
    }
    
    return files;
}
