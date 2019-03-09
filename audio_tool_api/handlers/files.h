//
//  files.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/16/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef files_handler_hpp
#define files_handler_hpp

#include <vector>
#include "base_handler.h"
#include "../audio/audio_processor.h"
#include "../models/file.h"

class file_list : public serialize::json_list<file_model> {};

class file_handler : public base_model_handler<file_model> {
public:
    void get(http_request request_data) override;
    void post(http_request request_data) override;
};

class file_data_handler: public base_handler {
public:
    void get(http_request request_data) override;
};

class file_list_handler : public base_model_handler<file_list> {
public:
    void get(http_request request_data) override;
};

#endif /* files_hpp */
