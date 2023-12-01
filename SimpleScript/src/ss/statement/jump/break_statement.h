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
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const { return true; }
        
        bool compare(const string value) const { return value == "break"; }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return EMPTY;
        }
        
        string execute(interpreter* ssu) {
            set_break();
            return EMPTY;
        }
        
        void exit() { }
        
        void set_break() { parent->set_break(); }
        
        void set_continue() { unsupported_error("set_continue()"); }
        
        void set_parent(statement_t* parent) { this->parent = parent; }
        
        void set_return(const string result) { unsupported_error("set_return()"); }
    };
}

#endif /* break_statement_h */
