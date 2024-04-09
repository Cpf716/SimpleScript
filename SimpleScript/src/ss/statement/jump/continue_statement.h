//
//  continue_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/20/23.
//

#ifndef continue_statement_h
#define continue_statement_h

#include "statement_t.h"

namespace ss {
    class continue_statement: public statement_t {
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        continue_statement() { }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            return true;
        }
        
        bool compare(const int value) const {
            return false;
        }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            set_continue();
            return empty();
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        };
        
        void kill() { }
        
        void set_break() {
            unsupported_error("set_break()");
        }
        
        void set_continue() {
            parent->set_continue();
        }
        
        void set_level(const size_t level) {
            unsupported_error("set_level()");
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
        }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* continue_statement_h */
