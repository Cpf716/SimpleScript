//
//  while_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/12/23.
//

#include "while_statement.h"

namespace ss {
    //  CONSTRUCTORS

    while_statement::while_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void while_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool while_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'while' statement has empty body\n");
            
            return false;
        }
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(ssu))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
                
        if (this->statementv[i]->analyze(ssu) &&
            (this->statementv[i]->compare(0) ||
             this->statementv[i]->compare(6)))
            logger_write("'while' statement will execute at most once\n");
        
        return false;
    }

    string while_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string while_statement::execute(interpreter* ssu) {
        this->should_return = false;
        
        while (1) {
            string buid = ssu->backup();
            
            if (!ss::evaluate(ssu->evaluate(this->expression))) {
                ssu->restore(buid);
                break;
            }
            
            this->should_continue = false;
            
            for (size_t i = 0; i < this->statementc; ++i) {
                this->statementv[i]->execute(ssu);
                
                if (this->should_return || this->should_continue)
                    break;
            }
            
            ssu->restore(buid);
            
            if (this->should_return)
                break;
        }
        
        return empty();
    }

    bool while_statement::compare(const int value) const {
        return false;
    }

    void while_statement::set_break() {
        this->should_return = true;
    }

    void while_statement::set_continue() {
        this->should_continue = true;
    }

    void while_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
