//
//  define_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/24/23.
//

#ifndef define_statement_h
#define define_statement_h

#include "statement_t.h"

namespace ss {
    class define_statement : public statement_t {
        //  MEMBER FIELDS
        
        string expression;
        
        string key;
    public:
        //  CONSTRUCTORS
        
        define_statement(const string specifier) {
            string tokenv[specifier.length() + 1];
            size_t tokenc = tokens(tokenv, specifier);
            
            if (!tokenc || !is_key(tokenv[0]))
                expect_error("key in 'define' statement specifier");
            
            this->key = tokenv[0];
            this->expression = specifier.substr(tokenv[0].length(), specifier.length() - tokenv[0].length());
            
            if (this->expression.empty())
                expect_error("expression");
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
            if (cp->is_defined(this->key))
                defined_error(this->key);
            
            string result = cp->evaluate(this->expression);
            
            cp->apply([this, &cp, result](const bool is_save) {
                if (ss::is_array(result))
                    cp->set_array(key, result);

                else if (result.empty() || is_string(result))
                    cp->set_string(key, result);
                else
                    cp->set_number(key, stod(result));
                
                cp->set_read_only(key, true);
            });
            
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

#endif /* define_statement_h */
