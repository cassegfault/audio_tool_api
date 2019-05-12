//
//  decode_file.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/2/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef decode_file_h
#define decode_file_h

#include <stdio.h>
#include "base_handler.h"
#include "../audio/audio_processor.h"
#include "../models/file.h"
#include <iostream>
#include <fstream>

class DecodeFileHandler : public base_handler {
    void post(http_request & request_data) override;
};



#endif /* decode_file_h */
