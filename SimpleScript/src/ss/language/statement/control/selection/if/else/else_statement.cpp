//
//  else_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "else_statement.h"

namespace ss {
    //  CONSTRUCTORS

    else_statement::else_statement(const size_t statementc, statement_t** statementv) {
        if (statementc &&
            (statementv[statementc - 1]->compare(case_t) ||
             statementv[statementc - 1]->compare(catch_t) ||
             statementv[statementc - 1]->compare(default_t) ||
             statementv[statementc - 1]->compare(finally_t)))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void else_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool else_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc) {
            logger_write("'else' statement has empty body\n");
            
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

    bool else_statement::compare(const statement_type value) const {
        return value == else_t;
    }

    string else_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string else_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        for (size_t i = 0; i < this->statementc; ++i) {
            // Begin Enhancement 1 - Thread safety - 2025-01-22
            this->check_paused();
            // End Enhancement 1

            this->statementv[i]->execute(cp);
            
            if (this->return_flag.load())
                break;
        }
        
        // while (this->is_paused.load());
        
        return null();
    }

    void else_statement::set_break() {
        this->return_flag.store(true);
        this->parent->set_break();
    }

    void else_statement::set_continue() {
        this->return_flag.store(true);
        this->parent->set_continue();
    }

    void else_statement::set_goto(const string key) {
        this->return_flag.store(true);
        this->parent->set_goto(key);
    }

    void else_statement::set_return(const string value) {
        this->return_flag.store(true);
        this->parent->set_return(value);
    }
}
