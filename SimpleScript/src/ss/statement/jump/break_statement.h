//
//  break_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef break_statement_h
#define break_statement_h

#include "statement_t.h"

namespace ss {
    class break_statement: public statement_t {
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        break_statement() { }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            return true;
        }
        
        bool compare(const int value) const {
            return value == 0;
        }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            set_break();
            return empty();
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        };
        
        void kill() { }
        
        void set_break() {
            parent->set_break();
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

#endif /* break_statement_h */
