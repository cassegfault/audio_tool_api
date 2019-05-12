//
//  project.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "project.h"

void project_model::fill_by_guid(db::Connection * db, string _guid) {
    auto query = db->query("SELECT id, name, project_data, creator_id, is_deleted FROM projects WHERE guid=?", _guid);
    if (query.row_count() < 1) {
        throw runtime_error("No project by this guid");
    }
    
    auto results = query.row();
    id = results["id"];
    guid = _guid;
    name = string(results["name"]);
    project_data = string(results["project_data"]);
    creator_id = results["creator_id"];
    is_deleted = results["is_deleted"];
    
}

void project_model::update(db::Connection *db) {
    auto query = db->query("UPDATE projects SET name=?, project_data=?, is_deleted=? WHERE guid=?", name, project_data, is_deleted, guid);
    query.execute();
    
}

void project_model::create(db::Connection *db) {
    auto query = db->query("INSERT INTO projects (guid, creator_id, name, project_data) VALUES (UUID(), ?, ?, ?)", name, project_data, is_deleted);
    query.execute();
    auto confirmation_query = db->query("SELECT guid FROM projects WHERE id=LAST_INSERT_ID()");
    if (confirmation_query.row_count() < 1) {
        LOG(ERROR) << "Confirmation query failed when creating project";
        throw runtime_error("Project failed to create");
    }
}

vector<project_model> get_projects_for_user(db::Connection * db, int user_id, bool with_data){
    vector<project_model> projects;
    string query_string;
    if (with_data)
        query_string = "SELECT id, guid, name, project_data, creator_id, is_deleted FROM projects WHERE creator_id=?";
    else
        query_string = "SELECT id, guid, name, creator_id, is_deleted FROM projects WHERE creator_id=?";
    
    auto query = db->query(query_string, user_id);
    
    if (query.row_count() > 0) {
        for(auto results : query){
            string project_data = "";
            if(with_data)
                project_data = string(results["project_data"]);
            
            projects.emplace_back(results["id"],
                                  results["guid"],
                                  results["name"],
                                  project_data,
                                  results["creator_id"],
                                  results["is_deleted"]);
        }
    }
    return projects;
}
