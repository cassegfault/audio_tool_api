//
//  managed_thread.h
//  audio_tool_api
//
//  Created by Chris Pauley on 3/28/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef managed_thread_h
#define managed_thread_h
#include <atomic>

class managed_thread {
public:
    managed_thread(std::function<void()> _runner): runner(_runner) {};
    
    void start(){
        is_running = true;
        t = new thread([](){
            runner();
        });
        
        is_running = false;
    }
private:
    std::function<void()> runner;
    atomic<bool> is_running;
    thread t;
}

#endif /* managed_thread_h */
