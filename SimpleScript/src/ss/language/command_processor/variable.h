//
//  variable.h
//  SimpleScript
//
//  Created by Corey Ferguson on 2/5/25.
//

#ifndef variable_h
#define variable_h

#include <iostream>

using namespace std;

namespace ss {
    // TYPEDEF

    template <typename T>
    struct variable {
        // CONSTRUCTORS
        
        variable(const string key, const T value, const size_t state) {
            this->_key = key;
            this->_value = value;
            this->_state = state;
        }
        
        variable(const variable<T>& data) : variable(data._key, data._value, data._state){
            this->_readonly = data._readonly;
            this->_suppressed = data._suppressed;
        }
        
        // MEMBER FUNCTIONS
        
        string key() const {
            return this->_key;
        }
        
        bool& readonly() {
            return this->_readonly;
        }
        
        void set_value(const T new_value) {
            if (this->readonly())
                write_error(this->key());
            
            this->_value = new_value;
            
            
        }
        
        size_t state() const {
            return this->_state;
        }
        
        bool suppressed() const {
            return this->_suppressed;
        }
        
        T& value() {
            this->_suppressed = true;
            
            return this->_value;
        }
    private:
        // MEMBER FIELDS
        
        string _key;
        bool   _readonly = false;
        size_t _state;
        bool   _suppressed = false;
        T      _value;
    };
}

#endif /* variable_h */
