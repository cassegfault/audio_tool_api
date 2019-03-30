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
    //acceptor = std::make_shared<tcp::acceptor>(*ioc, endpoint);
    is_running = true;
    
    for (int x = 0; x < config()->num_threads; x++) {
        threads.emplace_back(q, acceptor);
    }
    
    LOG(INFO) << "Listening on " << config()->server_host << ':' << config()->server_port;
    
    for(auto & t : threads) {
        t.start();
    }
    /*while(is_running){
        raw_accept();
        //ioc.run();
     }*/
    raw_accept();
}

void http_server::accept(){
    
    active_connection = make_shared<tcp::socket>(acceptor->get_io_context());
    conn = make_shared<http_connection>(acceptor->get_io_context());
    // This assumes a maximum FD_SETSIZE of 1024 which is standard on most machines
    auto num = q.size_approx();
    DLOG_IF(INFO, num > 5) << "Approx Connections: " << q.size_approx();
    if(q.size_approx() > 1000){
        //this_thread::sleep_for(chrono::milliseconds(1));
        LOG(WARNING) << "Too Many connections in queue";
        return accept();
    }
    
    //acceptor->async_accept(*active_connection, [this](boost::beast::error_code err){
    //acceptor->async_accept(conn->socket, [this](boost::beast::error_code err){
    boost::system::error_code err;
    acceptor->accept(conn->socket, err);
        if(err){
            LOG(ERROR) << "error accepting: " << err.message();
            stats()->increment("accept_errors");
            boost::asio::io_context::strand s(*ioc);
            conn->socket.shutdown(tcp::socket::shutdown_both,err);
            s.wrap([this](){
                conn->socket.close();
            });
        } else {
            conn->accepted_time = chrono::steady_clock::now();
            auto diff = conn->accepted_time - conn->created_time;
            q.enqueue(std::move(conn));
            if(chrono::duration_cast<chrono::milliseconds>(diff).count() > 10000){
                LOG(ERROR) << "Connect Timeout";
            } else {
                LOG(INFO) << "Duration: " << chrono::duration_cast<chrono::milliseconds>(diff).count();
            }
        }
        //accept();
    //});
}
void http_server::raw_accept(){
    int server_fd, sock;
    int opt = 1;
    socklen_t s = sizeof(opt);
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        LOG(ERROR) << "Error Creating server file descriptor: " << std::strerror(errno);
        return;
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, s)){
        LOG(ERROR) << "Error setting socket options: " << std::strerror(errno);
        exit(1);
        return;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( config()->server_port );
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOG(ERROR) << "Error binding to port: " << std::strerror(errno);
        return;
    }
    if (listen(server_fd, 3) < 0) {
        LOG(ERROR) << "Error listening on port: " << std::strerror(errno);
        return;
    }
    while(is_running) {
        if((sock = ::accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            LOG(ERROR) << "Error accepting: " << std::strerror(errno);
            return;
        }
        vector<http_work_thread>::iterator tt = threads.end();
        double min_load = -1.0;
        for(vector<http_work_thread>::iterator t = threads.begin(); t < threads.end(); t++){
            double thread_load = t->load_factor();
            if(thread_load < 0.2){
                tt = t;
                break;
            } else if (thread_load < min_load || min_load < 0.0) {
                min_load = thread_load;
                tt = t;
            } else if (tt == threads.end()){
                // make sure it will get enqueued somewhere
                tt = t;
            }
        }
        if (tt == threads.end()) {
            LOG(ERROR) << "All threads overloaded";
            return;
        }
        conn = make_shared<http_connection>(*tt->io_context());
        conn->socket.assign(boost::asio::ip::tcp::v4(), sock);
        conn->accepted_time = chrono::steady_clock::now();
        tt->enqueue(conn);
        // tell the thread to handle the connection
    }
    
    
}

void http_server::poll(){
    ioc->poll();
}

void http_server::run(){
    ioc->run();
}
