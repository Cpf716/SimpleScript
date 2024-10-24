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
            this->return_flag = true;
            this->parent->exit();
        }
        
        size_t get_level() const {
            return this->parent->get_level();
        };
        
        void kill() {
            this->return_flag = true;
            
            for (size_t i = 0; i < this->statementc; ++i)
                this->statementv[i]->kill();
        }
        
        void set_level(const size_t level) {
            this->parent->set_level(level);
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
            
            for (size_t i = 0; i < this->statementc; ++i)
                this->statementv[i]->set_parent(this);
        }
        
        void set_pause(const bool value) {
            this->pause_flag = value;
            
            for (size_t i = 0; i < this->statementc; ++i)
                this->statementv[i]->set_pause(this->pause_flag);
        }
    protected:
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
        
        bool pause_flag;
        
        bool return_flag;
        
        size_t statementc;
        statement_t** statementv = NULL;
    };
}

#endif /* control_statement_h */
