//
//  http_work_thread.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_work_thread.h"

void http_work_thread::start() {
    
    _thread = thread(&http_work_thread::thread_runner, this);
    //thread_runner();
}

void http_work_thread::thread_runner(){
    _is_running = true;
    num_workers = config()->num_workers;
    for(int x = 0; x < num_workers; x++){
        workers.emplace_back(work_thread_context);
    }
    //run_loop();
    accept_loop();
    work_thread_context.run();
}

void http_work_thread::accept(){
    boost::system::error_code err;
    atomic<int>counter;
    shared_ptr<http_connection> conn2 = make_shared<http_connection>(work_thread_context, counter);
    tcp::socket sock (work_thread_context);
    //acceptor->accept(sock,err);
    if(!err){
        //accepted
        LOG(INFO) << "new accept works maybe";
    } else {
        LOG(ERROR) << err.message();
    }
}
void http_work_thread::accept_loop() {
    if (_is_running) {
        dequeue();
        // Allow other asynchronous work to run
        poll_timer.cancel();
        poll_timer.expires_from_now(boost::posix_time::millisec(1));
        poll_timer.async_wait(boost::bind(&http_work_thread::accept_loop,this));
    }
}

void http_work_thread::run_loop(){
    
    vector<http_worker>::iterator worker_it;
    
    // don't do this every time
    bool have_worker = false;
    for (worker_it = workers.begin(); worker_it != workers.end(); worker_it++) {
        if(worker_it->has_finished()){
            have_worker = true;
            break;
        }
    }
    
    // should we try and dequeue here? What if we're hoarding connections? possibly
    if(_is_running && worker_it != workers.end()){
        // we have a worker, check if there is anything to dequeue
        if(_q.try_dequeue(*conn)){
            worker_it->start(std::move(*conn));
        }
    }
    
    // Continuously feed our timer unless the thread has been shut down. This keeps the io_context (and the thread) open
    if(_is_running){
        // Allow other asynchronous work to run
        poll_timer.cancel();
        poll_timer.expires_from_now(boost::posix_time::millisec(2));
        poll_timer.async_wait(boost::bind(&http_work_thread::run_loop,this));
    }
}
void http_work_thread::join() {
    _is_running = false;
    if(_thread.joinable()) {
        _thread.join();
    }
}
