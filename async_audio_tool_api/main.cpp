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
#include <boost/bind.hpp>

#include "external/concurrentqueue.h"

#include <thread>
#include <iostream>
#include <list>

using namespace std;
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;

class mything {
public:
    mything(int _num): num(_num) { cout << "Constructor" << endl; };
    ~mything() { cout << "Destructor" << endl; }
    int num;
};

void somefunc(boost::asio::io_context * ioc){
    ioc->poll();
}
string make_daytime_string(){
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, const char * argv[]) {
    
    boost::asio::io_context ioc1, ioc2;
    tcp::acceptor acc(ioc1, tcp::endpoint(tcp::v4(), 9091));
    tcp::socket socket(ioc2);
    thread t(boost::bind(&somefunc,&ioc2));
    acceptor.accept(socket);
    
    std::string message = make_daytime_string();
        
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
    
    for (;;)
    {
        ioc2.poll();
    }
    
    
    return 0;
}
