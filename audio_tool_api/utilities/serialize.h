//
//  serialize.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 1/12/19.
//  Copyright © 2019 Chris Pauley. All rights reserved.
//

#ifndef serialize_hpp
#define serialize_hpp
#define KVP(T) ::serialize::make_kvp(#T, T)

#include <stdio.h>
#include "json.hpp"

namespace serialize {
    template<typename T>
    class KeyValuePair {
    public:
        KeyValuePair( const char * _name, T && _value): name(_name), value(std::forward<T>(_value)) {}
        const char * name;
        T value;
    };

    template<typename T> inline
    KeyValuePair<T> make_kvp(const char * name, T && value){
        return { name, std::forward<T>(value) };
    }
    
    template<typename T>
    void fill_object(nlohmann::json * j, KeyValuePair<T> kvp){
        (*j)[kvp.name] = kvp.value;
    }
    
    template<typename T, typename ...O>
    void fill_object(nlohmann::json * j, KeyValuePair<T> first, const O & ...others){
        fill_object(j, first);
        fill_object(j, others...);
    }
    
    template<typename ...O>
    nlohmann::json jsonify(const O & ...others){
        nlohmann::json j = nlohmann::json::object();
        fill_object(&j, others...);
        return j;
    }
    
    template<typename ...O>
    std::string dump(const O & ...others){
        nlohmann::json j = jsonify(others...);
        return j.dump();
    }
    
    template<class T>
    class json_list : public vector<T> {
    public:
        virtual operator nlohmann::json(){
            nlohmann::json j = nlohmann::json::array();
            for(auto v : *this){
                j.push_back(v);
            }
            return j;
        }
    };
}
#endif /* serialize_hpp */
