//
//  sleep_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/28/23.
//

#ifndef sleep_statement_h
#define sleep_statement_h

#include "statement_t.h"

namespace ss {
    class sleep_statement: public statement_t {
        string expression;
        
    public:
        //  CONSTRUCTORS
        
        sleep_statement(const string expression) {
            if (expression.empty())
                expect_error("expression");
            
            this->expression = expression;
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
            return empty();
        }
        
        string execute(command_processor* cp) {
#if DEBUG_LEVEL
            assert(cp != NULL);
#endif
            string val = cp->evaluate(this->expression);
            
            if (ss::is_array(val))
                type_error(array_t, int_t);
            
            if (is_string(val))
                type_error(string_t, int_t);
            
            double num = stod(val);
            
            if (!is_int(num))
                type_error(int_t, double_t);
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            this_thread::sleep_for(milliseconds((long)num));
            
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
        
        void set_pause(const bool pause) { }
        
        void set_return(const string result) {
            unsupported_error("set_return()");
        }
    };
}

#endif /* sleep_statement_h */
