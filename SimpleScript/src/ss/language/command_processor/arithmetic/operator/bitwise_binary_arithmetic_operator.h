//
//  bitwise_binary_arithmetic_operator.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#ifndef bitwise_binary_arithmetic_operator_h
#define bitwise_binary_arithmetic_operator_h

#include "binary_arithmetic_operator_t.h"
#include "functions.h"

namespace ss {
    class bitwise_binary_arithmetic_operator: public binary_arithmetic_operator_t {
        //  MEMBER FIELDS
        
        std::function<double(double, double)> cb;
    public:
        //  CONSTRUCTORS
        
        bitwise_binary_arithmetic_operator(const string key, const std::function<double(double, double)> cb) {
            this->set_opcode(key);
            this->cb = cb;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        double apply(const double lhs, const double rhs) const {
            if (!is_int(lhs) || !is_int(rhs))
                type_error(number_t, int_t);
            
            return this->cb(lhs, rhs);
        }
    };
}

#endif /* bitwise_binary_arithmetic_operator_h */
