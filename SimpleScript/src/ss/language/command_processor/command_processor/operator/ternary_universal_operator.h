//
//  ternary_universal_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/1/23.
//

#ifndef ternary_universal_operator_h
#define ternary_universal_operator_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class ternary_universal_operator: public operator_t {
        //  MEMBER FIELDS
        
        std::function<string(const string, const string, const string)> cb;
    public:
        //  CONSTRUCTORS
        
        ternary_universal_operator(const string key, const std::function<string(const string, const string, const string)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        string apply(const string lhs, const string ctr, const string rhs) const {
            return this->cb(lhs, ctr, rhs);
        }
    };
}

#endif /* ternary_universal_operator_h */
