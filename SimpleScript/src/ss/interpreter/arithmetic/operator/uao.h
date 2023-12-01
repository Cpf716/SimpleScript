//
//  uao.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef uao_h
#define uao_h

#include "operator_t.h"

namespace ss {
    class uao : public operator_t {
        //  MEMBER FIELDS
        
        std::function<const double(const double)> opr;
    public:
        //  CONSTRUCTORS
        
        uao(const string opc, const std::function<double(const double)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        double apply(const double rhs) const {
            return this->opr(rhs);
        }
    };
}

#endif /* uao_h */
