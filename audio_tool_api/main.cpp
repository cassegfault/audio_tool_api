//
//  main.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/1/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>
#include "http/http_worker.h"

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
int main(int argc, char* argv[]) {
    
    auto const address = boost::asio::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(9090);
    auto const num_workers = 4;
    boost::asio::io_context ioc{1};
    tcp::acceptor acceptor{ioc, {address, port}};
    
    std::list<HTTPWorker> workers;
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(acceptor);
        workers.back().start();
    }
    

    // For the sake of development, run blocking
    /*while (true) {
        ioc.poll();
    }*/
    ioc.run();
    
    return 0;
    
}
