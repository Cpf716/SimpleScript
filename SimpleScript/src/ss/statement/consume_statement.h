//
//  consume_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/5/23.
//

#ifndef consume_statement_h
#define consume_statement_h

#include "statement_t.h"

namespace ss {
    class consume_statement: public statement_t {
        //  MEMBER FIELDS
        
        string symbol;
    public:
        //  CONSTRUCTORS
        
        consume_statement(const string symbol) {
            if (!is_symbol(symbol))
                expect_error("symbol: " + symbol);
            
            this->symbol = symbol;
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
            ssu->consume(symbol);
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

#endif /* consume_statement_h */
