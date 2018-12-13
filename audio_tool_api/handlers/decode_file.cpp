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
#include <fstream>



    
void DecodeFileHandler::post(HTTPRequest request_data) {
    AudioProcessor p;
    cout << "Files: " << request_data.files.size() << endl;
    cout << "Files[0] size: " << request_data.files[0].size() << endl;
    
    ofstream f("test.mp3",ios_base::binary);
    //f << request_data.files[0].data();
    f.write(request_data.files[0].data(),request_data.files[0].size());
    f.close();

    //unique_ptr<uint8_t> sptr((uint8_t *)request_data.files[0].c_str());
    p.set_input_content((uint8_t*)request_data.files[0].data(), request_data.files[0].size());
    p.execute();
    //cout << request_data._req.body().substr(0,400);
    response.body = p.get_output_content();
}
