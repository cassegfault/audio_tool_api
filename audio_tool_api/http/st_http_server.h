//
//  st_http_server.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/19/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef st_http_server_hpp
#define st_http_server_hpp

#include <stdio.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/optional/optional.hpp>

#include "utilities/config.h"
#include "http/http_work_thread.h"
#include "http/http_connection.h"
#include "external/concurrentqueue.h"
#include "http/st_http_worker.h"

using namespace std;

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class st_http_server {
public:
    st_http_server(): ioc(1), is_running(false) {};
    ~st_http_server(){};
    
    void start();
    void run() {
        is_running = true;
        while (is_running) {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        for(auto & t : threads) {
            t.join();
        }
    }
    
private:
    boost::optional<tcp::acceptor> acceptor;
    shared_ptr<tcp::socket> active_connection;
    boost::asio::io_context ioc;
    
    vector<shared_ptr<tcp::socket> > connections;
    moodycamel::ConcurrentQueue<shared_ptr<tcp::socket>> q;
    vector<thread> threads;
    bool is_running;
};
#endif /* st_http_server_hpp */
