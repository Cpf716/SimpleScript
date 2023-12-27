//
//  statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef statement_h
#define statement_h

#include "statement_t.h"

namespace ss {
    class statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;
    public:
        //  CONSTRUCTORS
        
        statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            if (!ssu->is_mutating(expression))
                logger_write("Expression result unused: (" + expression + ")\n");
            
            return false;
        }
        
        bool compare(const string value) const { return false; }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            ssu->evaluate(this->expression);
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

#endif /* statement_h */
