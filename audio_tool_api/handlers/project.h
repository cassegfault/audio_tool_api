//
//  project.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef project_handler_h
#define project_handler_h

#include <vector>
#include "base_handler.h"
#include "../audio/audio_processor.h"
#include "../models/project.h"

class project_list : public serialize::json_list<project_model> {};

class project_handler : public base_model_handler<project_model> {
public:
    void get(http_request &request_data) override;
    void post(http_request &request_data) override;
    void put(http_request &request_data) override;
};

class project_list_handler : public base_model_handler<project_list> {
public:
    void get(http_request & request_data) override;
};

#endif /* project_hpp */
