//
//  case_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#include "case_statement.h"

namespace ss {
    //  CONSTRUCTORS

    case_statement::case_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (statementc &&
            (statementv[statementc - 1]->compare(catch_t) ||
             statementv[statementc - 1]->compare(else_t) ||
             statementv[statementc  - 1]->compare(else_if_t) ||
             statementv[statementc - 1]->compare(finally_t)))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void case_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool case_statement::analyze(command_processor* cp) const {
    #if DEBUG_LEVEL
        assert(cp != NULL);
    #endif
        if (!this->statementc) {
            logger_write("'case' statement has empty body\n");
            
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

    bool case_statement::compare(const statement_type value) const {
        return value == case_t;
    }

    string case_statement::evaluate(command_processor* cp) {
    #if DEBUG_LEVEL
        assert(cp != NULL);
    #endif
        return cp->evaluate(this->expression);
    }

    string case_statement::execute(command_processor* cp) {
    #if DEBUG_LEVEL
        assert(cp != NULL);
    #endif
        this->should_pause = false;
        this->should_return = false;
        
        for (size_t i = 0; i < this->statementc; ++i) {
            while (this->should_pause);
            
            this->statementv[i]->execute(cp);
            
//            while (this->should_pause);
            
            if (this->should_return)
                break;
        }
        
        return null();
    }

    void case_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void case_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void case_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
