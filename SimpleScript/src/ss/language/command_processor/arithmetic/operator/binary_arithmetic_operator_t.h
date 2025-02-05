//  Author: Corey Ferguson
//  Date:   September 28, 2023
//  File:   binary_arithmetic_operator_t.h
//

#ifndef binary_arithmetic_operator_t_h
#define binary_arithmetic_operator_t_h

#include "operator_t.h"
#include <functional>

namespace ss {
    struct binary_arithmetic_operator_t : public operator_t {
        //  MEMBER FUNCTIONS
        
        virtual double apply(const double lhs, const double rhs) const = 0;
    };
}

#endif
