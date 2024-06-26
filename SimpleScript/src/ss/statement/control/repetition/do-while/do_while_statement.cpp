//
//  do_while_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/12/23.
//

#include "do_while_statement.h"

namespace ss {
    //  CONSTRUCTORS

    do_while_statement::do_while_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void do_while_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool do_while_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'do while' statement has empty body\n");
            
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
            logger_write("'do while' statement will execute at most once\n");
        
        return false;
    }

    bool do_while_statement::compare(const int value) const {
        return false;
    }

    string do_while_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string do_while_statement::execute(interpreter* ssu) {
        this->should_return = false;
        
        while (1) {
            string buid = ssu->backup();
            
            this->should_continue = false;
            
            for (size_t i = 0; i < this->statementc; ++i) {
                this->statementv[i]->execute(ssu);
                
                if (this->should_return || this->should_continue)
                    break;
            }
            
            if (this->should_return || !ss::evaluate(ssu->evaluate(this->expression))) {
                ssu->restore(buid);
                break;
            }
            
            ssu->restore(buid);
        }
        
        return empty();
    }

    void do_while_statement::set_break() {
        this->should_return = true;
    }

    void do_while_statement::set_continue() {
        this->should_continue = true;
    }

    void do_while_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
