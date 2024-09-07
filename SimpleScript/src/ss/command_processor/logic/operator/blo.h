//
//  blo.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef blo_h
#define blo_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class blo : public operator_t {
        //  MEMBER FIELDS
        
        std::function<double(const string, const string)> opr;
    public:
        //  CONSTRUCTORS
        
        blo(const string opc, const std::function<double(const string, const string)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        double apply(const string lhs, const string rhs) const {
            return this->opr(lhs, rhs);
        }
    };
}

#endif /* blo_h */
