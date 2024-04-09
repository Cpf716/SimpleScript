//
//  assert_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/3/23.
//

#ifndef assert_statement_h
#define assert_statement_h

#include "statement_t.h"

namespace ss {
    class assert_statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;

        statement_t* parent = NULL;
        
        bool should_return = false;
    public:
        //  CONSTRUCTORS
        
        assert_statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const {
            return false;
        }
        
        bool compare(const int value) const {
            return false;
        }
        
        string evaluate(interpreter* ssu) {
            unsupported_error("evaluate()");
            return empty();
        }
        
        string execute(interpreter* ssu) {
            string value = ssu->evaluate(expression);
            
            if (should_return || ss::evaluate(value))
                return empty();

            this->parent->set_level(0);
            throw ss::exception("Assertion failed: (" + expression + ")");
                
            return empty();
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        };
        
        void kill() {
            this->should_return = true;
        }
        
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

#endif /* assert_statement_h */
