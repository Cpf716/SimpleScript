//
//  exit_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/28/23.
//

#ifndef exit_statement_h
#define exit_statement_h

#include "statement_t.h"

namespace ss {
    class exit_statement: public statement_t {
        //  MEMBER FIELDS
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        exit_statement() { }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const {
            return true;
        }
        
        bool compare(const statement_type value) const {
            return value == exit_t;
        }
        
        string evaluate(command_processor* cp) {
            unsupported_error("evaluate()");
            return null();
        }
        
        string execute(command_processor* cp) {
            this->exit();
            return null();
        }
        
        void exit() {
            this->parent->exit();
        }
        
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
            unsupported_error("set_return()");
        }
    };
}

#endif /* exit_statement_h */
