//
//  else_if_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "else_if_statement.h"

namespace ss {
    //  CONSTRUCTORS

    else_if_statement::else_if_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (statementc && (statementv[statementc - 1]->compare("catch") ||
            statementv[statementc - 1]->compare("finally")))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void else_if_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool else_if_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'else if' statement has empty body\n");
            
            return false;
        }
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(ssu))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
        
        this->statementv[this->statementc - 1]->analyze(ssu);
        
        return false;
    }

    bool else_if_statement::compare(const string value) const {
        return value == "elseif";
    }

    string else_if_statement::evaluate(interpreter* ssu) {
        return ssu->evaluate(this->expression);
    }

    string else_if_statement::execute(interpreter* ssu) {
        this->should_return = false;
        
        for (size_t i = 0; i < this->statementc; ++i) {
            this->statementv[i]->execute(ssu);
            
            if (this->should_return)
                break;
        }
            
        return empty();
    }

    void else_if_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void else_if_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void else_if_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
