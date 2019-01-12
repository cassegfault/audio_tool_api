//
//  database.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/10/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef database_hpp
#define database_hpp

#include <stdio.h>
#include <string>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <mutex>

using namespace std;

/*
 This is a series of abstraction classes that provide a cleaner
 API for interacting with MySQL databases. With some work it could be
 further abstracted to interface with multiple relational databases.
*/

namespace db {
    class Connection;
    class Query;
    class query_iterator;
    class response_row;
    
    class Query {
        friend class response_field;
        friend class query_iterator;
    public:
        Query(db::Connection * conn, const char * query);
        ~Query();
        
        void execute();
        
        // These allow us to set any parameter using the same
        // function name, useful for templating
        void set_param(int32_t p);
        void set_param(uint32_t p);
        void set_param(int64_t p);
        void set_param(uint64_t p);
        void set_param(double p);
        void set_param(bool p);
        void set_param(string p);
        void set_param(char * p);
        
        // Necessary template setup for set_params below
        template<typename T>
        void set_any(T & param){
            set_param(param);
        };
        
        // This gives us an interface for binding paramaters by type
        // from arbitrary parameters, a much simpler interface
        template<typename T>
        void set_params(const T & first_param) {
            set_any(first_param);
        }
        template<typename T, typename ...O>
        void set_params(const T & first_param, const O & ...others) {
            set_params(first_param);
            set_params(std::cref(others)...);
        }
        
        /*
         These make the query response iterable, for example:
         
             for (auto row : db.query("SELECT 1, true as `some_name`, NOW()")) {
                 row[0]; // 1
                 row["some_name"]; // true
                // ...
             }
         
         */
        query_iterator begin();
        query_iterator end();
        
        // Get a single row back
        response_row row();
        bool did_execute = false;
    private:
        // set_param calls are sequential, this tracks which paramater we are on
        int param_index = 1;
        
        // @tag database abstraction
        sql::PreparedStatement *stmt;
        sql::ResultSet *results;
        
        Connection * conn;
    };
    
    class Connection {
        friend class Query;
    public:
        Connection();
        ~Connection();
        
        template<typename ...P>
        db::Query query(const char * query_string, const P & ...parameters){
            Query q(this, query_string);
            q.set_params(std::cref(parameters)...);
            return q;
        };
        db::Query raw_query(const char * query_string);
        
        bool is_open(){
            return _is_open;
        };
        
        void open(string connection_string, string user, string password, string database);
        void close();
        
    private:
        bool _is_open = false;
        
        // @tag database abstraction
        sql::Connection * sql;
    };
    
    class response_field {
    public:
        response_field(Query * _query, int idx): query(_query), index(idx), identifier(nullptr) {
            if(!query->did_execute){
                query->execute();
            }
        };
        response_field(Query * _query, string _id): query(_query), index(-1), identifier(_id) {
            if(!query->did_execute){
                query->execute();
            }
        };
        operator string(){
            if(index > -1){
                return query->results->getString(index);
            } else {
                return query->results->getString(identifier);
                
            }
        }
        operator std::istream * (){
            if(index > -1){
                return query->results->getBlob(index);
            } else {
                return query->results->getBlob(identifier);
                
            }
        }
        operator int(){
            if(index > -1){
                return query->results->getInt(index);
            } else {
                return query->results->getInt(identifier);
            }
        }
        operator uint(){
            if(index > -1){
                return query->results->getUInt(index);
            } else {
                return query->results->getUInt(identifier);
            }
        }
        operator int64_t(){
            if(index > -1){
                return query->results->getInt64(index);
            } else {
                return query->results->getInt64(identifier);
            }
        }
        operator uint64_t(){
            if(index > -1){
                return query->results->getUInt64(index);
            } else {
                return query->results->getUInt64(identifier);
            }
        }
        operator double(){
            if(index > -1){
                return query->results->getDouble(index);
            } else {
                return query->results->getDouble(identifier);
            }
        }
        operator bool(){
            if(index > -1){
                return query->results->getBoolean(index);
            } else {
                return query->results->getBoolean(identifier);
            }
        }
    private:
        Query * query;
        int index;
        string identifier;
    };
    
    // a single set of results
    class response_row {
    public:
        response_row(Query * _query): query(_query) {};
        response_field operator [](int index){
            return response_field(query, index);
        };
        response_field operator [](string identifier){
            return response_field(query, identifier);
        };
    private:
        Query * query;
    };
    
    /*
     A custom iterator to access the results of the query.
     
     Follow the chart at http://www.cplusplus.com/reference/iterator/ to implement
     additional operators for extended directional support
    */
    class query_iterator {
    public:
        query_iterator(Query *q, bool _at_end = false): query(q), at_end(_at_end) {
            if(!query->did_execute){
                query->execute();
            }
            if(at_end) {
                row_idx = query->results->rowsCount();
            } else {
                row_idx = 0;
                if(query->results->rowsCount() == 0) {
                    at_end = true;
                }
            }
            
        };
        
        // Necessary for all iterators
        query_iterator& operator = (const query_iterator& other) {
            this->row_idx = other.row_idx;
            this->at_end = other.at_end;
            this->query = other.query;
            return *this;
        }
        
        query_iterator& operator++ () {
            if (!at_end){
                //query->results->next();
                row_idx++;
            }
            if(query->results->rowsCount() <= row_idx){
                at_end = true;
            }
            return *this;
        }
        
        // Necessary for forward input
        response_row operator * () {
            return query->row();
        }
        
        bool operator == (const query_iterator& other) const {
            
            return row_idx == other.row_idx &&
                    at_end == other.at_end &&
                    query == other.query;
        }
        
        bool operator != (const query_iterator& other) const {
            return row_idx != other.row_idx ||
                    at_end != other.at_end ||
                    query != other.query;
        }
        
    private:
        int row_idx;
        bool at_end;
        Query * query;
    };
}


#endif /* database_hpp */
