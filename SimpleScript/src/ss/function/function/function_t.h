//
//  function_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/12/22.
//

#ifndef function_t_h
#define function_t_h

#include "common.h"
#include <iostream>

using namespace std;

namespace ss {
    class function_t {
        //  MEMBER FIELDS
        
        size_t _count = 0;
        string _name;
    public:
        //  CONSTRUCTORS
        
        virtual void close() = 0;
        
        //  MEMBER FUNCTIONS
        
        virtual string call(const size_t argc, string* argv) = 0;
        
        size_t count() const {
            return this->_count;
        }
        
        void consume() {
            ++this->_count;
        }
        
        virtual void exit() = 0;
        
        virtual size_t get_level() const = 0;
        
        virtual void kill() = 0;
        
        string name() const {
            return this->_name;
        }
        
        void rename(const string new_name) {
#if DEBUG_LEVEL
            assert(new_name.length());
#endif
            this->_name = new_name;
        }
        
        virtual void set_level(const size_t level) = 0;
        
        virtual void set_pause(const bool pause) = 0;
    };
}

#endif /* function_t_h */
