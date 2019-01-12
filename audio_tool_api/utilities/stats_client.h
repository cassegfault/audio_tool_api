//
//  stats_client.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/6/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef stats_client_h
#define stats_client_h

#include <netdb.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <glog/logging.h>

using namespace std;

class stats_client {
public:
    stats_client(const char * host, int port, const char * _stat_namespace);
    ~stats_client() {
        close(fd);
        if(message){
            delete [] message;
            message = nullptr;
        }
    }
    
    void send(const char * key, int value, const char * type);
    
    void increment(const char * key);
    void decrement(const char * key);
    void count(const char * key, int val);
    void guage(const char * key, int val);
    void time(const char * key, int ms);
    
private:
    int fd;
    char * message;
    string stat_namespace;
};

void setup_stats(const char * host, int port, const char * _stat_namespace);
void free_stats();
stats_client * stats();

#endif /* stats_client_h */
