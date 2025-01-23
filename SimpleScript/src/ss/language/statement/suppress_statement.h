//
//  suppress_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/5/23.
//

#ifndef suppress_statement_h
#define suppress_statement_h

#include "statement_t.h"

namespace ss {
    class suppress_statement: public statement_t {
        //  MEMBER FIELDS
        
        string key;
    public:
        //  CONSTRUCTORS
        
        suppress_statement(const string key) {
            if (!is_key(key))
                expect_error("key: " + key);
            
            this->key = key;
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const {
            return false;
        }
        
        bool compare(const statement_type value) const {
            return false;
        }
        
        string evaluate(command_processor* cp) {
            unsupported_error("evaluate()");
            return null();
        }
        
        string execute(command_processor* cp) {
#if DEBUG_LEVEL
            assert(cp != NULL);
#endif
            cp->consume(key);
            
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
        
        void set_parent(statement_t* parent) { }
        
        void set_paused(const bool value) { }
        
        void set_return(const string value) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* suppress_statement_h */
