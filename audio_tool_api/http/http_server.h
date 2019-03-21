//
//  http_server.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_server_hpp
#define http_server_hpp

#include <stdio.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/optional/optional.hpp>

#include "utilities/config.h"
#include "http/http_work_thread.h"
#include "http/http_connection.h"
#include "external/concurrentqueue.h"

using namespace std;

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_server {
public:
    http_server(): ioc(1) {};
    ~http_server(){};
    
    void start();
    void poll();
    void run();
    
private:
    void accept();
    boost::optional<tcp::acceptor> acceptor;
    shared_ptr<tcp::socket> active_connection;
    boost::asio::io_context ioc;
    
    vector<shared_ptr<tcp::socket> > connections;
    moodycamel::ConcurrentQueue<shared_ptr<tcp::socket>> q;
    vector<http_work_thread> threads;
};

#endif /* http_server_hpp */