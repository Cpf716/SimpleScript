//
//  for_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/12/23.
//

#include "for_statement.h"

namespace ss {
    //  CONSTRUCTORS

    for_statement::for_statement(const string specifier, const size_t statementc, statement_t** statementv) {
        string tokenv[specifier.length() + 1];
        size_t tokenc = parse(tokenv, specifier, ";");

        //  for-in
        if (tokenc == 1) {
            tokenc = tokens(tokenv, tokenv[0]);
            
            if (tokenc < 3 || !is_key(tokenv[0]) || tokenv[1] != "in")
                expect_error("';' in 'for' statement specifier");
            
            this->expressionv = new string[expressionc = 2];
            this->expressionv[0] = tokenv[0];
            this->expressionv[1] = tokenv[2];
            
            for (size_t i = 3; i < tokenc; ++i)
                this->expressionv[1] += " " + tokenv[i];
        } else {
            if (tokenc < 3)
                expect_error("';' in 'for' statement specifier");
            
            if (tokenc > 3)
                expect_error("expression");
            
            this->expressionv = new string[this->expressionc = 3];

            for (size_t i = 0; i < 3; ++i)
                this->expressionv[i] = tokenv[i];
        }
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void for_statement::close() {
        delete[] this->expressionv;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        
        if (this->valuev != NULL)
            delete[] this->valuev;
        
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool for_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc) {
            logger_write("'for' statement has empty body");
            return false;
        }
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(cp))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code");
                
        if (this->statementv[i]->analyze(cp) &&
            (this->statementv[i]->compare(break_t) ||
             this->statementv[i]->compare(exit_t) ||
             this->statementv[i]->compare(return_t)))
            logger_write("'for' statement will execute at most once");
        
        return false;
    }

    bool for_statement::compare(const statement_type value) const {
        return false;
    }

    string for_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string for_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        size_t state = cp->get_state();
        size_t valuec = 0;
        
        if (this->expressionc == 2) {
            if (cp->is_defined(this->expressionv[0]))
                cp->remove_key(this->expressionv[0]);
            
            string result = cp->evaluate(this->expressionv[1]);
            
            this->valuev = new string[result.length() + 1];
            
            valuec = parse(this->valuev, result);
        } else
            //  available to every iteration
            cp->evaluate(this->expressionv[0]);
        
        size_t pos = 0;
        
        while (true) {
            // Begin Enhancement 1 - Thread safety - 2025-01-22
            this->check_paused();
            // End Enhancement 1
            
            size_t _state = cp->get_state();
            
            if (this->expressionc == 2) {
                if (pos == valuec) {
                    cp->set_state(_state);
                    break;
                }
                
                if (valuev[pos].empty() || is_string(this->valuev[pos]))
                    cp->set_string(this->expressionv[0], this->valuev[pos]);
                else
                    cp->set_number(this->expressionv[0], stod(valuev[pos]));
                
                ++pos;
                
            } else if (!this->expressionv[1].empty() && !ss::evaluate(cp->evaluate(this->expressionv[1]))) {
                //  available for one iteration
                cp->set_state(_state);
                break;
            }
            
            this->continue_flag = false;
            
            for (size_t j = 0; j < this->statementc; ++j) {
                // Begin Enhancement 1 - Thread safety - 2025-01-22
                this->check_paused();
                // End Enhancement 1
                
                this->statementv[j]->execute(cp);
                
                if (this->return_flag.load() || this->continue_flag)
                    break;
            }
            
            // while (this->is_paused.load());
            
            if (this->return_flag.load()) {
                if (this->expressionc == 2)
                    cp->remove_key(this->expressionv[0]);
                
                cp->set_state(_state);

                break;
            }
            
            if (this->expressionc == 2)
                cp->remove_key(this->expressionv[0]);
            else
                //  available once
                cp->evaluate(this->expressionv[2]);
            
            cp->set_state(_state);
        }
        
        cp->set_state(state);
        
        if (valuev != NULL) {
            delete[] valuev;
            valuev = NULL;
        }
        
        return null();
    }

    void for_statement::set_break() {
        this->return_flag.store(true);
    }

    void for_statement::set_continue() {
        this->continue_flag = true;
    }

    void for_statement::set_goto(const string key) {
        this->return_flag.store(true);
        this->parent->set_goto(key);
    }

    void for_statement::set_return(const string value) {
        this->return_flag.store(true);
        this->parent->set_return(value);
    }
}
