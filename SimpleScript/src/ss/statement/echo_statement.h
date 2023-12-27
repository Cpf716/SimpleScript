//
//  echo_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef echo_statement_h
#define echo_statement_h

#include "statement_t.h"

namespace ss {
    class echo_statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;
    public:
        //  CONSTRUCTORS
        
        echo_statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            return false;
        }
        
        bool compare(const string value) const {
            return false;
        }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            string result = ssu->evaluate(expression);
            
            if (ss::is_array(result))
                type_error("array", "string");
            
            cout << (result.empty() ? "null" : decode(result));
            
            return empty();
        }
        
        void exit() { }
        
        void set_break() {
            unsupported_error("set_break()");
        }
        
        void set_continue() {
            unsupported_error("set_continue()");
        }
        
        void set_parent(statement_t* parent) { }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* echo_statement_h */
