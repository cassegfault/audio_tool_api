//
//  stats_client.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/6/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "stats_client.h"

stats_client * global_stats = nullptr;

stats_client::stats_client(const char * host, int port, const char * _stat_namespace): stat_namespace(_stat_namespace) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG;
    
    string portstr = to_string(port);
    
    struct addrinfo * res = 0;
    int error = getaddrinfo(host, portstr.c_str(), &hints, &res);
    if(error) {
        LOG(ERROR) << "Could not get address info for stats host";
        return;
    }
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        LOG(ERROR) << "Could not create socket connection to stats host";
        return;
    }
    cout << "Connecting to statsd" << endl;
    connect(fd, res->ai_addr, res->ai_addrlen);
    
    message = new char[4096];
    memset(message, 0, 4096);
    
    freeaddrinfo(res);
}

void stats_client::count(const char * key, int value) {
    send(key,value,"c");
}
void stats_client::guage(const char * key, int value) {
    send(key,value,"g");
}
void stats_client::time(const char * key, int value) {
    send(key,value,"ms");
}
void stats_client::increment(const char * key) {
    count(key, 1);
}
void stats_client::decrement(const char * key) {
    count(key, -1);
}

void stats_client::send(const char * key, int value, const char * type) {
    
    sprintf(message, "%s%s:%d|%s", stat_namespace.c_str(), key, value, type);
    cout << message << endl;
    size_t error = ::send(fd, message, strlen(message), 0);
    if (error < 0) {
        cerr << "Failed sending stat" << endl;
    }
}

void setup_stats(const char * host, int port, const char * _stat_namespace){
    if(!global_stats)
        global_stats = new stats_client(host, port, _stat_namespace);
}

void free_stats(){
    if (global_stats) {
        delete global_stats;
        global_stats = nullptr;
    }
}

stats_client * stats() {
    return global_stats;
}
