//
//  bao.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#ifndef bao_h
#define bao_h

#include "bao_t.h"

namespace ss {
    class bao : public bao_t {
        //  MEMBER FIELDS
        std::function<double(const double, const double)> opr;
        
    public:
        //  CONSTRUCTORS
        
        bao(const string opc, const std::function<double(const double, const double)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        double apply(const double lhs, const double rhs) const {
            return this->opr(lhs, rhs);
        }
    };
}

#endif /* bao_h */
