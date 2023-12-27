//
//  try_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "try_statement.h"

namespace ss {
    //  CONSTRUCTORS

    try_statement::try_statement(const size_t statementc, statement_t** statementv) {
        this->index = 0;
        while (this->index < statementc && !statementv[this->index]->compare("catch"))
            ++this->index;
        
        if (this->index == statementc)
            expect_error("'catch'");
        
        if (this->index == statementc - 1) {
            if (statementc == 1)
                logger_write("'try' statement has empty body\n");
            
        } else if (statementc == 2)
            logger_write("'try' statement has empty body\n");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void try_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool try_statement::analyze(interpreter* ssu) const {
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
        
        return false;
    }

    bool try_statement::compare(const string value) const { return false; }

    string try_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string try_statement::execute(interpreter* ssu) {
        try {
            this->should_return = false;
            
            string buid = ssu->backup();
            
            for (size_t i = 0; i < index; ++i) {
                this->statementv[i]->execute(ssu);
                
                if (this->should_return)
                    break;
            }
            
            ssu->restore(buid);
            
        } catch (exception& e) {
            size_t i = this->statementc - 1;
            
            if (this->statementv[i]->compare("finally"))
                --i;
            
            string buid = ssu->backup();
            string symbol = decode(this->statementv[i]->evaluate(ssu));
            
            if (ssu->is_defined(symbol))
                ssu->drop(symbol);
            
            ssu->set_string(symbol, encode(e.what()));
            
            this->statementv[i]->execute(ssu);
            
            ssu->drop(symbol);
            
            string symbolv[1];
            
            symbolv[0] = symbol;
            
            ssu->restore(buid, true, 1, symbolv);
            
            if (i != this->statementc - 1)
                this->statementv[this->statementc - 1]->execute(ssu);
        }
        
        return empty();
    }

    void try_statement::set_break() {
        this->should_return = true;
        this->parent->set_break();
    }

    void try_statement::set_continue() {
        this->should_return = true;
        this->parent->set_continue();
    }

    void try_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
