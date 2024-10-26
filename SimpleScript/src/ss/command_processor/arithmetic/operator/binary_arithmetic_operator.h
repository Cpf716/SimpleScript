//
//  binary_arithmetic_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#ifndef binary_arithmetic_operator_h
#define binary_arithmetic_operator_h

#include "binary_arithmetic_operator_t.h"

namespace ss {
    class binary_arithmetic_operator : public binary_arithmetic_operator_t {
        //  MEMBER FIELDS
        std::function<double(const double, const double)> cb;
        
    public:
        //  CONSTRUCTORS
        
        binary_arithmetic_operator(const string key, const std::function<double(const double, const double)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        double apply(const double lhs, const double rhs) const {
            return this->cb(lhs, rhs);
        }
    };
}

#endif /* binary_arithmetic_operator_h */
