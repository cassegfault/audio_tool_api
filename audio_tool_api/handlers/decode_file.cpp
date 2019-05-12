//
//  decode_file.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "decode_file.h"


void DecodeFileHandler::post(http_request & request_data) {
    if(request_data.files.size() == 0) {
        throw http_exception(400, "Must send at least one file to use this endpoint");
    }
    
    AP p;
    p.set_input_content((uint8_t*)request_data.files[0].data(), request_data.files[0].size());
    p.process();
    response.set_content(p.get_output_content());
}
