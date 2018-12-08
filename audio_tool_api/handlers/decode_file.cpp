//
//  decode_file.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "decode_file.h"
#include "../audio/processor.h"
#include <iostream>



    
void DecodeFileHandler::post(HTTPRequest request_data) {
    AudioProcessor p;
    p.set_input_content((uint8_t *)request_data._req.body().data(), request_data._req.body().size());
    p.execute();
    
    response.body = p.get_output_content();
    
}
