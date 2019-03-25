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
#include <list>
#include <glog/logging.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/bind/bind.hpp>

#include "external/blocking_concurrentqueue.h"
#include "http/http_connection.h"
#include "http/http_worker.h"
#include "utilities/config.h"

using namespace std;
using tcp = boost::asio::ip::tcp;

class http_work_thread {
    using q_type =moodycamel::ConcurrentQueue<shared_ptr<http_connection>>;
public:
    http_work_thread(q_type & q) : _is_running(false), _thread(), _q(q), poll_timer(work_thread_context) {};
    http_work_thread(http_work_thread && other): _is_running(bool(other._is_running)), _thread(std::move(other._thread)), _q(other._q), poll_timer(work_thread_context) {}
    ~http_work_thread() {
        join();
    };
    
    void start();
    void join();
    bool is_running() { return _is_running; }
    int num_workers = 4;
private:
    vector<http_worker> workers;
    void run_loop();
    void thread_runner();
    atomic<bool> _is_running;
    thread _thread;
    q_type & _q;
    boost::asio::io_context work_thread_context{1}; // single threaded io_context (this thread)
    boost::asio::deadline_timer poll_timer;
};

#endif /* http_work_thread_hpp */
