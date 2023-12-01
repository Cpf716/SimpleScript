//
//  function_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/12/22.
//

#ifndef function_t_h
#define function_t_h

#include <iostream>
#include "utility.h"

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
        
        size_t count() const { return this->_count; }
        
        void consume() { ++this->_count; }
        
        string name() const { return this->_name; }
        
        void rename(const string new_name) {
            if (!is_symbol(new_name))
                expect_error("symbol");
            
            this->_name = new_name;
        }
    };
}

#endif /* function_t_h */
