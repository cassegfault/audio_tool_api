//
//  tpc_http_server.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/20/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "tpc_http_server.h"

void st_http_server::start(){
    auto const address = boost::asio::ip::make_address(config()->server_host);
    auto const port = static_cast<unsigned short>(config()->server_port);
    acceptor.emplace(ioc, tcp::endpoint(address,port));
    
    for(int x=0; x < std::thread::hardware_concurrency(); x++){
        threads.emplace_back([this](){
            st_http_worker worker(*acceptor);
            worker.start_sync();
        });
    }
}
