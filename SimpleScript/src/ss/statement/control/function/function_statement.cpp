//
//  function_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "function_statement.h"

namespace ss {
    //  CONSTRUCTORS

    function_statement::function_statement(const string specifier, const size_t statementc, statement_t** statementv) {
        string* tokenv = new string[specifier.length() + 1];
        size_t tokenc = tokens(tokenv, specifier);
            
        if (!tokenc || !is_symbol(tokenv[0])) {
            delete[] tokenv;
            expect_error("symbol: " + tokenv[0]);
        }
        
        if (tokenc == 1)
            expressionc = 0;
        else {
            if (tokenv[1] != "(") {
                delete[] tokenv;
                expect_error("';' after expression");
            }
            
            if (tokenv[tokenc - 1] != ")") {
                delete[] tokenv;
                expect_error("')'");
            }
            
            size_t e = 2, s = e;   int p = 0;
            for (; e < tokenc - 1; ++e) {
                if (tokenv[e] == "(")
                    ++p;
                else if (tokenv[e] == ")")
                    --p;
                else if (!p && tokenv[e] == ",") {
                    for (size_t i = e; i > s + 1; --i) {
                        tokenv[s] += " " + tokenv[s + 1];
                        
                        for (size_t j = s + 1; j < tokenc - 1; ++j)
                            swap(tokenv[j], tokenv[j + 1]);
                        
                        --tokenc;    --e;
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
            
            int previous = -1;
            
            for (int i = 0; i < floor((tokenc - 3) / 2); ++i) {
                string _tokenv[tokenv[i * 2 + 2].length() + 1];
                size_t _tokenc = tokens(_tokenv, tokenv[i * 2 + 2]);
                
                if (_tokenc == 1) {
                    if (!is_symbol(_tokenv[0])) {
                        delete[] tokenv;
                        expect_error("symbol in 'function' statement specificer");
                    }
                    
                    if (previous != -1) {
                        delete[] tokenv;
                        throw error("Missing default argument on parameter '" + _tokenv[0] + "'");
                    }
                } else {
                    size_t j = 0;
                    while (j < _tokenc && _tokenv[j] == "(")
                        ++i;
                    
                    if (j < _tokenc && tolower(_tokenv[j]) == "const")
                        ++j;
                    
                    if (j < _tokenc && tolower(_tokenv[j]) == "array")
                        ++j;
                    
                    if (j == _tokenc || !is_symbol(_tokenv[j]) || _tokenv[j + 1] != "=") {
                        delete[] tokenv;
                        expect_error("symbol in 'function' statement specificer");
                    }
                    
                    if (previous != -1 && previous != (int)i - 1) {
                        delete[] tokenv;
                        throw error("Missing default argument on parameter '" + _tokenv[j] + "'");
                    }
                    
                    previous = i;
                    
                    ++this->optionalc;
                }
                
                if (tokenv[i * 2 + 3] != ",") {
                    delete[] tokenv;
                    expect_error("';' after expression");
                }
            }
            
            if (tokenc >= 4) {
                string _tokenv[tokenv[tokenc - 2].length() + 1];
                size_t _tokenc = tokens(_tokenv, tokenv[tokenc - 2]);
                
                if (_tokenc == 1) {
                    if (!is_symbol(tokenv[tokenc - 2])) {
                        delete[] tokenv;
                        expect_error("symbol in 'function' statement specificer");
                    }
                    
                    if (previous != -1) {
                        delete[] tokenv;
                        throw error("Missing default argument on parameter '" + tokenv[tokenc - 2] + "'");
                    }
                } else {
                    size_t i = 0;
                    while (i < _tokenc && _tokenv[i] == "(")
                        ++i;
                    
                    if (i < _tokenc && tolower(_tokenv[i]) == "const")
                        ++i;
                    
                    if (i < _tokenc && tolower(_tokenv[i]) == "array")
                        ++i;
                    
                    if (i == _tokenc || !is_symbol(_tokenv[i]) || _tokenv[i + 1] != "=") {
                        delete[] tokenv;
                        expect_error("symbol in 'function' statement specificer");
                    }
                    
                    ++this->optionalc;
                }
                
                this->expressionc = floor((tokenc - 3) / 2) + 1;
            } else
                this->expressionc = 0;
            
            this->expressionv = new string[this->expressionc];
            
            for (size_t i = 0; i < this->expressionc; ++i)
                this->expressionv[i] = tokenv[i * 2 + 2];
        }
        
        this->rename(tokenv[0]);
        
        delete[] tokenv;
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void function_statement::close() {
        delete[] this->expressionv;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool function_statement::analyze(interpreter* ssu) const {
        if (!this->statementc) {
            logger_write("'function' statement has empty body\n");
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

    string function_statement::call(const size_t argc, string* argv) {
        if (argc < this->expressionc - this->optionalc || argc > this->expressionc)
            expect_error(to_string(this->expressionc - this->optionalc) + " argument(s) but got " + to_string(argc));
        
        this->result = encode("undefined");
        this->should_return = false;
        
        string buid = this->ssu->backup();
        
        this->consume();
        
        string symbolv[this->expressionc];
        
        for (size_t i = 0; i < this->expressionc; ++i) {
            string tokenv[this->expressionv[i].length() + 1];
            size_t tokenc = tokens(tokenv, this->expressionv[i]);
            
            size_t j = 0;
            while (j < tokenc && tokenv[j] == "(")
                ++j;
            
            if (j < tokenc && tolower(tokenv[j]) == "const")
                ++j;
            
            if (j < tokenc && tolower(tokenv[j]) == "array")
                ++j;
            
            if (this->ssu->is_defined(tokenv[j]))
                this->ssu->remove_symbol(tokenv[j]);
            
            if (i < argc)
                this->set_value(tokenv[j], argv[i]);
            else
                this->ssu->evaluate(this->expressionv[i]);
            
            symbolv[i] = tokenv[j];
        }
        
        for (size_t i = 0; i < this->statementc; ++i) {
            this->statementv[i]->execute(this->ssu);

            if (this->should_return)
                break;
        }

        this->ssu->restore(buid, true, true, this->expressionc, symbolv);
        
        return result;
    }

    bool function_statement::compare(const int value) const {
        return false;
    }

    string function_statement::evaluate(interpreter* ssu) {
        unsupported_error("evaluate()");
        return empty();
    }

    string function_statement::execute(interpreter* ssu) {
        this->ssu = ssu;
        this->ssu->set_function(this);
        
        return empty();
    }

    void function_statement::exit() {
        this->should_return = true;
        this->parent->exit();
    }

    size_t function_statement::get_level() const {
        unsupported_error("get_level()");
        return 0;
    }

    void function_statement::kill() {
        this->should_return = true;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->kill();
    }

    void function_statement::set_break() {
        throw error("break cannot be used outside of a loop");
    }

    void function_statement::set_continue() {
        throw error("continue cannot be used outside of a loop");
    }

    void function_statement::set_level(const size_t level) {
        this->parent->set_level(level);
    }

    void function_statement::set_return(const string result) {
        this->result = result;
        this->should_return = true;
    }

    void function_statement::set_value(const string symbol, const string value) {
        if (this->ssu->is_defined(symbol))
            this->ssu->remove_symbol(symbol);
        
        if (ss::is_array(value))
            this->ssu->set_array(symbol, value);
            
        else if (value.empty() || is_string(value))
            this->ssu->set_string(symbol, value);
        else
            this->ssu->set_number(symbol, stod(value));
    }
}
