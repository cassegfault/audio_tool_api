//
//  http_connection.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_connection_h
#define http_connection_h

#include <stdio.h>
#include <iostream>
#include <boost/asio/ip/tcp.hpp>

using namespace std;
using tcp = boost::asio::ip::tcp;

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    http_connection(shared_ptr<tcp::socket> _socket): socket(std::move(_socket)), has_socket(true) {};
    http_connection(){ };
    ~http_connection(){
        if(has_socket){
            cout << "socket destructed" << endl;
        }
        if(socket != nullptr)
            socket->close();
    }
    shared_ptr<tcp::socket> socket;
private:
    bool has_socket = false;
};

#endif /* http_connection_hpp */
