//
//  switch_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#include "switch_statement.h"

namespace ss {
    //  CONSTRUCTORS

    switch_statement::switch_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (!statementc)
            expect_error("'default'");
        
        if ((!statementv[0]->compare(case_t) &&
             !statementv[0]->compare(default_t)) ||
             statementv[statementc - 1]->compare(statement_t::catch_t) ||
             statementv[statementc - 1]->compare(statement_t::else_t) ||
             statementv[statementc - 1]->compare(statement_t::else_if_t) ||
             statementv[statementc - 1]->compare(statement_t::finally_t))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void switch_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool switch_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->analyze(cp);
        
        return false;
    }

    bool switch_statement::compare(const statement_type value) const {
        return false;
    }

    string switch_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string switch_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        size_t state = cp->get_state();
        string result = cp->evaluate(this->expression);
        
        for (size_t i = 0; i < this->statementc; ++i) {
            size_t _state = cp->get_state();
            
            if (this->statementv[i]->compare(default_t) ||
                result == this->statementv[i]->evaluate(cp)) {
                this->statementv[i]->execute(cp);
                
                cp->set_state(_state);
                
                break;
            }
            
            cp->set_state(_state);
        }
        
        cp->set_state(state);
        
        return null();
    }

    void switch_statement::set_break() {
        this->should_return = true;
    }

    void switch_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void switch_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
