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
        
        string symbol;
    public:
        //  CONSTRUCTORS
        
        suppress_statement(const string symbol) {
            if (!is_symbol(symbol))
                expect_error("symbol: " + symbol);
            
            this->symbol = symbol;
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
            cp->consume(symbol);
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
        
        void set_parent(statement_t* parent) { }
        
        void set_pause(const bool pause) { }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* suppress_statement_h */
