//
//  files.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/16/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "files.h"

void file_handler::get(http_request & request_data) {
    if(request_data.json.find("guid") == request_data.json.end()) {
        throw http_exception(400, "Must provide GUID of file");
    }
    string guid = request_data.json["guid"];
    file_model f;
    f.fill_by_guid(db, guid);
    response_data = f;
}

void file_handler::post(http_request & request_data) {
    if(!request_data.has_param("name")) {
        throw http_exception(400, "Must include name in URL parameters");
    }
    if(request_data.files.size() == 0) {
        throw http_exception(400, "Must send at least one file to use this endpoint");
    }
    AP p;
    p.set_input_content((uint8_t*)request_data.files[0].data(), request_data.files[0].size());
    p.process();
    
    string data = p.get_output_content();
    string name = request_data.url_params.at("name");

    file_model f(data.size(), name, user.id, p.get_output_samplerate());
    f.data = data;
    try {
        f.create(db);
    } catch (runtime_error e) {
        LOG(INFO) << e.what();
    }
    response_data = f;
}

void file_data_handler::get(http_request & request_data) {
    if(!request_data.has_param("guid")) {
        throw http_exception(400, "Must include guid in URL parameters");
    }
    file_model f;
    f.fill_by_guid(db, request_data.url_params.at("guid"), true);
    response.set_content(f.data);
    response.content_type = "application/octet";
}

void file_list_handler::get(http_request & request_data) {
    vector<file_model> files = get_files_for_user(db, user.id);
    response_data.swap(files);
}
