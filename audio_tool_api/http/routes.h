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

#include <unordered_map>
#include <string>

const static std::unordered_map<std::string, std::function<BaseHandler *(void)> > application_routes = {
    { "/", []() { return new BaseHandler; } },
    { "/decode", []() { return new DecodeFileHandler; } },
};

#endif /* routes_h */
