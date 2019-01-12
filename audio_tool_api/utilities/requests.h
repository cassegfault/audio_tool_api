//
//  requests.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef requests_hpp
#define requests_hpp

#include <stdio.h>
#include <string>
#include <glog/logging.h>
#include <curl/curl.h>

using namespace std;

namespace Requests {
    
    static string url_encode(const string &value) {
        ostringstream escaped;
        escaped.fill('0');
        escaped << hex;
        
        for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            string::value_type c = (*i);
            
            // Keep alphanumeric and other accepted characters intact
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }
            
            // Any other characters are percent-encoded
            escaped << uppercase;
            escaped << '%' << setw(2) << int((unsigned char) c);
            escaped << nouppercase;
        }
        
        return escaped.str();
    }
}


#endif /* requests_hpp */
