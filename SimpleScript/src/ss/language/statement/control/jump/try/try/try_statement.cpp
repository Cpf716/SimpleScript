//
//  try_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "try_statement.h"

namespace ss {
    //  CONSTRUCTORS

    try_statement::try_statement(const size_t statementc, statement_t** statementv) {
        if (!statementc)
            expect_error("'catch'");
        
        if (statementv[statementc - 1]->compare(case_t) ||
            statementv[statementc - 1]->compare(else_t) ||
            statementv[statementc - 1]->compare(else_if_t) ||
            statementv[statementc - 1]->compare(default_t))
            expect_error("expression");
        
        this->position = statementc - 1;
        if (this->position && statementv[this->position]->compare(finally_t))
            --this->position;
        
        if (!statementv[this->position]->compare(catch_t))
            expect_error("'catch'");
        
        if (!this->position)
            logger_write("'try' statement has empty body");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void try_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool try_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (this->position) {
            size_t i = 0;
            while (i < this->position && !this->statementv[i]->analyze(cp))
                ++i;
            
            if (i != this->position)
                logger_write("Unreachable code");
        }
        
        for (size_t i = this->position; i < this->statementc; ++i)
            this->statementv[i]->analyze(cp);
        
        return false;
    }

    bool try_statement::compare(const statement_type value) const {
        return false;
    }

    string try_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string try_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        size_t state = cp->get_state();
        
        try {
            for (size_t i = 0; i < this->position; ++i) {
                // Begin Enhancement 1 - Thread safety - 2025-01-22
                this->check_paused();
                // End Enhancement 1
                
                this->statementv[i]->execute(cp);
                
                if (this->return_flag.load())
                    break;
            }
            
            // while (this->is_paused.load());
            
            cp->set_state(state);
            
        } catch (::exception& e) {
            if (this->get_level() >= 2)
                throw error(e.what());
            
            cp->set_state(state);
            
            state = cp->get_state();
            
            string key = decode_raw(this->statementv[this->position]->evaluate(cp));
            
            if (cp->is_defined(key))
                cp->remove_key(key);
            
            string what = e.what();
            
            try {
                cp->set_number(key, stod(what));
            } catch (std::exception& e) {
                cp->set_string(key, encode(what));
            }
            
            this->statementv[this->position]->execute(cp);
            
            cp->remove_key(key);
            cp->set_state(state);
            
            if (this->position != this->statementc - 1)
                this->statementv[this->statementc - 1]->execute(cp);
        }
        
        return null();
    }

    void try_statement::set_break() {
        this->return_flag.store(true);
        this->parent->set_break();
    }

    void try_statement::set_continue() {
        this->return_flag.store(true);
        this->parent->set_continue();
    }

    void try_statement::set_goto(const string key) {
        this->return_flag.store(true);
        this->parent->set_goto(key);
    }

    void try_statement::set_return(const string value) {
        this->return_flag.store(true);
        this->parent->set_return(value);
    }
}
