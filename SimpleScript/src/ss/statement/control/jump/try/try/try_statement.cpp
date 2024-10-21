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
        
        this->index = 0;
        while (this->index < statementc && !statementv[this->index]->compare(catch_t))
            ++this->index;
        
        if (this->index == statementc)
            expect_error("'catch'");
        
        if (this->index == statementc - 1) {
            if (statementc == 1)
                logger_write("'try' statement has empty body\n");
            
        } else if (statementc == 2)
            logger_write("'try' statement has empty body\n");
        
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
        if (this->index) {
            size_t i = 0;
            while (i < this->index - 1 && !this->statementv[i]->analyze(cp))
                ++i;
            
            if (i != this->index - 1)
                logger_write("Unreachable code\n");
            
            this->statementv[this->index - 1]->analyze(cp);
        }
        
        for (size_t i = this->index; i < this->statementc; ++i)
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
        this->should_pause = false;
        this->should_return = false;
        
        size_t state;
        
        try {
            state = cp->get_state();
            
            for (size_t i = 0; i < index; ++i) {
                while (this->should_pause);
                
                this->statementv[i]->execute(cp);
                
//                while (this->should_pause);
                
                if (this->should_return)
                    break;
            }
            
            cp->set_state(state);
            
        } catch (exception& e) {
            if (this->get_level() >= 2)
                throw error(e.what());
            
            cp->set_state(state);
            
            size_t i = this->statementc - 1;
            
            if (this->statementv[i]->compare(finally_t))
                --i;
            
            state = cp->get_state();
            
            string symbol = decode_raw(this->statementv[i]->evaluate(cp));
            
            if (cp->is_defined(symbol))
                cp->remove_symbol(symbol);
            
            cp->set_string(symbol, encode(e.what()));
            
            this->statementv[i]->execute(cp);
            
            cp->remove_symbol(symbol);
            
            cp->set_state(state);
            
            if (i != this->statementc - 1)
                this->statementv[this->statementc - 1]->execute(cp);
        }
        
        return null();
    }

    void try_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void try_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void try_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
