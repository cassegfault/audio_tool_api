//
//  timer.h
//  audio_tool_api
//
//  Created by Chris Pauley on 1/6/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef timer_h
#define timer_h

#include <chrono>
using namespace std;
using namespace std::chrono;
class timer {
public:
    timer() {
        start();
    }
    steady_clock::time_point start_time;
    steady_clock::time_point end_time;
    bool is_running = false;
    
    void start() {
        start_time = steady_clock::now();
        is_running = true;
    }
    
    void stop() {
        end_time = steady_clock::now();
        is_running = false;
    }
    
    double duration() {
        if (is_running){
            stop();
        }
        
        chrono::duration<double> span = end_time - start_time;
        return span.count();
    }
    
    double microseconds() {
        if (is_running) {
            stop();
        }
        auto span = duration_cast<std::chrono::microseconds>(end_time - start_time);
        return span.count();
    }
    
    double milliseconds() {
        if (is_running) {
            stop();
        }
        auto span = duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return span.count();
    }
};

#endif /* timer_h */
