//
//  exception_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/26/23.
//

#ifndef exception_statement_h
#define exception_statement_h

#include "statement_t.h"

namespace ss {
    class exception_statement: public statement_t {
        //  MEMBER FIELDS
        
        string message;
    public:
        //  CONSTRUCTORS
        
        exception_statement(const string message) {
            if (message.empty())
                expect_error("expression");
            
            this->message = message;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const { return true; }
        
        bool compare(const string value) const { return false; }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return EMPTY;
        }
        
        string execute(interpreter* ssu) {
            throw exception(decode(ssu->evaluate(message)));
            return EMPTY;
        }
        
        void exit() { }
        
        void set_break() { unsupported_error("set_break()"); }
        
        void set_continue() { unsupported_error("set_continue()"); }
        
        void set_parent(statement_t* parent) { }
        
        void set_return(const string result) { unsupported_error("set_return()"); }
    };
}

#endif /* exception_statement_h */
