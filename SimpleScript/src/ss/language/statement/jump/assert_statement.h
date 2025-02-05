//
//  assert_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/3/23.
//

#ifndef assert_statement_h
#define assert_statement_h

#include "statement_t.h"

namespace ss {
    class assert_statement: public statement_t {
        //  MEMBER FIELDS
        
        string expression;

        statement_t* parent = NULL;
        
        atomic<bool> return_flag = false;
    public:
        //  CONSTRUCTORS
        
        assert_statement(const string expression) {
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
            string result = cp->evaluate(expression);
            
            if (this->return_flag.load() || ss::evaluate(result))
                return null();

            this->parent->set_level(0);
            throw ::exception("Assertion failed: (" + expression + ")");
                
            return null();
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        };
        
        void kill() {
            this->return_flag.store(true);
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

#endif /* assert_statement_h */
