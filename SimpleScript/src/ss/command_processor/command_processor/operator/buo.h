//
//  buo.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.
//

#ifndef buo_h
#define buo_h

#include "operator_t.h"
#include <functional>

namespace ss {
    class buo: public operator_t {
        //  MEMBER FIELDS
        
        std::function<string(const string, const string)> opr;
    public:
        //  CONSTRUCTORS
        
        buo(const string opc, const std::function<string(const string, const string)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        string apply(const string lhs, const string rhs) const {
            return this->opr(lhs, rhs);
        }
    };
}

#endif /* buo_h */
