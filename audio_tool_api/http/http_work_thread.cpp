//
//  http_work_thread.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 3/9/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "http_work_thread.h"

void http_work_thread::start() {
    _is_running = true;
    for(int x = 0; x < num_workers; x++){
        workers.emplace_back(work_thread_context);
    }
    _thread = thread(&http_work_thread::thread_runner, this);
}

void http_work_thread::thread_runner(){
    run_loop();
    work_thread_context.run();
}

void http_work_thread::run_loop(){
    http_connection conn;
    
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
    bool did_connect = false;
    if(_q.try_dequeue(conn)){
        did_connect = true;
        cout << have_worker <<endl;
    }
    if(_is_running && worker_it != workers.end() && did_connect){
        // we have a connection, move it into ownership of this thread
        connections.emplace_back(std::move(conn));
        worker_it->start(connections.back());
    }
    
    // Continuously feed our timer unless the thread has been shut down. This keeps the io_context (and the thread) open
    if(_is_running){
        // Allow other asynchronous work to run
        poll_timer.cancel();
        poll_timer.expires_from_now(boost::posix_time::millisec(1));
        poll_timer.async_wait(boost::bind(&http_work_thread::run_loop,this));
    } else {
        cout << "thread shut down" <<endl;
    }
}
void http_work_thread::join() {
    _is_running = false;
    if(_thread.joinable()) {
        _thread.join();
    }
}
