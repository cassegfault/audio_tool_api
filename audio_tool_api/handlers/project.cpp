//
//  project.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "project.h"

void project_handler::get(http_request request_data) {
    if(!request_data.has_param("guid")) {
        throw http_exception(400, "Must provide GUID of project");
    }
    
    response_data.fill_by_guid(db, request_data.url_params["guid"]);
}

void project_handler::post(http_request request_data) {
    if(request_data.json.find("guid") == request_data.json.end()) {
        cout << request_data.json.dump() << endl;
        throw http_exception(400, "Must provide GUID of project");
    }
    if(request_data.json.find("name") == request_data.json.end()) {
        throw http_exception(400, "Must provide name of project");
    }
    project_model project;
    project.fill_by_guid(db, request_data.json["guid"]);
    project.name = request_data.json["name"];
    project.project_data = request_data.json["project_data"];
    project.update(db);
    response_data = project;
}

void project_handler::put(http_request request_data) {
    if(request_data.json.find("name") == request_data.json.end()) {
        throw http_exception(400, "Must provide name of project");
    }
    if(request_data.json.find("creator_id") == request_data.json.end()) {
        throw http_exception(400, "Must provide creator_id of project");
    }
    
    int creator_id = request_data.json["creator_id"];
    string project_data = "{}";
    if(request_data.json.find("project_data") != request_data.json.end()) {
        project_data = request_data.json["project_data"];
    }
    project_model project(request_data.json["name"], project_data, creator_id);
    project.create(db);
    response_data = project;
}

void project_list_handler::get(http_request request_data) {
    vector<project_model> projects = get_projects_for_user(db, user.id);
    response_data.swap(projects);
}
