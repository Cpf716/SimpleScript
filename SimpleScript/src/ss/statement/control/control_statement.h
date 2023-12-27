//
//  control_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef control_statement_h
#define control_statement_h

#include "statement_t.h"

namespace ss {
    struct control_statement: public statement_t {
        //  MEMBER FUNCTIONS
        
        void exit() {
            this->should_return = true;
            
            for (size_t i = 0; i < this->statementc; ++i)
                this->statementv[i]->exit();
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
            
            for (size_t i = 0; i < this->statementc; ++i)
                this->statementv[i]->set_parent(this);
        }
    protected:
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
        
        bool should_return;
        
        size_t statementc;
        statement_t** statementv = NULL;
    };
}

#endif /* control_statement_h */
