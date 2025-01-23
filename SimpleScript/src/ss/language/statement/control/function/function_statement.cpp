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
        size_t tokenc = tokens(tokenv, specifier, 3, (string[]){ "(", ")", "," });
            
        if (!tokenc || !is_key(tokenv[0])) {
            delete[] tokenv;
            expect_error("key: " + tokenv[0]);
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
        
        vector<string> keys;
        
        for (int i = 0; i < floor((tokenc - 3) / 2); ++i) {
            string _tokenv[tokenv[i * 2 + 2].length() + 1];
            size_t _tokenc = tokens(_tokenv, tokenv[i * 2 + 2], 3, (string[]){ "(", ")", "=" });
            
            if (_tokenc == 1) {
                if (!is_key(_tokenv[0])) {
                    delete[] tokenv;
                    expect_error("key in 'function' statement specificer");
                }
                
                if (previous != -1) {
                    delete[] tokenv;
                    throw error("Missing default argument on parameter '" + _tokenv[0] + "'");
                }
                
//                if (_tokenv[0] == "_")
//                    continue;
                
                size_t j = 0;
                while (j < keys.size() && keys[j] != _tokenv[0])
                    ++j;
                
                if (j != keys.size()) {
                    delete[] tokenv;
                    defined_error(_tokenv[0]);
                }
                
                keys.push_back(_tokenv[0]);
            } else {
                size_t j = 0;
                while (j < _tokenc && _tokenv[j] == "(")
                    ++i;
                
                if (j < _tokenc && (tolower(_tokenv[j]) == "const" || tolower(_tokenv[j]) == "var"))
                    ++j;
                
                if (j < _tokenc && tolower(_tokenv[j]) == to_string(array_t))
                    ++j;
                
                if (j == _tokenc || !is_key(_tokenv[j]) || _tokenv[j + 1] != "=") {
                    delete[] tokenv;
                    expect_error("key in 'function' statement specificer");
                }
                
                if (previous != -1 && previous != (int)i - 1) {
                    delete[] tokenv;
                    throw error("Missing default argument on parameter '" + _tokenv[j] + "'");
                }
                
                size_t k = 0;
                while (k < keys.size() && keys[k] != _tokenv[j])
                    ++k;
                
                if (k != keys.size()) {
                    delete[] tokenv;
                    defined_error(_tokenv[j]);
                }
                
                keys.push_back(_tokenv[j]);
                
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
            size_t _tokenc = tokens(_tokenv, tokenv[tokenc - 2], 3, (string[]){ "(", ")", "=" });
            
            if (_tokenc == 1) {
                if (!is_key(tokenv[tokenc - 2])) {
                    delete[] tokenv;
                    expect_error("key in 'function' statement specificer");
                }
                
                if (previous != -1) {
                    delete[] tokenv;
                    throw error("Missing default argument on parameter '" + tokenv[tokenc - 2] + "'");
                }
                
//                if (tokenv[tokenc - 2] != "_") {
//                    size_t i = 0;
//                    while (i < keys.size() && keys[i] != tokenv[tokenc - 2])
//                        ++i;
//                    
//                    if (i != keys.size()) {
//                        delete[] tokenv;
//                        defined_error(tokenv[tokenc - 2]);
//                    }
//                }
            } else {
                size_t i = 0;
                while (i < _tokenc && _tokenv[i] == "(")
                    ++i;
                
                if (i < _tokenc && (tolower(_tokenv[i]) == "const" || tolower(_tokenv[i]) == "var"))
                    ++i;
                
                if (i < _tokenc && tolower(_tokenv[i]) == to_string(array_t))
                    ++i;
                
                if (i == _tokenc || !is_key(_tokenv[i]) || _tokenv[i + 1] != "=") {
                    delete[] tokenv;
                    expect_error("key in 'function' statement specificer");
                }
                
                size_t j = 0;
                while (j < keys.size() && keys[j] != _tokenv[i])
                    ++j;
                
                if (j != keys.size()) {
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
        
        this->value = null();
        
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        size_t state = this->cp->get_state();
        
        this->consume();
        
        for (size_t i = 0; i < this->expressionc; ++i) {
            string tokenv[this->expressionv[i].length() + 1];
            size_t tokenc = tokens(tokenv, this->expressionv[i], 3, (string[]){ "(", ")", "=" });
            
//            if (tokenc == 1 && tokenv[0] == "_")
//                continue;
            
            size_t j = 0;
            while (j < tokenc && tokenv[j] == "(")
                ++j;
            
            if (j < tokenc && (tolower(tokenv[j]) == "const" || tolower(tokenv[j]) == "var"))
                ++j;
            
            if (j < tokenc && tolower(tokenv[j]) == to_string(array_t))
                ++j;
            
            if (this->cp->is_defined(tokenv[j]))
                this->cp->remove_key(tokenv[j]);
            
            if (i < argc)
                set_value(tokenv[j], argv[i]);
            else
                this->cp->evaluate(this->expressionv[i]);
        }
        
        for (size_t i = 0; i < this->statementc; ++i) {
            while (this->is_paused.load());
            
            this->statementv[i]->execute(this->cp);

            if (this->return_flag.load())
                break;
        }

        this->cp->set_state(state);
        
        return value;
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
        this->return_flag.store(true);
        this->parent->exit();
    }

    size_t function_statement::get_level() const {
        return this->parent->get_level();
    }

    void function_statement::kill() {
        this->return_flag.store(true);
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->kill();
    }

    void function_statement::set_break() {
        throw error("break cannot be used outside of a loop");
    }

    void function_statement::set_continue() {
        throw error("continue cannot be used outside of a loop");
    }

    void function_statement::set_goto(const string key) {
        throw error("goto cannot be used outside of a switch");
    }

    void function_statement::set_level(const size_t level) {
        this->parent->set_level(level);
    }

    void function_statement::set_paused(const bool value) {
        this->is_paused.store(value);
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->set_paused(value);
    }

    void function_statement::set_return(const string value) {
        this->value = value;
        this->return_flag.store(true);
    }

    void function_statement::set_value(const string key, const string value) {
#if DEBUG_LEVEL
        assert(key.length());
#endif
        if (this->cp->is_defined(key))
            this->cp->remove_key(key);
        
        if (ss::is_array(value))
            this->cp->set_array(key, value);
        else if (value.empty() || is_string(value))
            this->cp->set_string(key, value);
        else
            this->cp->set_number(key, stod(value));
    }
}
