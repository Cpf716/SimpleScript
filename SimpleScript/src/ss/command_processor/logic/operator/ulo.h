//
//  ulo.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef ulo_h
#define ulo_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class ulo : public operator_t {
        //  MEMBER FIELDS
        
        std::function<double(const string s)> opr;
    public:
        //  CONSTRUCTORS
        
        ulo(const string opc, const std::function<double(const string s)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        double apply(const string rhs) const {
            return this->opr(rhs);
        }
    };
}

#endif /* ulo_h */
