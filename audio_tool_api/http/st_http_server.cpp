//
//  st_http_server.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "st_http_server.h"

void st_http_server::start(){
    auto const address = boost::asio::ip::make_address(config()->server_host);
    auto const port = static_cast<unsigned short>(config()->server_port);
    acceptor.emplace(ioc, tcp::endpoint(address,port));
    
    for(int x=0; x < 1; x++){
        threads.emplace_back([this](){
            std::list<st_http_worker> workers;
            for (int i = 0; i < 4; ++i) {
                workers.emplace_back(*acceptor);
                workers.back().start();
            }
            while(is_running) {
                ioc.poll();
                this_thread::sleep_for(chrono::microseconds(1));
            }
        });
    }
}
