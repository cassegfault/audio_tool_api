//
//  main.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//
#define NDEBUG
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/lockfree/queue.hpp>

#include <boost/config.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstddef>
#include <memory>
#include <thread>
#include <unordered_set>

#include <csignal>
#include <glog/logging.h>
#include "external/concurrentqueue.h"

#include "http/http_work_thread.h"
#include "http/http_connection.h"
#include "http/http_worker.h"
#include "http/http_server.h"
#include "http/st_http_server.h"
#include "http/tpc_http_server.h"
#include "utilities/stats_client.h"
#include "utilities/config.h"

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
bool is_running = false;
void sighandler(int sig) {
    is_running = false;
}

int main(int argc, char* argv[]) {
    is_running = true;
    signal(SIGINT,sighandler);
    // Logfile generation. Also outputs INFO level logs to stdout
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    google::SetStderrLogging(google::INFO);
    
    load_config("config.json");
    
    setup_stats(config()->statsd_host.c_str(), config()->statsd_port, "audio_api.");
    if(strcmp(config()->server_type.c_str(), "st")==0){
        st_http_server server;
        server.start();
        server.run();
    } else if(strcmp(config()->server_type.c_str(), "tpc")==0){
        tpc_http_server server;
        server.start();
        server.run();
    } else {
        http_server server;
        server.start();
        while (is_running) {
            server.poll();
        }
    }
    
    free_stats();
    
    return 0;
    
}
