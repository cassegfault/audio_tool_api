//
//  database.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#include "database.h"
namespace db{
    mutex m;
    Connection::Connection(string _connection_string, string _user, string _password, string _database): _is_open(false), sql(nullptr), connection_string(_connection_string), user(_user), password(_password), database(_database) {
    }
    
    void Connection::open(){
        // @tag database abstraction
        m.lock();
            sql::Driver *driver;
            driver = get_driver_instance();
            sql = driver->connect(connection_string, user, password);
            sql->setSchema(database);
        m.unlock();
        this->_is_open = true;
    }
    
    void Connection::close(){
        if(is_open()){
            delete this->sql;
            sql = nullptr;
            _is_open = false;
        }
    }
    
    Query Connection::raw_query(const char * query_string) {
        if(!_is_open)
            open();
        Query q(this, query_string);
        return q;
    }
    
    Connection::~Connection(){
        close();
    }
    
    /*
     Query Class
    */
    Query::Query(db::Connection * _conn, const char * query): conn(_conn), results(nullptr) {
        stmt = conn->sql->prepareStatement(query);
    };
    
    Query::~Query() {
        stmt->close();
        delete stmt;
        delete results;
        stmt = nullptr;
        results = nullptr;
    }
    
    void Query::execute() {
        if(!did_execute){
            results = stmt->executeQuery();
            did_execute = true;
        }
        
    }
    
    response_row Query::row(){
        if (!did_execute){
            execute();
        }
        // iterate the row
        bool has_next = results->next();
        if (!has_next) {
            LOG(ERROR) << "Query::row call exceeded row count";
             throw runtime_error("Query::row call exceeded row count");
        }
        return response_row(this);
    }
    
    int Query::row_count(){
        if(!did_execute){
            execute();
        }
        return results->rowsCount();
    }
    
    void Query::set_param(int32_t p){
        // @tag database abstraction
        stmt->setInt(param_index, p);
        param_index++;
    }
    
    void Query::set_param(uint32_t p){
        // @tag database abstraction
        stmt->setUInt(param_index, p);
        param_index++;
    }
    
    void Query::set_param(int64_t p){
        // @tag database abstraction
        stmt->setInt64(param_index, p);
        param_index++;
    }
    
    void Query::set_param(uint64_t p){
        // @tag database abstraction
        stmt->setUInt64(param_index, p);
        param_index++;
    }
    #ifdef __APPLE__
    void Query::set_param(size_t p){
        // @tag database abstraction
        stmt->setUInt64(param_index, p);
        param_index++;
    }
    #endif
    
    void Query::set_param(double p){
        // @tag database abstraction
        stmt->setDouble(param_index, p);
        param_index++;
    }
    
    void Query::set_param(bool p){
        // @tag database abstraction
        stmt->setBoolean(param_index, p);
        param_index++;
    }
    
    void Query::set_param(string p){
        // @tag database abstraction
        stmt->setString(param_index, p);
        param_index++;
    }
    
    void Query::set_param(char * p){
        // @tag database abstraction
        stmt->setString(param_index, p);
        param_index++;
    }
    
    /*
     Query Iterator
    */
    query_iterator Query::begin() {
        return query_iterator(this, false);
    }
    
    query_iterator Query::end() {
        return query_iterator(this, true);
    }
    
}
