//
//  main.cpp
//  async_audio_tool_api
//
//  Created by Chris Pauley on 3/8/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "utilities/config.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>


#include <thread>
#include <iostream>
#include <list>

using namespace std;
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;

void accept(/*list<tcp::socket> & sockets, */tcp::acceptor & acceptor) {
    acceptor.async_accept(acceptor.get_executor().context(), [/*&sockets,*/&acceptor](boost::system::error_code &err){
        //sockets.push_back(tcp::socket{acceptor.get_executor().context()});
        accept(acceptor);
    });
}

int main(int argc, const char * argv[]) {
    auto const address = boost::asio::ip::make_address(config()->server_host);
    auto const port = static_cast<unsigned short>(config()->server_port);
    boost::asio::io_context ioc, thread_ioc;
    
    tcp::acceptor acceptor{ioc, {address, port}};
    list<tcp::socket> sockets;
    //boost::lockfree::queue<string> sockets;
    
    accept(/*sockets,*/acceptor);
    
    vector<thread> threads;
    bool run_threads = true;
    for(int x=0; x < 1; x++){
        threads.emplace_back([&](){
            while(run_threads) {
                thread_ioc.poll();
                if (!sockets.empty()) {
                    tcp::socket sock{std::move(sockets.front())};
                    sockets.pop_front();
                    http::request_parser<http::string_body> parser_;
                    boost::beast::flat_static_buffer<8192> buffer;
                    http::async_read(sock, buffer, parser_, [](boost::beast::error_code &err, size_t size){
                        // process, handle, async_write
                    });
                    
                } else {
                    this_thread::sleep_for(chrono::microseconds(1));
                }
            }
        });
    }
    
    
    for(;;){
        ioc.poll();
    }
    
    /* Accept connections, pass those connections into a work pool, threads wait on the work pool, pull in a connection,
     async handle the request (thread level asio context), handle more work
     */
    
    return 0;
}
