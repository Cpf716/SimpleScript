//
//  for_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/12/23.
//

#include "for_statement.h"

namespace ss {
    //  CONSTRUCTORS

    for_statement::for_statement(const string specifier, const size_t statementc, statement_t** statementv) {
        string tokenv[specifier.length() * 2 + 1];
        size_t tokenc = tokens(tokenv, specifier);
        
        size_t e = 0, s = e;   int p = 0;
        for (; e < tokenc; ++e) {
            if (tokenv[e] == "(")
                ++p;
            else if (tokenv[e] == ")")
                --p;
            else if (!p && tokenv[e] == ",") {
                for (size_t i = e; i > s + 1; --i) {
                    tokenv[s] += " " + tokenv[s + 1];
                    
                    for (size_t j = s + 1; j < tokenc - 1; ++j)
                        swap(tokenv[j], tokenv[j + 1]);
                    
                    --e;
                    --tokenc;
                }
                
                s = e + 1;
            }
        }
        
        for (size_t i = e; i > s + 1; --i) {
            tokenv[s] += " " + tokenv[s + 1];
            
            for (size_t j = s + 1; j < tokenc - 1; ++j)
                swap(tokenv[j], tokenv[j + 1]);
            
            --tokenc;
        }
        
        //  for-in
        if (tokenc == 1) {
            tokenc = tokens(tokenv, tokenv[0]);
            
            if (tokenc < 3 || !is_symbol(tokenv[0]) || tolower(tokenv[1]) != "in")
                expect_error("',' in 'for' statement specifier");
            
            this->expressionv = new string[expressionc = 2];
            this->expressionv[0] = tokenv[0];
            this->expressionv[1] = tokenv[2];
            
            for (size_t i = 3; i < tokenc; ++i)
                this->expressionv[1] += " " + tokenv[i];
        } else {
            //  for ,,
            for (size_t i = 0, n = (size_t)floor(tokenc / 2) + 1; i < n; ++i) {
                if (tokenv[i * 2] == ",") {
                    tokenv[tokenc] = empty();
                    
                    for (size_t j = tokenc; j > i * 2; --j)
                        swap(tokenv[j], tokenv[j - 1]);
                    
                    ++tokenc;
                }
            }
            
            if (tokenv[tokenc - 1] == ",")
                tokenv[tokenc++] = empty();
            
            if (tokenc < 5)
                expect_error("',' in 'for' statement specifier");
            
            if (tokenc > 5)
                expect_error("';' after expression");
            
            this->expressionv = new string[this->expressionc = 3];
            
            for (size_t i = 0; i < 3; ++i)
                this->expressionv[i] = tokenv[i * 2];
        }
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void for_statement::close() {
        delete[] this->expressionv;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        
        if (this->valuev != NULL)
            delete[] this->valuev;
        
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool for_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'for' statement has empty body\n");
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
            logger_write("'for' statement will execute at most once\n");
        
        return false;
    }

    bool for_statement::compare(const int value) const {
        return false;
    }

    string for_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string for_statement::execute(interpreter* ssu) {
        this->should_return = false;
        
        string buid = ssu->backup();
        size_t valuec = 0;
        
        if (this->expressionc == 2) {
            if (ssu->is_defined(this->expressionv[0]))
                ssu->remove_symbol(this->expressionv[0]);
            
            string value = ssu->evaluate(this->expressionv[1]);
            
            this->valuev = new string[value.length() + 1];
            
            valuec = parse(this->valuev, value);
        } else {
            string tokenv[this->expressionv[0].length() + 1];
            size_t tokenc = tokens(tokenv, this->expressionv[0]);
            
            size_t i = 0;
            while (i < tokenc && tokenv[i] == "(")
                ++i;
            
            if (i < tokenc && tolower(tokenv[i]) == "const")
                ++i;
            
            if (i < tokenc && tolower(tokenv[i]) == "array")
                ++i;
            
            if (i < tokenc - 1 && is_symbol(tokenv[i]) && tokenv[i + 1] == "=" && ssu->is_defined(tokenv[i])) {
                this->valuev = new string[valuec = 1];
                this->valuev[0] = tokenv[i];
                
                ssu->remove_symbol(this->valuev[0]);
            }
            
            //  available to every iteration
            ssu->evaluate(this->expressionv[0]);
        }
        
        size_t index = 0;
        while (1) {
            string _buid = ssu->backup();
            
            if (this->expressionc == 2) {
                if (index == valuec) {
                    ssu->restore(_buid, true, true, 1, this->expressionv);
                    break;
                }
                
                if (valuev[index].empty() || is_string(this->valuev[index]))
                    ssu->set_string(this->expressionv[0], this->valuev[index]);
                else
                    ssu->set_number(this->expressionv[0], stod(valuev[index]));
                
                ++index;
                
            } else if (!this->expressionv[1].empty() && !ss::evaluate(ssu->evaluate(this->expressionv[1]))) {
                //  available for one iteration
                ssu->restore(_buid);
                
                break;
            }
            
            this->should_continue = false;
            
            for (size_t j = 0; j < this->statementc; ++j) {
                this->statementv[j]->execute(ssu);
                
                if (this->should_return || this->should_continue)
                    break;
            }
            
            if (this->should_return) {
                if (this->expressionc == 2)
                    ssu->restore(_buid, true, true, 1, this->expressionv);
                else
                    ssu->restore(_buid);
                
                break;
            }
            
            if (this->expressionc == 2)
                ssu->restore(_buid, true, true, 1, this->expressionv);
            else {
                //  available once
                ssu->evaluate(this->expressionv[2]);
                ssu->restore(_buid);
            }
        }
        
        if (this->expressionc == 2 || !valuec)
            ssu->restore(buid);
        else
            ssu->restore(buid, true, true, 1, valuev);
        
        if (valuev != NULL) {
            delete[] valuev;
            valuev = NULL;
        }
        
        return empty();
    }

    void for_statement::set_break() {
        this->should_return = true;
    }

    void for_statement::set_continue() {
        this->should_continue = true;
    }

    void for_statement::set_return(const string result) {
        this->should_return = true;
        this->parent->set_return(result);
    }
}
