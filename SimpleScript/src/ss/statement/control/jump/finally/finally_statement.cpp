//
//  finally_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "finally_statement.h"

namespace ss {
    //  CONSTRUCTORS

    finally_statement::finally_statement(const size_t statementc, statement_t** statementv) {
        if (statementc && (statementv[statementc - 1]->compare("else") ||
            statementv[statementc - 1]->compare("elseif")))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void finally_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool finally_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'finally' statement has empty body\n");
            
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

    bool finally_statement::compare(const string value) const {
        return value == "finally";
    }

    string finally_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string finally_statement::execute(interpreter* ssu) {
        this->should_return = true;
        
        string buid = ssu->backup();
        
        for (size_t i = 0; i < this->statementc; ++i) {
            this->statementv[i]->execute(ssu);
            
            if (this->should_return)
                break;
        }
        
        ssu->restore(buid);
        
        return empty();
    }

    void finally_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void finally_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void finally_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
