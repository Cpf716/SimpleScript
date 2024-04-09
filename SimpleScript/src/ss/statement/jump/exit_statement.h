//
//  exit_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/28/23.
//

#ifndef exit_statement_h
#define exit_statement_h

#include "statement_t.h"

namespace ss {
    class exit_statement: public statement_t {
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        exit_statement() { }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            return true;
        }
        
        bool compare(const int value) const {
            return value == 4;
        }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            this->exit();
            return empty();
        }
        
        void exit() {
            this->parent->exit();
        }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        };
        
        void kill() { }
        
        void set_break() {
            unsupported_error("set_break()");
        }
        
        void set_continue() {
            unsupported_error("set_continue()");
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

#endif /* exit_statement_h */
