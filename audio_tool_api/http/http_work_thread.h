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
#include <queue>
#include <atomic>
#include <mutex>
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
    http_work_thread(q_type & q, shared_ptr<tcp::acceptor> _acceptor) : _is_running(false), _thread(), _q(q), poll_timer(work_thread_context), acceptor(_acceptor) {};
    http_work_thread(http_work_thread && other): _is_running(bool(other._is_running)), _thread(std::move(other._thread)), _q(other._q), poll_timer(work_thread_context) {}
    ~http_work_thread() {
        join();
    };
    
    void start();
    void join();
    bool is_running() { return _is_running; }
    int num_workers = 4;
    shared_ptr<http_connection> * conn = new shared_ptr<http_connection>();
    double load_factor(){
        double load = (double)_queue.size();
        for(auto & w : workers) {
            if(w.is_running()) {
                load += 1.0;
            }
        }
        return load / double(num_workers);
    };
    boost::asio::io_context * io_context(){
        return &work_thread_context;
    }
    
    
    void enqueue(shared_ptr<http_connection> & conn) {
        /*vector<http_worker>::iterator worker_it;
        for (worker_it = workers.begin(); worker_it != workers.end(); worker_it++) {
            if(worker_it->has_finished())
                break;
        }
        
        if(_is_running && worker_it != workers.end()){
            worker_it->start(conn);
        }*/
        _queue.push(conn);
        //queue.enqueue(conn);
    }
    void dequeue(){
        /*if(!queue.try_dequeue(*conn)){
            return;
        }*/
        if(_queue.empty())
            return;
        auto conn = _queue.front();
        
        vector<http_worker>::iterator worker_it;
        for (worker_it = workers.begin(); worker_it != workers.end(); worker_it++) {
            if(worker_it->has_finished())
                break;
        }
        
        if(_is_running && worker_it != workers.end()){
            worker_it->start(conn);
            _queue.pop();
        }
    }
private:
    shared_ptr<tcp::acceptor> acceptor;
    vector<http_worker> workers;
    queue<shared_ptr<http_connection>> _queue;
    void run_loop();
    void accept_loop();
    void accept();
    void thread_runner();
    mutex m;
    atomic<bool> _is_running;
    thread _thread;
    q_type & _q;
    boost::asio::io_context work_thread_context{1}; // single threaded io_context (this thread)
    boost::asio::deadline_timer poll_timer;
};

#endif /* http_work_thread_hpp */
