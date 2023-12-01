//
//  sleep_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/28/23.
//

#ifndef sleep_statement_h
#define sleep_statement_h

#include "statement_t.h"

namespace ss {
    class sleep_statement: public statement_t {
        string expression;
        
    public:
        //  CONSTRUCTORS
        
        sleep_statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
        }
        
        void close() { delete this; }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const { return false; }
        
        bool compare(const string value) const { return false; }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return EMPTY;
        }
        
        string execute(interpreter* ssu) {
            string val = ssu->evaluate(this->expression);
            
            if (ss::is_array(val))
                type_error("array", "int");
            
            if (is_string(val))
                type_error("string", "int");
            
            double num = stod(val);
            
            if (!is_int(num))
                type_error("double", "int");
            
            if (num < 0)
                range_error(rtrim(num));
            
            this_thread::sleep_for(milliseconds((long)num));
            
            return EMPTY;
        }
        
        void exit() { }
        
        void set_break() { unsupported_error("set_break()"); }
        
        void set_continue() { unsupported_error("set_continue()"); }
        
        void set_parent(statement_t* parent) { }
        
        void set_return(const string result) { unsupported_error("set_return()"); }
    };
}

#endif /* sleep_statement_h */
