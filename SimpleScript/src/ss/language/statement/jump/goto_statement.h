//
//  goto_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/22/24.
//

#ifndef goto_statement_h
#define goto_statement_h

#include "statement_t.h"

namespace ss {
    class goto_statement: public statement_t {
        //  MEMBER FIELDS
        
        string key;
        
        statement_t* parent = NULL;
    public:
        //  CONSTRUCTORS
        
        goto_statement(string key) {
            if (!is_key(key))
                expect_error("key: " + key);
            
            this->key = key;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const {
            return true;
        }
        
        bool compare(const statement_type value) const {
            return false;
        }
        
        string evaluate(command_processor* cp) {
            unsupported_error("evaluate()");
            
            return null();
        }
        
        string execute(command_processor* cp) {
            this->set_goto(this->key);
            
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
            this->parent->set_goto(key);
        }
        
        void set_level(const size_t level) {
            unsupported_error("set_level()");
        }
        
        void set_parent(statement_t* parent) {
            this->parent = parent;
        }
        
        void set_paused(const bool value) { }
        
        void set_return(const string value) {
            this->parent->set_return(value);
        }
    };
}

#endif /* goto_statement_h */
