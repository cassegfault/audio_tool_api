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

int main(int argc, const char * argv[]) {
    
    shared_ptr<mything> inta(make_shared<mything>(1));
    shared_ptr<mything> intb(inta);
    
    inta = make_shared<mything>(2);
    
    intb->num = 3;
    
    cout << "inta: " << inta->num << endl;
    cout << "intb: " << intb->num << endl;
    
    
    return 0;
}
