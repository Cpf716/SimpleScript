//
//  catch_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "catch_statement.h"

namespace ss {
    //  CONSTRUCTORS

    catch_statement::catch_statement(const string symbol, const size_t statementc, statement_t** statementv) {
        if (!is_symbol(symbol))
            expect_error("symbol");
        
        this->symbol = symbol;
        
        if (statementc && (statementv[statementc - 1]->compare("else") ||
            statementv[statementc - 1]->compare("elseif")))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void catch_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool catch_statement::analyze(interpreter* ssu) const {
        if (!this->statementc)
            return false;
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(ssu))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
        
        this->statementv[this->statementc - 1]->analyze(ssu);
        
        return false;
    }

    bool catch_statement::compare(const string value) const { return value == "catch"; }

    string catch_statement::evaluate(interpreter* ssu) { return encode(symbol); }

    string catch_statement::execute(interpreter* ssu) {
        this->should_return = false;
        
        for (size_t i = 0; i < this->statementc; ++i) {
            this->statementv[i]->execute(ssu);
            
            if (this->should_return)
                break;
        }
        
        return EMPTY;
    }

    void catch_statement::exit() {
        this->should_return = true;
        
        for (size_t i = 0; i < statementc; ++i)
            statementv[i]->exit();
    }

    void catch_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void catch_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void catch_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
