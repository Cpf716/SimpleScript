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
        
        string symbol;
        string expression;
    public:
        //  CONSTRUCTORS
        
        define_statement(const string specifier) {
            string tokenv[specifier.length() + 1];
            size_t tokenc = tokens(tokenv, specifier);
            
            if (!tokenc || !is_symbol(tokenv[0]))
                expect_error("symbol in 'define' statement specifier");
            
            this->symbol = tokenv[0];
            
            ostringstream ss;
            
            if (tokenc > 1) {
                for (size_t i = 1; i < tokenc - 1; ++i)
                    ss << tokenv[i] << " ";
                
                ss << tokenv[tokenc - 1];
            }
            
            if (ss.str().empty())
                expect_error("expression");
            
            this->expression = ss.str();
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
            if (ssu->is_defined(this->symbol))
                defined_error(this->symbol);
            
            string value = ssu->evaluate(this->expression);
            
            if (ss::is_array(value))
                ssu->set_array(symbol, value, true);

            else if (value.empty() || is_string(value))
                ssu->set_string(symbol, value, true);
            else
                ssu->set_number(symbol, stod(value), true);
            
            return empty();
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
        
        void set_parent(statement_t* parent) { }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* define_statement_h */
