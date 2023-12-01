//  Author: Corey Ferguson
//  Date:   September 28, 2023
//  File:   bao_t.h
//

#ifndef bao_t_h
#define bao_t_h

#include "operator_t.h"

namespace ss {
    struct bao_t : public operator_t {
        //  MEMBER FUNCTIONS
        
        virtual double apply(const double lhs, const double rhs) const = 0;
    };
}

#endif
