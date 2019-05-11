//
//  routes.h
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef routes_h
#define routes_h

#include "../handlers/base_handler.h"
#include "../handlers/decode_file.h"
#include "../handlers/auth.h"
#include "../handlers/user.h"
#include "../handlers/files.h"
#include "../handlers/project.h"
#include "../handlers/test.h"

#include <unordered_map>
#include <string>

const static std::unordered_map<std::string, std::function<base_handler *(void)> > application_routes = {
    { "/", []() { return new base_handler; } },
    { "/test", []() { return new test_handler; } },
    { "/auth", []() { return new auth_handler; } },
    { "/user", []() { return new user_handler; } },
    { "/file", []() { return new file_handler; } },
    { "/file_data", []() { return new file_data_handler; } },
    { "/files", []() { return new file_list_handler; } },
    { "/project", []() { return new project_handler; } },
    { "/projects", []() { return new project_list_handler; } },
    
    
};

#endif /* routes_h */
