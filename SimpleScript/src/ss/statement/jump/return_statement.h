//
//  return_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/20/23.
//

#ifndef return_statement_h
#define return_statement_h

#include "statement_t.h"

namespace ss {
    class return_statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        return_statement(string expression) {
            this->expression = expression.empty() ? encode(to_string(undefined_t)) : expression;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const {
            return true;
        }
        
        bool compare(const statement_type value) const {
            return value == return_t;
        }
        
        string evaluate(command_processor* cp) {
            unsupported_error("evaluate()");
            return null();
        }
        
        string execute(command_processor* cp) {
            this->set_return(cp->evaluate(expression));
            return null();
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
            unsupported_error("set_continue()");
        }
        
        void set_level(const size_t level) {
            unsupported_error("set_level()");
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
        }
        
        void set_pause(const bool pause) { }
        
        void set_return(const string result) {
            this->parent->set_return(result);
        }
    };
}

#endif /* return_statement_h */
