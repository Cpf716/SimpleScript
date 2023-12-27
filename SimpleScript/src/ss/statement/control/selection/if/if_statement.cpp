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
        
        if (statementc && (statementv[statementc - 1]->compare("catch") ||
            statementv[statementc - 1]->compare("finally")))
            expect_error("expression");
        
        index = 0;
        while (index < statementc && !statementv[index]->compare("elseif") && !statementv[index]->compare("else"))
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

    bool if_statement::analyze(interpreter* ssu) const {
        if (this->index) {
            size_t i = 0;
            while (i < this->index - 1 && !this->statementv[i]->analyze(ssu))
                ++i;
            
            if (i != this->index - 1)
                logger_write("Unreachable code\n");
            
            this->statementv[this->index - 1]->analyze(ssu);
        }
        
        for (size_t i = this->index; i < this->statementc; ++i)
            this->statementv[i]->analyze(ssu);
        
        if (!this->index)
            logger_write("'if' statement has empty body\n");
        
        return false;
    }

    bool if_statement::compare(const string value) const {
        return false;
    }

    string if_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string if_statement::execute(interpreter* ssu) {
        string buid = ssu->backup();
        
        if (ss::evaluate(ssu->evaluate(this->expression))) {
            this->should_return = false;
            
            for (size_t i = 0; i < this->index; ++i) {
                this->statementv[i]->execute(ssu);
                
                if (this->should_return)
                    break;
            }
            
            ssu->restore(buid);
        } else {
            ssu->restore(buid);
            
            size_t i;
            for (i = this->index; i < this->statementc; ++i) {
                buid = ssu->backup();
                
                bool flag = !this->statementv[i]->compare("elseif") || ss::evaluate(this->statementv[i]->evaluate(ssu));
                
                if (flag)
                    this->statementv[i]->execute(ssu);
                
                ssu->restore(buid);
                
                if (flag)
                    break;
            }
        }
        
        return empty();
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
