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
            this->expression = expression.empty() ? null() : expression;
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
        
        void set_goto(const string key) {
            unsupported_error("set_goto()");
        }
        
        void set_level(const size_t level) {
            unsupported_error("set_level()");
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
        }
        
        // Begin Enhancement 1 - Thread safety - 2025-01-22
        void set_paused() {
            unsupported_error("set_paused()");
        }
        // End Enhancement 1
        
        void set_paused(const bool value) { }
        
        void set_return(const string value) {
            this->parent->set_return(value);
        }
    };
}

#endif /* return_statement_h */
