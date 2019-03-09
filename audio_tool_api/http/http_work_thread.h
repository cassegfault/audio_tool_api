//
//  http_work_thread.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef http_work_thread_h
#define http_work_thread_h

#include <stdio.h>
#include <thread>
#include <atomic>
#include <glog/logging.h>

using namespace std;

class http_work_thread {
    http_work_thread() : _is_running(false), _thread() {};
    ~http_work_thread() {};
    
    void start();
    void join();
    bool is_running() { return _is_running; }
private:
    atomic<bool> _is_running;
    thread _thread;
};

#endif /* http_work_thread_hpp */
