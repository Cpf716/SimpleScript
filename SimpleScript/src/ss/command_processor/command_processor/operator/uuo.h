//
//  uuo.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.
//

#ifndef uuo_h
#define uuo_h

#include "operator_t.h"
#include <functional>

using namespace std;

namespace ss {
    class uuo: public operator_t {
        //  MEMBER FIELDS
        
        std::function<string(const string)> opr;
    public:
        //  CONSTRUCTORS
        
        uuo(const string opc, const std::function<string(const string)> opr) {
            this->set_opcode(opc);
            this->opr = opr;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        string apply(const string rhs) const {
            return this->opr(rhs);
        }
    };
}

#endif /* uuo_h */
