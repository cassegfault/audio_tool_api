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
    
    active_connection = make_shared<tcp::socket>(acceptor->get_io_context());
    // This assumes a maximum FD_SETSIZE of 1024 which is standard on most machines
    DLOG_EVERY_N(INFO, 50) << "Approx Connections: " << q.size_approx();
    if(q.size_approx() > 1000){
        //this_thread::sleep_for(chrono::milliseconds(1));
        DLOG(INFO) << "Too Many connections in queue";
        return accept();
    }
    
    acceptor->async_accept(*active_connection, [this](boost::beast::error_code err){
        if(err){
            LOG(ERROR) << "error accepting: " << err.message();
        } else {
            // https://stackoverflow.com/questions/43830917/boost-asio-async-reading-and-writing-to-socket-using-queue
            // create the socket inplace with the http_connection, use shared_ptr<http_connection>
            // list to queue connections,
            //shared_ptr<tcp::socket> socket_ptr(std::make_shared<tcp::socket>(*active_connection));
            
            //connections.push_back(active_connection);
            // active connection is getting changed before we're able to handle a response from it
            shared_ptr<tcp::socket> new_shared(active_connection);
            connections.emplace_back(new_shared);
            q.enqueue(connections.back());
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
