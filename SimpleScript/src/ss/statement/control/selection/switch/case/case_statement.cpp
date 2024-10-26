//
//  case_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#include "case_statement.h"

namespace ss {
    //  CONSTRUCTORS

    case_statement::case_statement(const string specifier, const size_t statementc, statement_t** statementv) {
        if (specifier.empty())
            expect_error("expression");
        
        string tokenv[specifier.length() + 1];
        size_t tokenc = tokens(tokenv, specifier, 3, (string[]){ "(", ")", ":" });
        
        int i; size_t p = 0;
        for (i = 0; i < tokenc; ++i) {
            if (tokenv[i] == "(")
                ++p;
            else if (tokenv[i] == ")")
                --p;
            else if (!p && tokenv[i] == ":")
                break;
        }
        
        for (int j = 0; j < i - 1; ++j) {
            tokenv[0] += " " + tokenv[1];
            
            for (size_t k = 1; k < tokenc - 1; ++k)
                swap(tokenv[k], tokenv[k + 1]);
            
            --tokenc;
        }
        
        if (tokenc != 1) {
            if (tokenc > 3)
                expect_error("';' after expression");
                
            string token = tokenv[tokenc - 1];
            
            if (!is_key(token))
                expect_error("key: " + token);
            
            if (token == "defined")
                defined_error(token);
            
            this->key = token;
        }
        
        this->expression = tokenv[0];
        
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
        this->pause_flag = false;
        this->return_flag = false;
        
        for (size_t i = 0; i < this->statementc; ++i) {
            while (this->pause_flag);
            
            this->statementv[i]->execute(cp);
            
//            while (this->should_pause);
            
            if (this->return_flag)
                break;
        }
        
        return null();
    }

    string case_statement::get_key() {
        return this->key;
    }

    void case_statement::set_break() {
        this->return_flag = true;
        this->parent->set_break();
    }

    void case_statement::set_continue() {
        this->return_flag = true;
        this->parent->set_continue();
    }

    void case_statement::set_goto(const string key) {
        this->return_flag = true;
        this->parent->set_goto(key);
    }

    void case_statement::set_return(const string value) {
        this->return_flag = true;
        this->parent->set_return(value);
    }
}
