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
        if (statementc && (statementv[statementc - 1]->compare("catch") ||
            statementv[statementc - 1]->compare("finally")))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void else_statement::close() {
        for (size_t i = 0; i < statementc; ++i)
            statementv[i]->close();
        
        delete[] statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool else_statement::analyze(interpreter* ssu) const {
        if (!statementc) {
            logger_write("'else' statement has empty body\n");
            
            return false;
        }
        
        size_t i = 0;
        while (i < statementc - 1 && !statementv[i]->analyze(ssu))
            ++i;
        
        if (i != statementc - 1)
            logger_write("Unreachable code\n");
        
        statementv[statementc - 1]->analyze(ssu);
        
        return false;
    }

    bool else_statement::compare(const string value) const { return value == "else"; }

    string else_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return EMPTY;
    }

    string else_statement::execute(interpreter* ssu) {
        should_return = false;
        
        for (size_t i = 0; i < statementc; ++i) {
            statementv[i]->execute(ssu);
            
            if (should_return)
                break;
        }
        
        return EMPTY;
    }

    void else_statement::exit() {
        this->should_return = true;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->exit();
    }

    void else_statement::set_break() {
        should_return = true;
        parent->set_break();
    }

    void else_statement::set_continue() {
        should_return = true;
        parent->set_continue();
    }

    void else_statement::set_return(const string result) {
        should_return = true;
        parent->set_return(result);
    }
}
