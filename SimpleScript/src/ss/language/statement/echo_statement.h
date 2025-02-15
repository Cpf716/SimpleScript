//
//  echo_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef echo_statement_h
#define echo_statement_h

#include "statement_t.h"

namespace ss {
    class echo_statement: public statement_t {
        //  MEMBER FIELDS
        
        string       expression;
//        atomic<bool> return_flag = false;
    public:
        //  CONSTRUCTORS
        
        echo_statement(const string expression) {
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
            
            return null();
        }
        
        string execute(command_processor* cp) {
#if DEBUG_LEVEL
            assert(cp != NULL);
#endif
            string result = cp->evaluate(this->expression);
            
            string valuev[result.length() + 1];
            size_t valuec = parse(valuev, result);
            
            if (valuec != 1) {
                ostringstream ss;
                
                size_t i;
                for (i = 0; i < valuec - 1; ++i)
                    ss << valuev[i] << escape(separator());
                
                ss << valuev[i];
                
                throw error("Unexpected token: " + ss.str());
            }
            
//            if (this->return_flag.load())
//                return null();
            
            cout << (result.empty() ? "null" : escape(decode_raw(result))) << flush;
            
            return null();
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            
            return 0;
        };
        
        void kill() {
//            this->return_flag.store(true);
        }
        
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

#endif /* echo_statement_h */
