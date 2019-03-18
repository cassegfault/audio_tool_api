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
    tcp::socket sock{acceptor->get_executor().context()};
    acceptor->async_accept(sock, [this](boost::beast::error_code err){
        if(err){
            
        } else {
            // https://stackoverflow.com/questions/43830917/boost-asio-async-reading-and-writing-to-socket-using-queue
            // create the socket inplace with the http_connection, use shared_ptr<http_connection>
            // list to queue connections,
            shared_ptr<tcp::socket> socket_ptr(std::make_shared<tcp::socket>(acceptor->get_executor().context()));
            connections.push_back(socket_ptr);
            shared_ptr<http_connection> conn = make_shared<http_connection>(socket_ptr);
            conn_objs.push_back(conn->get_ptr());
            q.enqueue(conn->get_ptr());
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
