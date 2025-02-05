//
//  finally_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "finally_statement.h"

namespace ss {
    //  CONSTRUCTORS

    finally_statement::finally_statement(const size_t statementc, statement_t** statementv) {
        if (statementc &&
            (statementv[statementc - 1]->compare(case_t) ||
             statementv[statementc - 1]->compare(default_t) ||
             statementv[statementc - 1]->compare(else_t) ||
             statementv[statementc - 1]->compare(else_if_t)))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void finally_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool finally_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc) {
            logger_write("'finally' statement has empty body\n");
            
            return false;
        }
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(cp))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
        
        this->statementv[this->statementc - 1]->analyze(cp);
        
        return false;
    }

    bool finally_statement::compare(const statement_type value) const {
        return value == finally_t;
    }

    string finally_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string finally_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->is_paused.store(false);
        
        this->return_flag.store(true);
        
        size_t state = cp->get_state();
        
        for (size_t i = 0; i < this->statementc; ++i) {
            // Begin Enhancement 1 - Thread safety - 2025-01-22
            this->check_paused();
            // End Enhancement 1
            
            this->statementv[i]->execute(cp);
            
            if (this->return_flag.load())
                break;
        }
        
        // while (this->is_paused.load());
        
        cp->set_state(state);
        
        return null();
    }

    void finally_statement::set_break() {
        this->return_flag.store(true);
        this->parent->set_break();
    }

    void finally_statement::set_continue() {
        this->return_flag.store(true);
        this->parent->set_continue();
    }

    void finally_statement::set_goto(const string key) {
        this->return_flag.store(true);
        this->parent->set_goto(key);
    }

    void finally_statement::set_return(const string value) {
        this->return_flag.store(true);
        this->parent->set_return(value);
    }
}
