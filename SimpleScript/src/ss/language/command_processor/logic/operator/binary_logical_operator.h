//
//  binary_logical_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef binary_logical_operator_h
#define binary_logical_operator_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class binary_logical_operator : public operator_t {
        //  MEMBER FIELDS
        
        std::function<double(const string, const string)> cb;
    public:
        //  CONSTRUCTORS
        
        binary_logical_operator(const string key, const std::function<double(const string, const string)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        double apply(const string lhs, const string rhs) const {
            return this->cb(lhs, rhs);
        }
    };
}

#endif /* binary_logical_operator_h */
