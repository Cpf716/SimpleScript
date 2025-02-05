//
//  unary_arithmetic_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef unary_arithmetic_operator_h
#define unary_arithmetic_operator_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class unary_arithmetic_operator : public operator_t {
        //  MEMBER FIELDS
        
        std::function<const double(const double)> cb;
    public:
        //  CONSTRUCTORS
        
        unary_arithmetic_operator(const string key, const std::function<double(const double)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        double apply(const double rhs) const {
            return this->cb(rhs);
        }
    };
}

#endif /* unary_arithmetic_operator_h */
