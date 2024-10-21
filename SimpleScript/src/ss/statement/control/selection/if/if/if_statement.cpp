//
//  if_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "if_statement.h"

namespace ss {
    if_statement::if_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
                
        this->expression = expression;
        
        if (statementc &&
            (statementv[statementc - 1]->compare(case_t) ||
             statementv[statementc - 1]->compare(catch_t) ||
             statementv[statementc - 1]->compare(default_t) ||
            statementv[statementc - 1]->compare(finally_t)))
            expect_error("expression");
        
        index = 0;
        while (index < statementc && !statementv[index]->compare(else_t) && !statementv[index]->compare(else_if_t))
            ++index;
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void if_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool if_statement::analyze(command_processor* cp) const {
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
        
        if (!this->index)
            logger_write("'if' statement has empty body\n");
        
        return false;
    }

    bool if_statement::compare(const statement_type value) const {
        return false;
    }

    string if_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string if_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->should_pause = false;
        this->should_return = false;
        
        size_t state = cp->get_state();
        
        if (ss::evaluate(cp->evaluate(this->expression))) {
            for (size_t i = 0; i < this->index; ++i) {
                while (this->should_pause);
                
                this->statementv[i]->execute(cp);
                
//                while (this->should_pause);
                
                if (this->should_return)
                    break;
            }
            
            cp->set_state(state);
        } else {
            cp->set_state(state);
            
            size_t i;
            for (i = this->index; i < this->statementc; ++i) {
                state = cp->get_state();
                
                if (this->statementv[i]->compare(else_t) ||
                    ss::evaluate(this->statementv[i]->evaluate(cp))) {
                    this->statementv[i]->execute(cp);
                    
                    cp->set_state(state);
                    
                    break;
                }
                
                cp->set_state(state);
            }
        }
        
        return null();
    }

    void if_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void if_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void if_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
