//
//  binary_universal_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.
//

#ifndef binary_universal_operator_h
#define binary_universal_operator_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class binary_universal_operator: public operator_t {
        //  MEMBER FIELDS
        
        std::function<string(const string, const string)> cb;
    public:
        //  CONSTRUCTORS
        
        binary_universal_operator(const string key, const std::function<string(const string, const string)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        string apply(const string lhs, const string rhs) const {
            return this->cb(lhs, rhs);
        }
    };
}

#endif /* binary_universal_operator_h */
