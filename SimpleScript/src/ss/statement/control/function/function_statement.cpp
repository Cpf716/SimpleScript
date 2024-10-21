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
        
        if (tokenc == 1 || tokenv[1] != "(") {
            delete[] tokenv;
            expect_error("'(' in 'function' statement specifier");
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
        
        vector<string> symbols;
        
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
                
                if (_tokenv[0] == "_")
                    continue;
                
                size_t j = 0;
                while (j < symbols.size() && symbols[j] != _tokenv[0])
                    ++j;
                
                if (j != symbols.size()) {
                    delete[] tokenv;
                    defined_error(_tokenv[0]);
                }
                
                symbols.push_back(_tokenv[0]);
            } else {
                size_t j = 0;
                while (j < _tokenc && _tokenv[j] == "(")
                    ++i;
                
                if (j < _tokenc && (tolower(_tokenv[j]) == "const" || tolower(_tokenv[j]) == "var"))
                    ++j;
                
                if (j < _tokenc && tolower(_tokenv[j]) == to_string(array_t))
                    ++j;
                
                if (j == _tokenc || !is_symbol(_tokenv[j]) || _tokenv[j + 1] != "=") {
                    delete[] tokenv;
                    expect_error("symbol in 'function' statement specificer");
                }
                
                if (previous != -1 && previous != (int)i - 1) {
                    delete[] tokenv;
                    throw error("Missing default argument on parameter '" + _tokenv[j] + "'");
                }
                
                size_t k = 0;
                while (k < symbols.size() && symbols[k] != _tokenv[j])
                    ++k;
                
                if (k != symbols.size()) {
                    delete[] tokenv;
                    defined_error(_tokenv[j]);
                }
                
                symbols.push_back(_tokenv[j]);
                
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
                
                if (tokenv[tokenc - 2] != "_") {
                    size_t i = 0;
                    while (i < symbols.size() && symbols[i] != tokenv[tokenc - 2])
                        ++i;
                    
                    if (i != symbols.size()) {
                        delete[] tokenv;
                        defined_error(tokenv[tokenc - 2]);
                    }
                }
            } else {
                size_t i = 0;
                while (i < _tokenc && _tokenv[i] == "(")
                    ++i;
                
                if (i < _tokenc && (tolower(_tokenv[i]) == "const" || tolower(_tokenv[i]) == "var"))
                    ++i;
                
                if (i < _tokenc && tolower(_tokenv[i]) == to_string(array_t))
                    ++i;
                
                if (i == _tokenc || !is_symbol(_tokenv[i]) || _tokenv[i + 1] != "=") {
                    delete[] tokenv;
                    expect_error("symbol in 'function' statement specificer");
                }
                
                size_t j = 0;
                while (j < symbols.size() && symbols[j] != _tokenv[i])
                    ++j;
                
                if (j != symbols.size()) {
                    delete[] tokenv;
                    defined_error(_tokenv[i]);
                }
                
                ++this->optionalc;
            }
            
            this->expressionc = floor((tokenc - 3) / 2) + 1;
        } else
            this->expressionc = 0;
        
        //  copy params
        this->expressionv = new string[this->expressionc];
        
        for (size_t i = 0; i < this->expressionc; ++i)
            this->expressionv[i] = tokenv[i * 2 + 2];
        
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

    bool function_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc) {
            logger_write("'function' statement has empty body\n");
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

    string function_statement::call(const size_t argc, string* argv) {
        if (argc < this->expressionc - this->optionalc || argc > this->expressionc)
            expect_error(std::to_string(this->expressionc - this->optionalc) + " argument(s) but got " + std::to_string(argc));
        
        this->result = encode(to_string(undefined_t));
        this->should_pause = false;
        this->should_return = false;
        
        size_t state = this->cp->get_state();
        
        this->consume();
        
        for (size_t i = 0; i < this->expressionc; ++i) {
            string tokenv[this->expressionv[i].length() + 1];
            size_t tokenc = tokens(tokenv, this->expressionv[i]);
            
            if (tokenc == 1 && tokenv[0] == "_")
                continue;
            
            size_t j = 0;
            while (j < tokenc && tokenv[j] == "(")
                ++j;
            
            if (j < tokenc && (tolower(tokenv[j]) == "const" || tolower(tokenv[j]) == "var"))
                ++j;
            
            if (j < tokenc && tolower(tokenv[j]) == to_string(array_t))
                ++j;
            
            if (this->cp->is_defined(tokenv[j]))
                this->cp->remove_symbol(tokenv[j]);
            
            if (i < argc)
                set_value(tokenv[j], argv[i]);
            else
                this->cp->evaluate(this->expressionv[i]);
        }
        
        for (size_t i = 0; i < this->statementc; ++i) {
            while (this->should_pause);
            
            this->statementv[i]->execute(this->cp);
            
//            while (this->should_pause);

            if (this->should_return)
                break;
        }

        this->cp->set_state(state);
        
        return result;
    }

    bool function_statement::compare(const statement_type value) const {
        return false;
    }

    string function_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string function_statement::execute(command_processor* cp) {
        this->cp = cp;
        this->cp->set_function(this);
        
        return null();
    }

    void function_statement::exit() {
        this->should_return = true;
        this->parent->exit();
    }

    size_t function_statement::get_level() const {
        return this->parent->get_level();
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

    void function_statement::set_pause(const bool pause) {
        this->should_pause = pause;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->set_pause(pause);
    }

    void function_statement::set_return(const string result) {
        this->result = result;
        this->should_return = true;
    }

    void function_statement::set_value(const string symbol, const string value) {
#if DEBUG_LEVEL
        assert(symbol.length());
#endif
        if (this->cp->is_defined(symbol))
            this->cp->remove_symbol(symbol);
        
        if (ss::is_array(value))
            this->cp->set_array(symbol, value);
        else if (value.empty() || is_string(value))
            this->cp->set_string(symbol, value);
        else
            this->cp->set_number(symbol, stod(value));
    }
}
