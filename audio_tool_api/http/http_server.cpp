//
//  http_server.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_server.h"

void http_server::start(){
    auto const address = boost::asio::ip::make_address(config()->server_host);
    unsigned short const port = static_cast<unsigned short>(config()->server_port);
    tcp::endpoint endpoint(address,port);
    acceptor.emplace(ioc, endpoint);
    
    int num_threads = std::thread::hardware_concurrency();
    for (int x = 0; x < num_threads; x++) {
        threads.emplace_back(q);
    }
    
    LOG(INFO) << "Listening on " << config()->server_host << ':' << config()->server_port;
    accept();
    
    for(auto & t : threads) {
        t.start();
    }
}

void http_server::accept(){
    connections.emplace_back(new tcp::socket(acceptor->get_executor().context()));
    shared_ptr<tcp::socket> _sock = connections.back();
    acceptor->async_accept(*_sock, [this](boost::beast::error_code err){
        if(err){
            
        } else {
            http_connection conn(std::move(connections.back()));
            q.enqueue(conn);
        }
        accept();
    });
}

void http_server::poll(){
    ioc.poll();
}

void http_server::run(){
    ioc.run();
}
