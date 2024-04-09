//
//  throw_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/26/23.
//

#ifndef throw_statement_h
#define throw_statement_h

#include "statement_t.h"

namespace ss {
    class throw_statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;
        
        statement_t* parent = NULL;
        
        bool should_return = false;
    public:
        //  CONSTRUCTORS
        
        throw_statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
        }
        
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
            string message = ssu->evaluate(this->expression);
            
            if (should_return)
                return empty();
            
            if (ss::is_array(message))
                type_error("array", "string");
            
            if (message.empty())
                null_error();
            
            if (!is_string(message))
                type_error("double", "string");
            
            this->set_level(0);
            
            throw exception(decode(message));
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
            this->parent->set_level(level);
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
        }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* throw_statement_h */
