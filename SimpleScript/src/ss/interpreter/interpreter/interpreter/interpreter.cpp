//
//  interpreter.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.
//

#include "interpreter.h"

namespace ss {
    //  CONSTRUCTORS

    interpreter::interpreter() {
        initialize();
        
        //  setAskBeforeExit()
        set_number("askBeforeExit", 0);
        set_read_only("askBeforeExit", true);
        
        set_string("null", empty());
        std::get<2>(* stringv[io_string("null")]) = pair<bool, bool>(true, false);
        
        //  setTimeout()
        set_number("timeout", 0);
        set_read_only("timeout", true);
        
        //  setTimeoutMessage()
        set_string("timeoutMessage", empty());
        std::get<2>(* stringv[io_string("timeoutMessage")]) = pair<bool, bool>(true, false);
        
        for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
            set_array("types", i, encode(types[i]));
        
        int i = io_array("types");
        
        std::get<1>(* arrayv[i]).shrink_to_fit();
        std::get<2>(* arrayv[i]) = pair<bool, bool>(true, false);
        
        set_string("undefined", encode(types[5]));
        std::get<2>(* stringv[io_string("undefined")]) = pair<bool, bool>(true, false);
        
        buid = backup();
    }

    interpreter::~interpreter() {
        //  close files
        for (size_t i = 0; i < filev.size(); ++i) {
            filev[i].second->close();
            delete filev[i].second;
        }
        
        //  restore backups
        while (backupc)
            restore(bu_numberv[backupc - 1], false);
        
        //  deallocate backups
        delete[] bu_arrayv;
        delete[] bu_functionv;
        delete[] bu_numberv;
        delete[] bu_stringv;
        
        //  close arrays tree
        if (array_bst != NULL)
            array_bst->close();
        
        //  deallocate arrays
        for (size_t i = 0; i < arrayc; ++i)
            delete arrayv[i];
        
        delete[] arrayv;
        
        //  close functions tree
        if (function_bst != NULL)
            function_bst->close();
        
        //  close functions
        for (size_t i = 0; i < functionc; ++i)
            functionv[i]->close();
        
        delete[] functionv;
        
        //  close strings tree
        if (string_bst != NULL)
            string_bst->close();
        
        //  deallocate strings
        for (size_t i = 0; i < stringc; ++i)
            delete stringv[i];
        
        delete[] stringv;
        
        //  deallocate universal operators tree
        for (size_t i = 0; i < 6; ++i)
            delete[] buov[i];
        
        delete[] buov;
        
        //  close universal operators
        for (size_t i = 0; i < uoc; ++i)
            uov[i]->close();
        
        delete[] uov;
    }

    //  MEMBER FUNCTIONS

    string interpreter::backup() {
        //  resize backups
        if (is_pow(backupc, 2)) {
            //  resize numbers
            string* _bu_numberv = new string[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_numberv[i] = bu_numberv[i];
            
            delete[] bu_numberv;
            bu_numberv = _bu_numberv;
            
            //  resize functions
            pair<size_t, function_t**>** _bu_functionv = new pair<size_t, function_t**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_functionv[i] = bu_functionv[i];
            
            delete[] bu_functionv;
            bu_functionv = _bu_functionv;
            
            //  resize arrays
            //  array value is a pointer, as its internal pointer will be deallocated otherwise when control exits the array's declaring scope
            pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>** _bu_arrayv = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_arrayv[i] = bu_arrayv[i];
            
            delete[] bu_arrayv;
            bu_arrayv = _bu_arrayv;
            
            //  resize strings
            pair<size_t, tuple<string, string, pair<bool, bool>>**>** _bu_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc;  ++i)
                _bu_stringv[i] = bu_stringv[i];
            
            delete[] bu_stringv;
            bu_stringv = _bu_stringv;
        }
        
        //  backup arrays
        tuple<string, ss::array<string>*, pair<bool, bool>>** _arrayv = new tuple<string, ss::array<string>*, pair<bool, bool>>*[pow2(arrayc)];
        
        for (size_t i = 0; i < arrayc; ++i) {
            string first = std::get<0>(* arrayv[i]);
            ss::array<string>* second = new ss::array<string>(std::get<1>(* arrayv[i]));
            pair<bool, bool> third = std::get<2>(* arrayv[i]);
            
            _arrayv[i] = new tuple<string, ss::array<string>*, pair<bool, bool>>(first, second, third);
        }
        
        bu_arrayv[backupc] = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>(arrayc, _arrayv);
        
        //  backup functions
        function_t** _functionv = new function_t*[pow2(functionc)];
        
        for (size_t i = 0; i < functionc; ++i)
            _functionv[i] = functionv[i];
        
        bu_functionv[backupc] = new pair<size_t, function_t**>(functionc, _functionv);
        
        string _uuid = arithmetic::backup();
        
        //  backup numbers
        bu_numberv[backupc] = _uuid;
        
        //  backup strings
        tuple<string, string, pair<bool, bool>>** _stringv = new tuple<string, string, pair<bool, bool>>*[pow2(stringc)];
        
        for (size_t i = 0; i < stringc; ++i) {
            string first = std::get<0>(* stringv[i]);
            string second = std::get<1>(* stringv[i]);
            pair<bool, bool> third = std::get<2>(* stringv[i]);
            
            _stringv[i] = new tuple<string, string, pair<bool, bool>>(first, second, third);
        }
        
        bu_stringv[backupc] = new pair<size_t, tuple<string, string, pair<bool, bool>>**>(stringc, _stringv);
        
        ++backupc;
        
        return _uuid;
    }

    void interpreter::backup(const string buid) {
        size_t i = 0;
        while (i < backupc && bu_numberv[i] != buid)
            ++i;
        
        if (i != backupc)
            defined_error(buid);
        
        //  resize backups
        if (is_pow(backupc, 2)) {
            
            //  resize numbers
            string* _bu_numberv = new string[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_numberv[i] = bu_numberv[i];
            
            delete[] bu_numberv;
            bu_numberv = _bu_numberv;
            
            //  resize functions
            pair<size_t, function_t**>** _bu_functionv = new pair<size_t, function_t**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_functionv[i] = bu_functionv[i];
            
            delete[] bu_functionv;
            bu_functionv = _bu_functionv;
            
            //  resize arrays
            //  array value is a pointer, as its internal pointer will be deallocated otherwise when control exits the array's declaring scope
            pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>** _bu_arrayv = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_arrayv[i] = bu_arrayv[i];
            
            delete[] bu_arrayv;
            bu_arrayv = _bu_arrayv;
            
            //  resize strings
            pair<size_t, tuple<string, string, pair<bool, bool>>**>** _bu_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc;  ++i)
                _bu_stringv[i] = bu_stringv[i];
            
            delete[] bu_stringv;
            bu_stringv = _bu_stringv;
        }
        
        //  backup arrays
        tuple<string, ss::array<string>*, pair<bool, bool>>** _arrayv = new tuple<string, ss::array<string>*, pair<bool, bool>>*[pow2(arrayc)];
        
        for (size_t i = 0; i < arrayc; ++i) {
            string first = std::get<0>(* arrayv[i]);
            ss::array<string>* second = new ss::array<string>(std::get<1>(* arrayv[i]));
            pair<bool, bool> third = std::get<2>(* arrayv[i]);
            
            _arrayv[i] = new tuple<string, ss::array<string>*, pair<bool, bool>>(first, second, third);
        }
        
        bu_arrayv[backupc] = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>(arrayc, _arrayv);
        
        //  backup functions
        function_t** _functionv = new function_t*[pow2(functionc)];
        
        for (size_t i = 0; i < functionc; ++i)
            _functionv[i] = functionv[i];
        
        bu_functionv[backupc] = new pair<size_t, function_t**>(functionc, _functionv);
        
        arithmetic::backup(buid);
        
        //  backup numbers
        bu_numberv[backupc] = buid;
        
        //  backup strings
        tuple<string, string, pair<bool, bool>>** _stringv = new tuple<string, string, pair<bool, bool>>*[pow2(stringc)];
        
        for (size_t i = 0; i < stringc; ++i) {
            string first = std::get<0>(* stringv[i]);
            string second = std::get<1>(* stringv[i]);
            pair<bool, bool> third = std::get<2>(* stringv[i]);
            
            _stringv[i] = new tuple<string, string, pair<bool, bool>>(first, second, third);
        }
        
        bu_stringv[backupc] = new pair<size_t, tuple<string, string, pair<bool, bool>>**>(stringc, _stringv);
        
        ++backupc;
    }

    void interpreter::consume(const string symbol) {
        if (symbol.empty())
            expect_error("symbol: " + symbol);
        
        int i = io_function(symbol);
        
        if (i == -1) {
            i = io_array(symbol);
            
            if (i == -1) {
                i = io_string(symbol);
                
                if (i == -1)
                    arithmetic::consume(symbol);
                else
                    std::get<2>(* stringv[i]).second = true;
            } else
                std::get<2>(* arrayv[i]).second = true;
        } else
            functionv[i]->consume();
    }

    string interpreter::element(const string val) {
        if (ss::is_array(val))
            type_error(0, 4);
            //  array != string
        
        if (val.empty())
            return empty();
        
        if (is_string(val))
            return encode(decode(val));
        
        if (is_symbol(val)) {
            if (io_array(val) != -1)
                type_error(0, 4);
                //  array != string
            
            int i = io_string(val);
            if (i == -1)
                return rtrim(get_number(val));
            
            std::get<2>(* stringv[i]).second = true;
            
            return std::get<1>(* stringv[i]);
        }
        
        return rtrim(stod(val));
    }

    string interpreter::evaluate(const string expression) {
        current_expression = expression;
        
        string data[expression.length() * 2 + 1];
        int n = prefix(data, expression);
        
        ss::stack<string> operands = ss::stack<string>();
        
        for (int i = n - 1; i >= 0; --i) {
            if (data[i] == "(" || data[i] == ")" || tolower(data[i]) == uov[const_oi]->opcode())
                continue;
            
            string term = tolower(data[i]);
            
            size_t j = 0;
            while (j < uoc && term != uov[j]->opcode())
                ++j;
                    
            if (j == uoc) {
                for (j = 0; j < aoc - 2; ++j) {
                    if (j == 10 || (j >= 16 && j < 22))
                        continue;
                    
                    if (term == tolower(aov[j]->opcode()))
                        break;
                }
                
                if (j == aoc - 2) {
                    j = 0;
                    while (j < loc - 1 && data[i] != lov[j]->opcode())
                        ++j;
                    
                    if (j == loc - 1) {
                        int j = io_function(data[i]);
                        
                        if (j == -1) {
                            //  validate operand
                            
                            string tokenv[data[i].length() + 1];
                            int tokenc = (int)tokens(tokenv, data[i]);
                            
                            tokenc = merge_numbers(tokenc, tokenv);
                            
                            if (tokenc == 1 && !is_string(tokenv[0]) &&
                                !is_symbol(tokenv[0]) && !is_number(tokenv[0]))
                                throw error("Unexpected token: " + data[i]);
                            
                            operands.push(data[i]);
                        } else {
                            size_t k = i;
                            while (k > 0 && data[k - 1] == "(")
                                --k;
                            
                            if (k > 0 && data[k - 1] == uov[const_oi]->opcode())
                                --k;
                            
                            if (k > 0 && data[k - 1] == uov[assignment_oi + 1]->opcode())
                                continue;
                            
                            size_t argc = 0;   int p = 0;   bool flag = false;
                            for (k = i + 1; k < n; ++k) {
                                if (data[k] == "(")
                                    ++p;
                                else if (data[k] == ")") {
                                    if (p <= 1)
                                        break;
                                    
                                    --p;
                                } else if (p == 1 && data[k] == uov[unary_count]->opcode())
                                    ++argc;
                                else {
                                    size_t l = 0;
                                    while (l < data[k].length() && (data[k][l] == ' ' || data[k][l] == '('))
                                        ++l;
                                    
                                    while (l < data[k].length() && (data[k][l] == ' ' || data[k][l] == ')'))
                                        ++j;
                                    
                                    if (l == data[k].length())
                                        operands.pop();
                                    else
                                        flag = true;
                                }
                                
                                if (p == 0)
                                    break;
                            }
                            
                            if (flag)
                                ++argc;
                            
                            string argv[argc];
                            
                            for (k = 0; k < argc; ++k) {
                                argv[k] = operands.pop();
                                argv[k] = evaluate(argv[k]);
                            }
                            
                            stack.push(functionv[j]);
                            
                            //  call function and push its result
                            operands.push(functionv[j]->call(argc, argv));
                            
                            stack.pop();
                        }
                    } else {
                        double d;
                        
                        if (j == 0) {
                            string rhs = operands.top();
                            
                            if (ss::is_array(rhs))
                                d = 1;
                            else {
                                if (rhs.empty())
                                    d = 0;
                                else if (is_string(rhs)) {
                                    rhs = decode(rhs);
                                    d = !(rhs.empty() || rhs == types[5]);
                                    
                                } else if (is_symbol(rhs)) {
                                    int k = io_array(rhs);
                                    if (k == -1) {
                                        k = io_string(rhs);
                                        if (k == -1)
                                            d = get_number(rhs);
                                        else {
                                            rhs = std::get<1>(* stringv[k]);
                                            
                                            if (rhs.empty())
                                                d = 0;
                                            else {
                                                rhs = decode(rhs);
                                                
                                                d = !(rhs.empty() || rhs == types[5]);
                                            }
                                            
                                            std::get<2>(* stringv[k]).second = true;
                                        }
                                    } else {
                                        std::get<2>(* arrayv[k]).second = true;
                                        
                                        if (std::get<1>(* arrayv[k]).size() == 1) {
                                            rhs = std::get<1>(* arrayv[k])[0];
                                            
                                            if (rhs.empty())
                                                d = 0;
                                            else if (is_string(rhs))
                                                d = decode(rhs) == types[5] ? 0 : 1;
                                            else
                                                d = stod(rhs) ? 1 : 0;
                                        } else
                                            d = 1;
                                    }
                                } else
                                    d = stod(rhs);
                            }
                            
                            d = ((ulo *)lov[j])->apply(to_string(d));
                        } else {
                            string  lhs = operands.pop();
                            
                            if (lhs.empty()) {
                                if (j == 1)
                                    d = 0;
                                else {
                                    string rhs = operands.top();
                                    
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs))
                                        d = 1;
                                    else {
                                        if (rhs.empty())
                                            d = 0;
                                        else if (is_string(rhs)) {
                                            rhs = decode(rhs);
                                            
                                            d = !(rhs.empty() || rhs == types[5]);
                                        } else
                                            d = stod(rhs) ? 1 : 0;
                                    }
                                }
                            } else {
                                lhs = evaluate(lhs);
                                
                                if (lhs.empty()) {
                                    if (j == 1)
                                        d = 0;
                                    else {
                                        string rhs = operands.top();
                                        
                                        rhs = evaluate(rhs);
                                        
                                        if (ss::is_array(rhs))
                                            d = 1;
                                        else {
                                            if (rhs.empty())
                                                d = 0;
                                            else if (is_string(rhs)) {
                                                rhs = decode(rhs);
                                                
                                                d = !(rhs.empty() || rhs == types[5]);
                                            } else
                                                d = stod(rhs) ? 1 : 0;
                                        }
                                    }
                                } else {
                                    if (!ss::is_array(lhs)) {
                                        if (is_string(lhs)) {
                                            //  d = 1
                                            
                                            lhs = decode(lhs);
                                            
                                            d = !(lhs.empty() || lhs == types[5]);
                                            
                                            if (d == 1 && j == 1) {
                                                string rhs = operands.top();
                                                
                                                rhs = evaluate(rhs);
                                                
                                                if (ss::is_array(rhs))
                                                    d = 1;
                                                else {
                                                    if (rhs.empty())
                                                        d = 0;
                                                    else if (is_string(rhs)) {
                                                        rhs = decode(rhs);
                                                        
                                                        d = !(rhs.empty() || rhs == types[5]);
                                                    } else
                                                        d = stod(rhs) ? 1 : 0;
                                                }
                                            }
                                        } else {
                                            if (stod(lhs)) {
                                                if (j == 1) {
                                                    string rhs = operands.top();
                                                    
                                                    rhs = evaluate(rhs);
                                                    
                                                    if (ss::is_array(rhs))
                                                        d = 1;
                                                    else {
                                                        if (rhs.empty())
                                                            d = 0;
                                                        else if (is_string(rhs)) {
                                                            rhs = decode(rhs);
                                                            
                                                            d = !(rhs.empty() || rhs == types[5]);
                                                        } else
                                                            d = stod(rhs) ? 1 : 0;
                                                    }
                                                } else
                                                    d = 1;
                                            } else {
                                                if (j == 1)
                                                    d = 0;
                                                else {
                                                    string rhs = operands.top();
                                                    
                                                    rhs = evaluate(rhs);
                                                    
                                                    if (ss::is_array(rhs))
                                                        d = 1;
                                                    else {
                                                        if (rhs.empty())
                                                            d = 0;
                                                        else if (is_string(rhs)) {
                                                            rhs = decode(rhs);
                                                            
                                                            d = !(rhs.empty() || rhs == types[5]);
                                                        } else
                                                            d = stod(rhs) ? 1 : 0;
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        //  d = 1
                                        
                                        if (j == 1) {
                                            string rhs = operands.top();
                                            
                                            rhs = evaluate(rhs);
                                            
                                            if (ss::is_array(rhs))
                                                d = 1;
                                            else {
                                                if (rhs.empty())
                                                    d = 0;
                                                else if (is_string(rhs)) {
                                                    rhs = decode(rhs);
                                                    
                                                    d = !(rhs.empty() || rhs == types[5]);
                                                } else
                                                    d = stod(rhs) ? 1 : 0;
                                            }
                                        } else
                                            d = 1;
                                    }
                                }
                            }
                        }
                        
                        operands.pop();
                        operands.push(rtrim(d));
                    }
                } else {
                    string rhs;
                    
                    string* dst = NULL;
                    size_t s;
                    
                    if (j >= 25) {
                        string lhs = operands.pop();
                        
                        if (lhs.empty())
                            operation_error();
                        
                        dst = new string[lhs.length() + 1];
                        s = parse(dst, lhs);
                        
                        if (s == 1) {
                            delete[] dst;
                            
                            if (is_string(lhs) || !is_symbol(lhs))
                                operation_error();
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                if (io_string(lhs) != -1)
                                    operation_error();
                                
                                size_t l = i + 1;
                                while (l < n && data[l] == "(")
                                    ++l;
                                
                                if (data[l] == uov[indexer_oi]->opcode())
                                    operation_error();
                                
                                rhs = operands.top();
                                
                                if (rhs.empty())
                                    type_error(4, 2);
                                    //  string != double
                                
                                dst = new string[rhs.length() + 1];
                                s = parse(dst, rhs);
                                delete[] dst;
                                
                                if (s != 1)
                                    type_error(0, 2);
                                    //  array != double
                                
                                if (is_string(rhs))
                                    type_error(4, 2);
                                    //  string != double
                                    
                                double b;
                                if (is_symbol(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (io_string(rhs) != -1)
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    b = get_number(rhs);
                                } else
                                    b = stod(rhs);
                                
                                double a = get_number(lhs);
                                
                                a = ((bao *)aov[j])->apply(a, b);
                                
                                set_number(lhs, a);
                                
                                rhs = rtrim(a);
                            } else {
                                size_t l = i + 1;
                                while (l < n && data[l] == "(")
                                    ++l;
                                
                                if (data[l] != uov[indexer_oi]->opcode())
                                    operation_error();
                                
                                rhs = operands.pop();
                                
                                if (rhs.empty()) {
                                    if (is_dictionary(std::get<1>(* arrayv[k])))
                                        null_error();
                                    
                                    type_error(4, 3);
                                    //  string != int
                                }
                                
                                dst = new string[rhs.length() + 1];
                                s = parse(dst, rhs);
                                delete[] dst;
                                
                                if (s != 1)
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (is_string(rhs)) {
                                    if (!is_dictionary(std::get<1>(* arrayv[k])))
                                        type_error(0, 6);
                                        //  array != dictionary
                                    
                                    rhs = decode(rhs);
                                    
                                    if (rhs.empty())
                                        undefined_error(encode((empty())));
                                    
                                    rhs = encode(rhs);
                                    
                                    size_t l = 0;
                                    while (l < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[l * 2] != rhs)
                                        ++l;
                                    
                                    if (l == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                        undefined_error(rhs);
                                    
                                    if (std::get<1>(* arrayv[k])[l * 2 + 1].empty() || is_string(std::get<1>(* arrayv[k])[l * 2 + 1]))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    dst = new string[rhs.length() + 1];
                                    s = parse(dst, rhs);
                                    delete[] dst;
                                    
                                    if (s != 1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    double b;
                                    if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = stod(rhs);
                                    
                                    double a = stod(std::get<1>(* arrayv[k])[l * 2 + 1]);
                                    
                                    a = ((bao *)aov[j])->apply(a, b);
                                    
                                    rhs = rtrim(a);
                                    
                                    std::get<1>(* arrayv[k])[l * 2 + 1] = rhs;
                                    
                                } else if (is_symbol(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    int l = io_string(rhs);
                                    if (l == -1) {
                                        double idx = get_number(rhs);
                                        
                                        if (!is_int(idx))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (idx < 0 || idx >= std::get<1>(* arrayv[k]).size())
                                            range_error("index " + rtrim(idx) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                        
                                        if (std::get<1>(* arrayv[k])[(size_t)idx].empty() || is_string(std::get<1>(* arrayv[k])[(size_t)idx]))
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        dst = new string[rhs.length() + 1];
                                        s = parse(dst, rhs);
                                        delete[] dst;
                                        
                                        if (s != 1)
                                            type_error(0, 2);
                                            //  array !=  double
                                        
                                        if (is_string(rhs))
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        double b;
                                        if (is_symbol(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(0, 2);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(4, 2);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = stod(rhs);
                                        
                                        double a = stod(std::get<1>(* arrayv[k])[(size_t)idx]);
                                        
                                        a = ((bao *)aov[j])->apply(a, b);
                                        
                                        rhs = rtrim(a);
                                        
                                        std::get<1>(* arrayv[k])[(size_t)idx] = rhs;
                                    } else {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(0, 6);
                                            //  array != dictionary
                                        
                                        rhs = std::get<1>(* stringv[l]);
                                        
                                        if (rhs.empty())
                                            null_error();
                                        
                                        if (rhs.length() == 2)
                                            undefined_error(empty());
                                        
                                        size_t m = 0;
                                        while (m < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[m * 2] != rhs)
                                            ++m;
                                        
                                        if (m == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                            undefined_error(rhs);
                                        
                                        if (std::get<1>(* arrayv[k])[m * 2 + 1].empty() || is_string(std::get<1>(* arrayv[k])[m * 2 + 1]))
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        dst = new string[rhs.length() + 1];
                                        s = parse(dst, rhs);
                                        delete[] dst;
                                        
                                        if (s != 1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (is_string(rhs))
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        double b;
                                        if (is_symbol(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(0, 2);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(4, 2);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = stod(rhs);
                                        
                                        double a = stod(std::get<1>(* arrayv[k])[m * 2 + 1]);
                                        
                                        b = ((bao *)aov[j])->apply(a, b);
                                        
                                        rhs = rtrim(a);
                                        
                                        std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                    }
                                } else {
                                    double idx = stod(rhs);
                                    
                                    if (!is_int(idx))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    if (idx < 0 || idx >= std::get<1>(* arrayv[k]).size())
                                        range_error("index " + rtrim(idx) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                    
                                    if (std::get<1>(* arrayv[k])[(size_t)idx].empty() || is_string(std::get<1>(* arrayv[k])[(size_t)idx]))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    dst = new string[rhs.length() + 1];
                                    s = parse(dst, rhs);
                                    delete[] dst;
                                    
                                    if (s != 1)
                                        type_error(0, 2);
                                        //  array !=  double
                                    
                                    if (is_string(rhs))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    double b;
                                    if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = stod(rhs);
                                    
                                    double a = stod(std::get<1>(* arrayv[k])[(size_t)idx]);
                                    
                                    a = ((bao *)aov[j])->apply(a, b);
                                    
                                    rhs = rtrim(a);
                                    
                                    std::get<1>(* arrayv[k])[(size_t)idx] = rhs;
                                }
                            }
                        } else {
                            size_t k = i + 1;
                            while (k < n && data[k] == "(")
                                ++k;
                            
                            if (data[k] != uov[indexer_oi]->opcode()) {
                                delete[] dst;
                                operation_error();
                            }
                            
                            rhs = operands.pop();
                            
                            if (rhs.empty()) {
                                delete[] dst;
                                
                                if (is_dictionary(s, dst))
                                    null_error();
                                
                                type_error(4, 3);
                                //  string != int
                            }
                            
                            string* _dst = new string[rhs.length() + 1];
                            size_t _s = parse(_dst, rhs);
                            
                            if (_s != 1) {
                                delete[] dst;
                                type_error(0, 3);
                                //  array != int
                            }
                            
                            if (is_string(rhs)) {
                                if (!is_dictionary(s, dst)) {
                                    delete[] dst;
                                    type_error(0, 6);
                                    //  array != dictionary
                                }
                                
                                rhs = decode(rhs);
                                
                                if (rhs.empty()) {
                                    delete[] dst;
                                    undefined_error(rhs);
                                }
                                
                                rhs = encode(rhs);
                                
                                size_t k = 0;
                                while (k < (size_t)floor(s / 2) && dst[k * 2] != rhs)
                                    ++k;
                                
                                if (k == (size_t)floor(s / 2)) {
                                    delete[] dst;
                                    undefined_error(rhs);
                                }
                                
                                if (dst[k * 2 + 1].empty() || is_string(dst[k * 2 + 1]))
                                    type_error(4, 2);
                                    //  string != double
                                
                                rhs = operands.top();
                                
                                if (rhs.empty())
                                    type_error(4, 2);
                                    //  string != double
                                
                                _dst = new string[rhs.length() + 1];
                                _s = parse(_dst, rhs);
                                delete[] _dst;
                                
                                if (_s != 1)
                                    type_error(0, 2);
                                    //  array != double
                                
                                if (is_string(rhs))
                                    type_error(4, 2);
                                    //  string != double
                                
                                double b;
                                if (is_symbol(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (io_string(rhs) != -1)
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    b = get_number(rhs);
                                } else
                                    b = stod(rhs);
                                
                                double a = stod(dst[k * 2 + 1]);
                                    
                                a = ((bao *)aov[j])->apply(a, b);
                                
                                dst[k * 2 + 1] = rtrim(a);
                                
                                rhs = stringify(s, dst);
                                
                            } else if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(0, 3);
                                    //  array != int
                                
                                int k = io_string(rhs);
                                if (k == -1) {
                                    double idx = get_number(rhs);
                                    
                                    if (!is_int(idx)) {
                                        delete[] dst;
                                        type_error(2, 3);
                                        //  double != int
                                    }
                                    
                                    if (idx < 0 || idx >= s)
                                        range_error("index " + rtrim(idx) + ", count " + to_string(s));
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    _dst = new string[rhs.length() + 1];
                                    _s = parse(_dst, rhs);
                                    delete[] _dst;
                                    
                                    if (_s != 1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    double b;
                                    if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = stod(rhs);
                                    
                                    double a = stod(dst[(size_t)idx]);
                                        
                                    a = ((bao *)aov[j])->apply(a, b);
                                    
                                    dst[(size_t)idx] = rtrim(a);
                                    
                                    rhs = stringify(s, dst);
                                    
                                } else {
                                    if (!is_dictionary(s, dst)) {
                                        delete[] dst;
                                        type_error(0, 6);
                                        //  array != dictionary
                                    }
                                    
                                    rhs = std::get<1>(* stringv[k]);
                                    
                                    if (rhs.empty()) {
                                        delete[] dst;
                                        null_error();
                                    }
                                    
                                    if (rhs.length() == 2) {
                                        delete[] dst;
                                        undefined_error(rhs);
                                    }
                                    
                                    size_t l = 0;
                                    while (l < (size_t)floor(s / 2) && dst[l * 2] != rhs)
                                        ++l;
                                    
                                    if (l == (size_t)floor(s / 2)) {
                                        delete[] dst;
                                        undefined_error(rhs);
                                    }
                                    
                                    if (dst[l * 2 + 1].empty() || is_string(dst[l * 2 + 1]))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    _dst = new string[rhs.length() + 1];
                                    _s = parse(_dst, rhs);
                                    delete[] _dst;
                                    
                                    if (_s != 1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    double b;
                                    if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = stod(rhs);
                                    
                                    double a = stod(dst[l * 2 + 1]);
                                        
                                    a = ((bao *)aov[j])->apply(a, b);
                                    
                                    dst[l * 2 + 1] = rtrim(a);
                                    
                                    rhs = stringify(s, dst);
                                }
                            } else {
                                double idx = stod(rhs);
                                
                                if (!is_int(idx)) {
                                    delete[] dst;
                                    type_error(2, 3);
                                    //  double != int
                                }
                                
                                if (idx < 0 || idx >= s)
                                    range_error("index " + rtrim(idx) + ", count " + to_string(s));
                                
                                rhs = operands.top();
                                
                                if (rhs.empty())
                                    type_error(4, 2);
                                    //  string != double
                                
                                _dst = new string[rhs.length() + 1];
                                _s = parse(_dst, rhs);
                                delete[] _dst;
                                
                                if (_s != 1)
                                    type_error(0, 2);
                                    //  array != double
                                
                                if (is_string(rhs))
                                    type_error(4, 2);
                                    //  string != double
                                
                                double b;
                                if (is_symbol(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (io_string(rhs) != -1)
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    b = get_number(rhs);
                                } else
                                    b = stod(rhs);
                                
                                double a = stod(dst[(size_t)idx]);
                                    
                                a = ((bao *)aov[j])->apply(a, b);
                                
                                dst[(size_t)idx] = rtrim(a);
                                
                                rhs = stringify(s, dst);
                            }
                        }
                    } else {
                        rhs = operands.pop();
                        
                        if (rhs.empty())
                            type_error(4, 2);
                            //  string != double
                        
                        dst = new string[rhs.length() + 1];
                        s = parse(dst, rhs);
                        delete[] dst;
                        
                        if (s != 1)
                            type_error(0, 2);
                            //  array != double
                        
                        if (is_string(rhs))
                            type_error(4, 2);
                            //  string != double
                        
                        double a;
                        if (is_symbol(rhs)) {
                            if (io_array(rhs) != -1)
                                type_error(0, 2);
                                //  array != double
                            
                            if (io_string(rhs) != -1)
                                type_error(4, 2);
                                //  string != double
                            
                            a = get_number(rhs);
                        } else
                            a = stod(rhs);
                        
                        if (j < arithmetic::unary_count)
                            a = ((uao *)aov[j])->apply(a);
                        else {
                            rhs =   operands.pop();
                            
                            dst = new string[rhs.length() + 1];
                            s = parse(dst, rhs);
                            delete[] dst;
                            
                            if (s != 1)
                                type_error(0, 2);
                                //  array != double
                            
                            if (is_string(rhs))
                                type_error(4, 2);
                                //  string != double
                            
                            double b;
                            if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(0, 2);
                                    //  array != double
                                
                                if (io_string(rhs) != -1)
                                    type_error(4, 2);
                                    //  string != douoble
                                
                                b = get_number(rhs);
                            } else
                                b = stod(rhs);
                            
                            a = ((bao_t *)aov[j])->apply(a, b);
                        }
                        
                        rhs = rtrim(a);
                    }
                    
                    operands.push(rhs);
                }
            } else {
                string rhs;
                
                if (j < unary_count) {
                    rhs =   operands.pop();
                    
                    rhs = ((uuo *)uov[j])->apply(rhs);
                    
                    operands.push(rhs);
                } else if (j > unary_count) {
                    //  binary string operator
                    
                    string lhs =    operands.pop();
                    
                    bool flag;
                    
                    if (j == indexer_oi) {
                        if (i != 1) {
                            size_t k = i - 1;
                            while (k > 0 && data[k] == "(")
                                --k;
                            if (data[k] == "(")
                                flag = false;
                            else if (data[k] == uov[assignment_oi]->opcode() || data[k] == uov[assignment_oi + 1]->opcode())
                                flag = true;
                            else {
                                size_t l = 25;
                                while (l < aoc - 2 && data[k] != aov[l]->opcode())
                                    ++l;
                                
                                flag = l != aoc - 2;
                            }
                        } else
                            flag = false;
                    } else
                        flag = false;
                    
                    if (flag)
                        rhs = lhs;
                    else if (j == aggregate_oi ||
                             j == filter_oi ||
                             j == find_oi ||
                             j == find_index_oi ||
                             j == map_oi) {
                        string ctr = operands.pop();
                        
                        rhs = operands.pop();
                        rhs = ((tuo *)uov[j])->apply(lhs, ctr, rhs);
                    } else if (j == cell_oi ||
                               j == insert_oi) {
                        string ctr = operands.pop();
                        
                        ctr = evaluate(ctr);
                        rhs = operands.pop();
                        rhs = evaluate(rhs);
                        rhs = ((tuo *)uov[j])->apply(lhs, ctr, rhs);
                    } else if (j == splice_oi) {
                        string* valuev = new string[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            if (lhs.empty() || is_string(lhs))
                                type_error(4, 0);
                                //  string != array
                            
                            if (!is_symbol(lhs))
                                type_error(2, 0);
                                //  double != array
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                k = io_string(lhs);
                                if (k == -1) {
                                    if (is_defined(lhs))
                                        type_error(2, 0);
                                        //  double != array
                                    
                                    undefined_error(lhs);
                                }
                                
                                type_error(4, 0);
                                //  string != array
                            }
                            
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            k = io_array(lhs);
                            
                            if (ss::is_array(rhs))
                                type_error(0, 3);
                                //  array != int
                            
                            if (rhs.empty()) {
                                if (is_dictionary(std::get<1>(* arrayv[k])))
                                    null_error();
                                
                                type_error(4, 3);
                                //  string != int
                            }
                            
                            if (is_string(rhs)) {
                                size_t l = i + 1;   int q = 1;
                                do {
                                    if (data[l] == "(")
                                        ++q;
                                    else if (data[l] == ")")
                                        --q;
                                    ++l;
                                    if (!q)
                                        break;
                                } while (l < n);
                                
                                if (data[l] == uov[unary_count]->opcode())
                                    operation_error();
                                
                                if (!is_dictionary(std::get<1>(* arrayv[k])))
                                    type_error(4, 3);
                                    //  string != int
                                
                                rhs = decode(rhs);
                                
                                if (rhs.empty())
                                    undefined_error(empty());
                                
                                rhs = encode(rhs);
                                
                                l = 0;
                                while (l < floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[l * 2] != rhs)
                                    ++l;
                                
                                if (l == floor(std::get<1>(* arrayv[k]).size() / 2))
                                    undefined_error(rhs);
                                
                                if (std::get<2>(* arrayv[k]).first)
                                    write_error(lhs);
                                
                                valuev = new string[valuec = 2];
                                
                                for (size_t m = 0; m < valuec; ++m) {
                                    valuev[m] = std::get<1>(* arrayv[k])[l * 2];
                                    
                                    std::get<1>(* arrayv[k]).remove(l * 2);
                                }
                                
                                if (!std::get<1>(* arrayv[k]).size())
                                    std::get<1>(* arrayv[k]).push(empty());
                                    
                                rhs = stringify(valuec, valuev);
                                
                                delete[] valuev;
                            } else {
                                double s = stod(rhs);
                                
                                if (!is_int(s))
                                    type_error(2, 3);
                                    //  double != int
                                
                                if (s >= std::get<1>(* arrayv[k]).size())
                                    range_error("start " + rtrim(s) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                
                                size_t l = i + 1;   int q = 1;
                                do {
                                    if (data[l] == "(")
                                        ++q;
                                    else if (data[l] == ")")
                                        --q;
                                    ++l;
                                    if (!q)
                                        break;
                                } while (l < n);
                                
                                if (data[l] == uov[unary_count]->opcode()) {
                                    rhs = operands.pop();
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(4, 3);
                                        //  string != int
                                    
                                    double e = stod(rhs);
                                    if (!is_int(e))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    k = io_array(lhs);
                                    
                                    if (e < 0 || s + e > std::get<1>(* arrayv[k]).size())
                                        range_error("start " + rtrim(s) + ", length " + rtrim(e) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                    
                                    valuec = (size_t)e;
                                    valuev = new string[valuec];
                                    
                                    for (l = 0; l < valuec; ++l) {
                                        valuev[l] = std::get<1>(* arrayv[k])[(size_t)s];
                                        
                                        std::get<1>(* arrayv[k]).remove((size_t)s);
                                    }
                                    
                                    rhs = stringify(valuec, valuev);
                                    
                                    delete[] valuev;
                                } else {
                                    rhs = std::get<1>(* arrayv[k])[(size_t)s];
                                    
                                    std::get<1>(* arrayv[k]).remove((size_t)s);
                                }
                            }
                        } else {
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            if (ss::is_array(rhs)) {
                                delete[] valuev;
                                
                                type_error(0, 3);
                                //  array != int
                            }
                            
                            if (rhs.empty()) {
                                delete[] valuev;
                                
                                if (is_dictionary(valuec, valuev))
                                    null_error();
                                
                                type_error(4, 3);
                                //  string != int
                            }
                            
                            if (is_string(rhs)) {
                                if (!is_dictionary(valuec, valuev)) {
                                    delete[] valuev;
                                    
                                    type_error(4, 3);
                                    //  string != int
                                }
                                
                                size_t k = i + 1;   int q = 1;
                                do {
                                    if (data[k] == "(")
                                        ++q;
                                    else if (data[k] == ")")
                                        --q;
                                    ++k;
                                    if (!q)
                                        break;
                                } while (k < n);
                                
                                if (data[k] == uov[unary_count]->opcode()) {
                                    delete[] valuev;
                                    
                                    operation_error();
                                }
                                
                                rhs = decode(rhs);
                                
                                if (rhs.empty()) {
                                    delete[] valuev;
                                    undefined_error(empty());
                                }
                                
                                rhs = encode(rhs);
                                
                                k = 0;
                                while (k < valuec / 2 && valuev[k * 2] != rhs)
                                    ++k;
                                
                                if (k == valuec / 2) {
                                    delete[] valuev;
                                    undefined_error(rhs);
                                }
                                                            
                                for (size_t l = 0; l < 2; ++l) {
                                    for (size_t m = k * 2; m < valuec - 1; ++m)
                                        swap(valuev[m], valuev[m + 1]);
                                    
                                    --valuec;
                                }
                            } else {
                                double s = stod(rhs);
                                
                                if (!is_int(s)) {
                                    delete[] valuev;
                                    
                                    type_error(2, 3);
                                    //  double != int
                                }
                                
                                if (s >= valuec) {
                                    delete[] valuev;
                                    
                                    range_error("start " + rtrim(s) + ", count " + to_string(valuec));
                                }
                                
                                size_t k = i + 1;   int q = 1;
                                do {
                                    if (data[k] == "(")
                                        ++q;
                                    else if (data[k] == ")")
                                        --q;
                                    ++k;
                                    if (!q)
                                        break;
                                } while (k < n);
                                
                                if (data[k] == uov[unary_count]->opcode()) {
                                    rhs = operands.top();
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs)) {
                                        delete[] valuev;
                                        
                                        type_error(0, 3);
                                        //  array != int
                                    }
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] valuev;
                                        
                                        type_error(4, 3);
                                        //  string != int
                                    }
                                    
                                    double e = stod(rhs);
                                    if (!is_int(e)) {
                                        delete[] valuev;
                                        
                                        type_error(2, 3);
                                        //  double != int
                                    }
                                    
                                    if (e < 0 || s + e > valuec) {
                                        delete[] valuev;
                                        
                                        range_error("start " + rtrim(s) + ", length " + rtrim(e) + ", count " + to_string(valuec));
                                    }
                                    
                                    for (k = 0; k < e; ++k) {
                                        for (size_t l = s; l < valuec - 1; ++l)
                                            swap(valuev[l], valuev[l + 1]);
                                        
                                        --valuec;
                                    }
                                } else {
                                    for (k = s; k < valuec - 1; ++k)
                                        swap(valuev[k], valuev[k + 1]);
                                    
                                    --valuec;
                                }
                            }
                            
                            rhs = stringify(valuec, valuev);
                            
                            delete[] valuev;
                        }
                    } else if (j == slice_oi) {
                        string valuev[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                string text = decode(lhs);
                                
                                rhs = operands.pop();
                                rhs = evaluate(rhs);
                                
                                if (ss::is_array(rhs))
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(4, 3);
                                    //  string != int
                                
                                double s = stod(rhs);
                                
                                if (!is_int(s))
                                    type_error(2, 3);
                                    //  double != int
                                
                                if (s < 0 || s > text.length())
                                    range_error("start " + rtrim(s) + ", count " + to_string(text.length()));
                                
                                size_t k = i + 1;   int p = 1;
                                do {
                                    if (data[k] == "(")
                                        ++p;
                                    else if (data[k] == ")")
                                        --p;
                                    
                                    ++k;
                                    
                                    if (!p)
                                        break;
                                } while (k < n);
                                
                                if (data[k] == uov[unary_count]->opcode()) {
                                    rhs = operands.pop();
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(4, 3);
                                        //  string != int
                                    
                                    double l = stod(rhs);
                                    
                                    if (!is_int(l))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    if (l < 0 || s + l > text.length())
                                        range_error("start " + rtrim(s) + ", length " + rtrim(l) + ", count " + to_string(text.length()));
                                    
                                    rhs = text.substr((size_t)s, (size_t)l);
                                } else
                                    rhs = text.substr((size_t)s);
                                
                                rhs = encode(rhs);
                            } else {
                                if (!is_symbol(lhs))
                                    type_error(2, 0);
                                    //  double != array
                                
                                int k = io_array(lhs);
                                if (k == -1) {
                                    k = io_string(lhs);
                                    if (k == -1) {
                                        if (is_defined(lhs))
                                            type_error(2, 0);
                                            //  double != array
                                        
                                        undefined_error(lhs);
                                    }
                                    
                                    string text = std::get<1>(* stringv[k]);
                                    
                                    if (text.empty())
                                        null_error();
                                    
                                    text = decode(text);
                                    
                                    rhs = operands.pop();
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(4, 3);
                                        //  string != int
                                    
                                    double s = stod(rhs);
                                    
                                    if (!is_int(s))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    if (s < 0 || s > text.length())
                                        range_error("start " + rtrim(s) + ", count " + to_string(text.length()));
                                    
                                    size_t l = i + 1;   int q = 1;
                                    do {
                                        if (data[l] == "(")
                                            ++q;
                                        else if (data[l] == ")")
                                            --q;
                                        ++l;
                                        if (!q)
                                            break;
                                    } while (l < n);
                                    
                                    if (data[l] == uov[unary_count]->opcode()) {
                                        rhs = operands.pop();
                                        rhs = evaluate(rhs);
                                        
                                        if (ss::is_array(rhs))
                                            type_error(0, 3);
                                            //  array != int
                                        
                                        if (rhs.empty() || is_string(rhs))
                                            type_error(4, 3);
                                            //  string != int
                                        
                                        double l = stod(rhs);
                                        
                                        if (!is_int(l))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (l < 0 || s + l > text.length())
                                            range_error("start " + rtrim(s) + ", length " + rtrim(l) + ", count " + to_string(text.length()));
                                        
                                        rhs = text.substr((size_t)s, (size_t)l);
                                    } else
                                        rhs = text.substr((size_t)s);
                                    
                                    rhs = encode(rhs);
                                    
                                    std::get<2>(* stringv[io_string(lhs)]).second = true;
                                } else {
                                    array<string> arr = std::get<1>(* arrayv[k]);
                                    
                                    rhs = operands.pop();
                                    rhs = evaluate(rhs);
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(4, 3);
                                        //  string != int
                                    
                                    double s = stod(rhs);
                                    
                                    if (!is_int(s))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    if (s > arr.size())
                                        range_error("start " + rtrim(s) + ", count " + to_string(arr.size()));
                                    
                                    size_t l = i + 1;   int q = 1;
                                    do {
                                        if (data[l] == "(")
                                            ++q;
                                        else if (data[l] == ")")
                                            --q;
                                        ++l;
                                        if (!q)
                                            break;
                                    } while (l < n);
                                    
                                    if (data[l] == uov[unary_count]->opcode()) {
                                        rhs = operands.pop();
                                        rhs = evaluate(rhs);
                                        
                                        if (ss::is_array(rhs))
                                            type_error(0, 3);
                                            //  array != int
                                        
                                        if (rhs.empty() || is_string(rhs))
                                            type_error(4, 3);
                                            //  string != int
                                        
                                        double e = stod(rhs);
                                        
                                        if (!is_int(e))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (s + e > arr.size())
                                            range_error("start " + rtrim(s) + ", length " + rtrim(e) + ", count " + to_string(arr.size()));
                                        
                                        rhs = stringify(arr, (size_t)s, (size_t)e);
                                    } else
                                        rhs = stringify(arr, (size_t)s);
                                    
                                    std::get<2>(* arrayv[io_array(lhs)]).second = true;
                                }
                            }
                        } else {
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            if (ss::is_array(rhs))
                                type_error(0, 3);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(4, 3);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(2, 3);
                                //  double != int
                            
                            if (s > valuec)
                                range_error("start " + rtrim(s) + ", count " + to_string(valuec));
                            
                            size_t k = i + 1;   int q = 1;
                            do {
                                if (data[k] == "(")
                                    ++q;
                                else if (data[k] == ")")
                                    --q;
                                ++k;
                                
                                if (!q)
                                    break;
                            } while (k < n);

                            if (data[k] == uov[unary_count]->opcode()) {
                                rhs = operands.pop();
                                rhs = evaluate(rhs);
                                
                                if (ss::is_array(rhs))
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(4, 3);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(2, 3);
                                    //  double != int
                                
                                if (s + e > valuec)
                                    range_error("start " + rtrim(s) + ", length " + rtrim(e) + ", count " + to_string(valuec));
                                
                                string _valuev[(size_t)e];
                                
                                for (size_t k = 0; k < (size_t)e; ++k)
                                    _valuev[k] = valuev[(size_t)s + k];
                                
                                rhs = stringify((size_t)e, _valuev);
                            } else {
                                string _valuev[valuec - (size_t)s];
                                
                                for (size_t k = 0; k < valuec - (size_t)s; ++k)
                                    _valuev[k] = valuev[(size_t)s + k];
                                
                                rhs = stringify(valuec - (size_t)s, _valuev);
                            }
                        }
                    } else if (j == substr_oi) {
                        if (ss::is_array(lhs))
                            type_error(0, 4);
                            //  array != string
                        
                        if (lhs.empty())
                            null_error();
                        
                        if (!is_string(lhs)) {
                            if (!is_symbol(lhs))
                                type_error(2, 4);
                                //  double != string
                            
                            if (io_array(lhs) != -1)
                                type_error(0, 4);
                                //  array != string
                            
                            int k = io_string(lhs);
                            if (k == -1) {
                                if (is_defined(lhs))
                                    type_error(2, 4);
                                    //  double != string
                                
                                undefined_error(lhs);
                            }
                            
                            lhs = std::get<1>(* stringv[k]);
                            
                            if (lhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[k]).second = true;
                        }
                        
                        lhs = decode(lhs);
                        
                        rhs = operands.pop();
                        rhs = evaluate(rhs);
                        
                        if (ss::is_array(rhs))
                            type_error(0, 3);
                            //  array != int
                        
                        if (rhs.empty() || is_string(rhs))
                            type_error(4, 3);
                            //  string != int
                        
                        double s = stod(rhs);
                        
                        if (!is_int(s))
                            type_error(2, 3);
                            //  double != int
                        
                        if (s < 0 || s > lhs.length())
                            range_error("start " + rtrim(s) + ", count " + to_string(lhs.length()));
                        
                        size_t k = i + 1;   int q = 1;
                        do {
                            if (data[k] == "(")
                                ++q;
                            else if (data[k] == ")")
                                --q;
                            ++k;
                            
                            if (!q)
                                break;
                        } while (k < n);
                        
                        if (data[k] == uov[unary_count]->opcode()) {
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            if (ss::is_array(rhs))
                                type_error(0, 3);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(4, 3);
                                //  string != int
                                
                            double e = stod(rhs);
                            
                            if (!is_int(e))
                                type_error(2, 3);
                                //  double != int
                            
                            if (e < s || e > lhs.length())
                                range_error("start " + rtrim(s) + ", end " + rtrim(e) + ", count " + to_string(lhs.length()));
                            
                            rhs = lhs.substr((size_t)s, (size_t)(e - s));
                        } else
                            rhs = lhs.substr((size_t)s);
                        
                        rhs = encode(rhs);
                        
                    } else if (j == tospliced_oi) {
                        string* valuev = new string[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            delete[] valuev;
                            
                            if (!is_symbol(lhs))
                                operation_error();
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                if (io_string(lhs) != -1 || is_defined(lhs))
                                    operation_error();
                                
                                undefined_error(lhs);
                            }
                            
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            if (ss::is_array(rhs))
                                type_error(0, 3);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(4, 3);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(2, 3);
                                //  double != int
                            
                            k = io_array(lhs);
                            
                            if (s < 0 || s > std::get<1>(* arrayv[k]).size())
                                range_error("start " + rtrim(s) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                            
                            size_t l = i + 1;   int p = 1;
                            do {
                                if (data[l] == "(")
                                    ++p;
                                else if (data[l] == ")")
                                    --p;
                                
                                ++l;
                                
                                if (!p)
                                    break;
                                
                            } while (k < n);
                            
                            if (data[l] == uov[unary_count]->opcode()) {
                                rhs = operands.pop();
                                rhs = evaluate(rhs);
                                
                                if (ss::is_array(rhs))
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(4, 3);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(2, 3);
                                    //  double != int
                                
                                k = io_array(lhs);
                                
                                if (e < 0 || s + e > std::get<1>(* arrayv[k]).size())
                                    range_error("start " + rtrim(s) + " length " + rtrim(e) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                
                                valuev = new string[(size_t)(std::get<1>(* arrayv[k]).size() - e)];
                                valuec = 0;
                                
                                for (size_t m = 0; m < s; ++m)
                                    valuev[valuec++] = std::get<1>(* arrayv[k])[m];
                                
                                for (size_t m = s + e; m < std::get<1>(* arrayv[k]).size(); ++m)
                                    valuev[valuec++] = std::get<1>(* arrayv[k])[m];
                                
                                std::get<2>(* arrayv[k]).second = true;
                            } else {
                                valuev = new string[std::get<1>(* arrayv[k]).size() - 1];
                                valuec = 0;
                                
                                for (size_t m = 0; m < s; ++m)
                                    valuev[valuec++] = std::get<1>(* arrayv[k])[m];
                                
                                for (size_t m = s + 1; m < std::get<1>(* arrayv[k]).size(); ++m)
                                    valuev[valuec++] = std::get<1>(* arrayv[k])[m];
                                
                                std::get<2>(* arrayv[k]).second = true;
                            }
                        } else {
                            rhs = operands.pop();
                            rhs = evaluate(rhs);
                            
                            if (ss::is_array(rhs))
                                type_error(0, 3);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(4, 3);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(2, 3);
                                //  double != int
                            
                            if (s < 0 || s > valuec)
                                range_error("start " + rtrim(s) + ", count " + to_string(valuec));
                            
                            size_t k = i + 1;   int p = 1;
                            do {
                                if (data[k] == "(")
                                    ++p;
                                else if (data[k] == ")")
                                    --p;
                                
                                ++k;
                                
                                if (!p)
                                    break;
                                
                            } while (k < n);
                            
                            if (data[k] == uov[unary_count]->opcode()) {
                                rhs = operands.pop();
                                rhs = evaluate(rhs);
                                
                                if (ss::is_array(rhs))
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(4, 3);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(2, 3);
                                    //  double != int
                                
                                if (e < 0 || s + e > valuec)
                                    range_error("start " + rtrim(s) + ", length " + rtrim(e) + ", count " + to_string(valuec));

                                for (size_t l = 0; l < e; ++l) {
                                    for (size_t m = s; m < valuec - 1; ++m)
                                        swap(valuev[m], valuev[m + 1]);
                                    
                                    --valuec;
                                }
                            } else {
                                for (size_t l = s; l < valuec - 1; ++l)
                                    swap(valuev[l], valuev[l + 1]);
                                
                                --valuec;
                            }
                        }
                        
                        rhs = stringify(valuec, valuev);
                        
                        delete[] valuev;
                    } else if (j == assignment_oi) {
                        if (lhs.empty())
                            operation_error();
                        
                        string* v = new string[lhs.length() + 1];
                        size_t p = parse(v, lhs);
                        
                        if (p == 1) {
                            delete[] v;
                            
                            if (!is_symbol(lhs))
                                operation_error();
                            
                            rhs = operands.pop();
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                k = io_string(lhs);
                                if (k == -1) {
                                    double d = get_number(lhs);
                                    
                                    size_t l = i + 1;
                                    while (l < n && data[l] == "(")
                                        ++l;
                                    
                                    if (data[l] == uov[indexer_oi]->opcode())
                                        operation_error();
                                    
                                    if (rhs.empty())
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    v = new string[rhs.length() + 1];
                                    p = parse(v, rhs);
                                    delete[] v;
                                    
                                    if (p != 1)
                                        type_error(0, 2);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(4, 2);
                                        //  string != double
                                    
                                    if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 2);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(4, 2);
                                            //  string != double
                                        
                                        d += get_number(rhs);
                                    } else
                                        d += stod(rhs);
                                    
                                    set_number(lhs, d);
                                    
                                    rhs = rtrim(d);
                                } else {
                                    size_t l = i + 1;
                                    while (l < n && data[l] == "(")
                                        ++l;
                                    
                                    if (data[l] == uov[indexer_oi]->opcode())
                                        operation_error();
                                    
                                    string tmp = rhs;
                                    
                                    rhs = std::get<1>(* stringv[k]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[k]).second = true;
                                    
                                    rhs = decode(rhs);
                                    
                                    swap(rhs, tmp);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    v = new string[rhs.length() + 1];
                                    p = parse(v, rhs);
                                    delete[] v;
                                    
                                    if (p != 1)
                                        type_error(0, 4);
                                        //  array != string
                                    
                                    if (is_string(rhs))
                                        rhs = decode(rhs);
                                    else if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 4);
                                            //  array != string
                                        
                                        int j = io_string(rhs);
                                        if (j == -1)
                                            rhs = rtrim(get_number(rhs));
                                        else {
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            std::get<2>(* stringv[j]).second = true;
                                            
                                            rhs = decode(rhs);
                                        }
                                    } else
                                        rhs = rtrim(stod(rhs));
                                    
                                    rhs = tmp + rhs;
                                    rhs = encode(rhs);
                                    
                                    set_string(lhs, rhs);
                                }
                            } else {
                                size_t l = i + 1;
                                while (l < n && data[l] == "(")
                                    ++l;
                                
                                if (data[l] == uov[indexer_oi]->opcode()) {
                                    if (rhs.empty()) {
                                        if (is_dictionary(std::get<1>(* arrayv[k])))
                                            null_error();
                                        
                                        type_error(4, 3);
                                        //  string != int
                                    }
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (is_string(rhs)) {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(4, 3);
                                            //  string != int
                                        
                                        rhs = decode(rhs);
                                        
                                        if (rhs.empty())
                                            undefined_error(encode(rhs));
                                        
                                        rhs = encode(rhs);
                                        
                                        size_t m = 0;
                                        while (m < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[m * 2] != rhs)
                                            ++m;
                                        
                                        if (m == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                            undefined_error(rhs);
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        lhs = std::get<1>(* arrayv[k])[m * 2 + 1];
                                        
                                        if (lhs.empty())
                                            null_error();
                                            
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty())
                                            null_error();
                                        
                                        if (is_string(lhs)) {
                                            if (ss::is_array(rhs))
                                                type_error(0, 4);
                                                //  array != string
                                            
                                            lhs = decode(lhs);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (is_string(rhs))
                                                rhs = decode(rhs);
                                            
                                            else if (is_symbol(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(0, 4);
                                                    //  array != string
                                                
                                                int q = io_string(rhs);
                                                if (q == -1)
                                                    rhs = rtrim(get_number(rhs));
                                                else {
                                                    rhs = std::get<1>(* stringv[q]);
                                                    
                                                    if (rhs.empty())
                                                        null_error();
                                                    
                                                    std::get<2>(* stringv[q]).second = true;
                                                    
                                                    rhs = decode(rhs);
                                                }
                                            } else
                                                rhs = rtrim(stod(rhs));
                                            
                                            rhs = lhs + rhs;
                                            rhs = encode(rhs);
                                            
                                            std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        } else {
                                            double d = stod(lhs);
                                            
                                            if (p != 1)
                                                type_error(0, 2);
                                                //  array != double
                                            
                                            if (is_string(rhs))
                                                type_error(4, 2);
                                                //  string != double
                                                
                                            if (is_symbol(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(0, 2);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(4, 2);
                                                    //  string != double
                                                
                                                d += get_number(rhs);
                                            } else
                                                d += stod(rhs);
                                            
                                            rhs = rtrim(d);
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        }
                                    } else if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 3);
                                            //  array != int
                                        
                                        int m = io_string(rhs);
                                        if (m == -1) {
                                            double index = get_number(rhs);
                                            
                                            if (!is_int(index))
                                                type_error(2, 3);
                                                //  double != int
                                            
                                            if (index < 0 || index >= std::get<1>(* arrayv[k]).size())
                                                range_error("index " + rtrim(index) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            lhs = std::get<1>(* arrayv[k])[(size_t)index];
                                            
                                            if (lhs.empty())
                                                null_error();
                                                
                                            rhs = operands.pop();
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            v = new string[rhs.length() + 1];
                                            p = parse(v, rhs);
                                            delete[] v;
                                            
                                            if (is_string(lhs)) {
                                                if (p != 1)
                                                    type_error(0, 4);
                                                    //  array != string
                                                
                                                lhs = decode(lhs);
                                                
                                                if (rhs.empty())
                                                    null_error();
                                                
                                                if (is_string(rhs))
                                                    rhs = decode(rhs);
                                                
                                                else if (is_symbol(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(0, 4);
                                                        //  array != string
                                                    
                                                    l = io_string(rhs);
                                                    if (l == -1)
                                                        rhs = rtrim(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[l]);
                                                        
                                                        if (rhs.empty())
                                                            null_error();
                                                        
                                                        std::get<2>(* stringv[l]).second = true;
                                                        
                                                        rhs = decode(rhs);
                                                    }
                                                } else
                                                    rhs = rtrim(stod(rhs));
                                                
                                                rhs = lhs + rhs;
                                                rhs = encode(rhs);
                                                
                                                std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                            } else {
                                                double d = stod(std::get<1>(* arrayv[k])[(size_t)index]);
                                                
                                                if (p != 1)
                                                    type_error(0, 2);
                                                    //  array != double
                                                
                                                if (is_string(rhs))
                                                    type_error(4, 2);
                                                    //  string != double
                                                    
                                                if (is_symbol(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(0, 2);
                                                        //  array != double
                                                    
                                                    if (io_string(rhs) != -1)
                                                        type_error(4, 2);
                                                        //  string != double
                                                    
                                                    d += get_number(rhs);
                                                } else
                                                    d += stod(rhs);
                                                
                                                rhs = rtrim(d);
                                                
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                            }
                                        } else {
                                            if (!is_dictionary(std::get<1>(* arrayv[k])))
                                                type_error(4, 3);
                                                //  string != int
                                            
                                            rhs = std::get<1>(* stringv[m]);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (rhs.length() == 2)
                                                undefined_error(rhs);
                                            
                                            size_t q = 0;
                                            while (q < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[q * 2] != rhs)
                                                ++q;
                                            
                                            if (q == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                                undefined_error(rhs);
                                            
                                            lhs = std::get<1>(* arrayv[k])[q * 2 + 1];
                                            
                                            if (lhs.empty())
                                                null_error();
                                                
                                            rhs = operands.pop();
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            v = new string[rhs.length() + 1];
                                            p = parse(v, rhs);
                                            delete[] v;
                                            
                                            if (is_string(lhs)) {
                                                if (p != 1)
                                                    type_error(0, 4);
                                                    //  array != string
                                                
                                                lhs = decode(lhs);
                                                
                                                if (rhs.empty())
                                                    null_error();
                                                
                                                if (is_string(rhs))
                                                    rhs = decode(rhs);
                                                
                                                else if (is_symbol(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(0, 4);
                                                        //  array != string
                                                    
                                                    size_t r = io_string(rhs);
                                                    if (r == -1)
                                                        rhs = rtrim(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[r]);
                                                        
                                                        if (rhs.empty())
                                                            null_error();
                                                        
                                                        std::get<2>(* stringv[r]).second = true;
                                                        
                                                        rhs = decode(rhs);
                                                    }
                                                } else
                                                    rhs = rtrim(stod(rhs));
                                                
                                                rhs = lhs + rhs;
                                                rhs = encode(rhs);
                                                
                                                std::get<1>(* arrayv[k])[q * 2 + 1] = rhs;
                                            } else {
                                                double d = stod(lhs);
                                                
                                                if (p != 1)
                                                    type_error(0, 2);
                                                    //  array != double
                                                
                                                if (is_string(rhs))
                                                    type_error(4, 2);
                                                    //  string != double
                                                    
                                                if (is_symbol(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(0, 2);
                                                        //  array != double
                                                    
                                                    if (io_string(rhs) != -1)
                                                        type_error(4, 2);
                                                        //  string != double
                                                    
                                                    d += get_number(rhs);
                                                } else
                                                    d += stod(rhs);
                                                
                                                rhs = rtrim(d);
                                                
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                std::get<1>(* arrayv[k])[q * 2 + 1] = rhs;
                                            }
                                        }
                                    } else {
                                        double index = stod(rhs);
                                        
                                        if (!is_int(index))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (index < 0 || index >= std::get<1>(* arrayv[k]).size())
                                            range_error("index " + rtrim(index) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        lhs = std::get<1>(* arrayv[k])[(size_t)index];
                                        
                                        if (lhs.empty())
                                            null_error();
                                            
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty())
                                            null_error();
                                        
                                        v = new string[rhs.length() + 1];
                                        p = parse(v, rhs);
                                        delete[] v;
                                        
                                        if (is_string(lhs)) {
                                            if (p != 1)
                                                type_error(0, 4);
                                                //  array != string
                                            
                                            lhs = decode(lhs);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (is_string(rhs))
                                                rhs = decode(rhs);
                                            
                                            else if (is_symbol(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(0, 4);
                                                    //  array != string
                                                
                                                int m = io_string(rhs);
                                                if (m == -1)
                                                    rhs = rtrim(get_number(rhs));
                                                else {
                                                    rhs = std::get<1>(* stringv[m]);
                                                    
                                                    if (rhs.empty())
                                                        null_error();
                                                    
                                                    std::get<2>(* stringv[m]).second = true;
                                                    
                                                    rhs = decode(rhs);
                                                }
                                            } else
                                                rhs = rtrim(stod(rhs));
                                            
                                            rhs = lhs + rhs;
                                            rhs = encode(rhs);
                                            
                                            std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                        } else {
                                            double d = stod(std::get<1>(* arrayv[k])[(size_t)index]);
                                            
                                            if (p != 1)
                                                type_error(0, 2);
                                                //  array != double
                                            
                                            if (is_string(rhs))
                                                type_error(4, 2);
                                                //  string != double
                                                
                                            if (is_symbol(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(0, 2);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(4, 2);
                                                    //  string != double
                                                
                                                d += get_number(rhs);
                                            } else
                                                d += stod(rhs);
                                            
                                            rhs = rtrim(d);
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                        }
                                    }
                                } else {
                                    if (rhs.empty()) {
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        std::get<1>(* arrayv[k]).push(rhs);
                                    } else {
                                        v = new string[rhs.length() + 1];
                                        p = parse(v, rhs);
                                        
                                        if (p == 1) {
                                            delete[] v;
                                            
                                            if (is_string(rhs)) {
                                                rhs = decode(rhs);
                                                rhs = encode(rhs);
                                                
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                std::get<2>(* arrayv[k]).second = true;
                                                std::get<1>(* arrayv[k]).push(rhs);
                                                
                                            } else if (is_symbol(rhs)) {
                                                int l = io_array(rhs);
                                                if (l == -1) {
                                                    l = io_string(rhs);
                                                    if (l == -1)
                                                        rhs = rtrim(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[l]);
                                                        std::get<2>(* stringv[l]).second = true;
                                                    }
                                                    
                                                    if (std::get<2>(* arrayv[k]).first)
                                                        write_error(lhs);
                                                    
                                                    std::get<2>(* arrayv[k]).second = true;
                                                    std::get<1>(* arrayv[k]).push(rhs);
                                                } else {
                                                    if (std::get<2>(* arrayv[k]).first)
                                                        write_error(lhs);
                                                    
                                                    rhs = stringify(std::get<1>(* arrayv[l]));
                                                    
                                                    std::get<2>(* arrayv[l]).second = true;
                                                    
                                                    size_t m = std::get<1>(* arrayv[l]).size();
                                                    
                                                    for (size_t q = 0; q < m; ++q)
                                                        std::get<1>(* arrayv[k]).push(std::get<1>(* arrayv[l])[q]);
                                                }
                                            } else {
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                rhs = rtrim(stod(rhs));
                                                
                                                std::get<2>(* arrayv[k]).second = true;
                                                std::get<1>(* arrayv[k]).push(rhs);
                                            }
                                        } else {
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                                                                        
                                            for (size_t l = 0; l < p; ++l)
                                                std::get<1>(* arrayv[k]).push(v[l]);
                                            
                                            delete[] v;
                                        }
                                    }
                                    
                                    rhs = to_string(std::get<1>(* arrayv[k]).size());
                                }
                            }
                        } else {
                            size_t k = i + 1;
                            while (k < n && data[k] == "(")
                                ++k;
                            
                            if (data[k] != uov[indexer_oi]->opcode())
                                operation_error();
                            
                            rhs = operands.pop();
                            
                            //  empty indexer argument
                            if (rhs.empty()) {
                                delete[] v;
                                
                                //  null dictionary key
                                if (is_dictionary(p, v))
                                    null_error();
                                
                                type_error(4, 3);
                                //  string != int
                            }
                            
                            if (ss::is_array(rhs)) {
                                delete[] v;
                                type_error(0, 3);
                                //  array != int
                            }
                            
                            if (is_string(rhs)) {
                                if (!is_dictionary(p, v)) {
                                    delete[] v;
                                    type_error(0, 6);
                                    //  array != dictionary
                                }
                                
                                rhs = decode(rhs);
                                
                                if (rhs.empty()) {
                                    delete[] v;
                                    undefined_error(empty());
                                }
                                
                                rhs = encode(rhs);
                                
                                size_t l = 0;
                                while (l < (size_t)floor(p / 2) && v[l * 2] != rhs)
                                    ++l;
                                
                                if (l == (size_t)floor(p / 2)) {
                                    delete[] v;
                                    undefined_error(rhs);
                                }
                                
                                if (v[l * 2 + 1].empty()) {
                                    delete[] v;
                                    null_error();
                                }
                                
                                if (is_string(v[l * 2 + 1])) {
                                    string text = v[l * 2 + 1];
                                    
                                    text = decode(text);
                                    
                                    rhs = operands.pop();
                                    
                                    rhs = element(rhs);
                                    rhs = decode(rhs);
                                    
                                    rhs = text + rhs;
                                    rhs = encode(rhs);
                                    
                                    v[l * 2 + 1] = rhs;
                                } else {
                                    double number = stod(v[l * 2 + 1]);
                                    
                                    rhs = operands.pop();
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] v;
                                        type_error(4, 2);
                                        //  string != double
                                    }
                                    
                                    number += stod(rhs);
                                    
                                    v[l * 2 + 1] = rtrim(number);
                                }
                            } else if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(0, 3);
                                    //  array != int
                                
                                int l = io_string(rhs);
                                if (l == -1) {
                                    double idx = get_number(rhs);
                                    
                                    if (!is_int(idx)) {
                                        delete[] v;
                                        type_error(2, 3);
                                        //  double != int
                                    }
                                    
                                    if (idx < 0 || idx >= p)
                                        range_error("index " + rtrim(p) + ", count " + to_string(p));
                                    
                                    if (v[(size_t)idx].empty()) {
                                        delete[] v;
                                        null_error();
                                    }
                                    
                                    if (is_string(v[(size_t)idx])) {
                                        string text = v[(size_t)idx];
                                        
                                        text = decode(text);
                                        
                                        rhs = operands.pop();
                                        
                                        rhs = element(rhs);
                                        rhs = decode(rhs);
                                        
                                        rhs = text + rhs;
                                        rhs = encode(rhs);
                                        
                                        v[(size_t)idx] = rhs;
                                    } else {
                                        double number = stod(v[(size_t)idx]);
                                        
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty() || is_string(rhs)) {
                                            delete[] v;
                                            type_error(4, 2);
                                            //  string != double
                                        }
                                        
                                        number += stod(rhs);
                                        
                                        v[(size_t)idx] = rtrim(number);
                                    }
                                } else {
                                    if (!is_dictionary(p, v)) {
                                        delete[] v;
                                        type_error(0, 6);
                                        //  array != dictionary
                                    }
                                    
                                    rhs = std::get<1>(* stringv[l]);
                                    
                                    if (rhs.empty()) {
                                        delete[] v;
                                        null_error();
                                    }
                                    
                                    if (rhs.length() == 2) {
                                        delete[] v;
                                        undefined_error(empty());
                                    }
                                    
                                    size_t m = 0;
                                    while (m < (size_t)floor(p / 2) && v[m * 2] != rhs)
                                        ++m;
                                    
                                    if (m == (size_t)floor(p / 2)) {
                                        delete[] v;
                                        undefined_error(rhs);
                                    }
                                    
                                    if (v[m * 2 + 1].empty()) {
                                        delete[] v;
                                        null_error();
                                    }
                                    
                                    if (is_string(v[m * 2 + 1])) {
                                        string text = v[m * 2 + 1];
                                        
                                        text = decode(text);
                                        
                                        rhs = operands.pop();
                                        
                                        rhs = element(rhs);
                                        rhs = decode(rhs);
                                        
                                        rhs = text + rhs;
                                        rhs = encode(rhs);
                                        
                                        v[m * 2 + 1] = rhs;
                                    } else {
                                        double number = stod(v[m * 2 + 1]);
                                        
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty() || is_string(rhs)) {
                                            delete[] v;
                                            type_error(4, 2);
                                            //  string != double
                                        }
                                        
                                        number += stod(rhs);
                                        
                                        v[m * 2 + 1] = rtrim(number);
                                    }
                                }
                            } else {
                                double idx = stod(rhs);
                                
                                if (!is_int(idx)) {
                                    delete[] v;
                                    type_error(2, 3);
                                    //  double != int
                                }
                                
                                if (idx < 0 || idx >= p)
                                    range_error("index " + rtrim(p) + ", count " + to_string(p));
                                
                                if (v[(size_t)idx].empty()) {
                                    delete[] v;
                                    null_error();
                                }
                                
                                if (is_string(v[(size_t)idx])) {
                                    string text = v[(size_t)idx];
                                    
                                    text = decode(text);
                                    
                                    rhs = operands.pop();
                                    
                                    rhs = element(rhs);
                                    rhs = decode(rhs);
                                    
                                    rhs = text + rhs;
                                    rhs = encode(rhs);
                                    
                                    v[(size_t)idx] = rhs;
                                } else {
                                    double number = stod(v[(size_t)idx]);
                                    
                                    rhs = operands.pop();
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] v;
                                        type_error(4, 2);
                                        //  string != double
                                    }
                                    
                                    number += stod(rhs);
                                    
                                    v[(size_t)idx] = rtrim(number);
                                }
                            }
                            
                            rhs = stringify(p, v);
                            delete[] v;
                        }
                    } else if (j == assignment_oi + 1) {
                        if (lhs.empty())
                            operation_error();
                        
                        string* v = new string[lhs.length() + 1];
                        size_t p = parse(v, lhs);
                        
                        rhs = operands.pop();
                        
                        size_t k = i + 1;
                        while (k < n && data[k] == "(")
                            ++k;
                        
                        if (data[k] == uov[indexer_oi]->opcode()) {
                            ++k;
                            while (k < n && data[k] == "(")
                                ++k;
                            
                            if (p == 1) {
                                if (!is_symbol(lhs))
                                    operation_error();
                                
                                int k = io_array(lhs);
                                if (k == -1) {
                                    if (io_string(lhs) != -1)
                                        operation_error();
                                    
                                    if (is_defined(lhs))
                                        type_error(2, 4);
                                        //  double != string
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (ss::is_array(rhs))
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (is_string(rhs)) {
                                        rhs = decode(rhs);
                                        
                                        if (rhs.empty())
                                            undefined_error(empty());
                                        
                                        rhs = encode(rhs);
                                        
                                        string ctr = rhs;
                                        
                                        rhs = element(operands.pop());
                                        
                                        set_array(lhs, 0, ctr);
                                        set_array(lhs, 1, rhs);
                                        
                                    } else if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 3);
                                            //  array != int
                                        
                                        k = io_string(rhs);
                                        if (k == -1) {
                                            double idx = get_number(rhs);
                                            
                                            if (!is_int(idx))
                                                type_error(2, 3);
                                                //  double != int
                                            
                                            if (idx)
                                                range_error(rtrim(idx));
                                                //  revisit
                                            
                                            rhs = operands.pop();
                                            
                                            rhs = element(rhs);
                                            
                                            set_array(lhs, 0, rhs);
                                        } else {
                                            rhs = std::get<1>(* stringv[k]);
                                            
                                            if (rhs.empty())
                                                null_error();

                                            if (rhs.length() == 2)
                                                undefined_error(empty());

                                            string key = rhs;
                                            
                                            rhs = operands.pop();
                                            
                                            rhs = element(rhs);
                                            
                                            set_array(lhs, 0, key);
                                            set_array(lhs, 1, rhs);
                                        }
                                    } else {
                                        double idx = stod(rhs);
                                        
                                        if (!is_int(idx))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (idx)
                                            range_error(rtrim(idx));
                                            //  revisit
                                        
                                        rhs = operands.pop();
                                        
                                        rhs = element(rhs);
                                        
                                        set_array(lhs, 0, rhs);
                                    }
                                } else {
                                    if (rhs.empty()) {
                                        if (is_dictionary(std::get<1>(* arrayv[k])))
                                            null_error();
                                        
                                        type_error(4, 3);
                                        //  string != int
                                    }
                                    
                                    v = new string[rhs.length() + 1];
                                    p = parse(v, rhs);
                                    delete[] v;
                                    
                                    if (p != 1)
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    if (is_string(rhs)) {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(0, 6);
                                            //  array != table
                                        
                                        rhs = decode(rhs);
                                        
                                        if (rhs.empty())
                                            undefined_error(encode(rhs));
                                        
                                        string ctr = rhs;
                                        ctr = encode(ctr);
                                        
                                        rhs = operands.pop();
                                        
                                        rhs = element(rhs);
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        size_t l = 0;
                                        while (l < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[l * 2] != ctr)
                                            ++l;
                                        
                                        if (l == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2)) {
                                            std::get<1>(* arrayv[k]).push(ctr);
                                            std::get<1>(* arrayv[k]).push(rhs);
                                        } else
                                            std::get<1>(* arrayv[k])[l * 2 + 1] = rhs;
                                    } else if (is_symbol(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(0, 3);
                                            //  array != int
                                        
                                        int l = io_string(rhs);
                                        if (l == -1) {
                                            double d = get_number(rhs);
                                            
                                            if (!is_int(d))
                                                type_error(2, 3);
                                                //  double != int
                                            
                                            if (d < 0 || d > std::get<1>(* arrayv[k]).size())
                                                range_error("index " + rtrim(d) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                            
                                            rhs = operands.pop();
                                            
                                            rhs = element(rhs);
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            set_array(lhs, (int)d, rhs);
                                        } else {
                                            string ctr = std::get<1>(* stringv[l]);
                                            
                                            if (ctr.empty())
                                                null_error();
                                            
                                            if (ctr.length() == 2)
                                                undefined_error(ctr);
                                            
                                            rhs = element(operands.pop());
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            size_t m = 0;
                                            while (m < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[m * 2] != ctr)
                                                ++m;
                                            
                                            if (m == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2)) {
                                                std::get<1>(* arrayv[k]).push(ctr);
                                                std::get<1>(* arrayv[k]).push(rhs);
                                            } else
                                                std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        }
                                    } else {
                                        double num = stod(rhs);
                                        
                                        if (!is_int(num))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (num < 0 || num > std::get<1>(* arrayv[k]).size())
                                            range_error("index " + rtrim(num) + ", count " + to_string(std::get<1>(* arrayv[k]).size()));
                                        
                                        rhs = element(operands.pop());
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        set_array(lhs, (int)num, rhs);
                                    }
                                }
                            } else {
                                if (rhs.empty()) {
                                    if (is_dictionary(p, v))
                                        null_error();
                                    
                                    type_error(4, 3);
                                    //  string != int
                                }
                                
                                if (ss::is_array(rhs))
                                    type_error(0, 3);
                                    //  array != int
                                
                                if (is_string(rhs)) {
                                    if (!is_dictionary(p, v))
                                        type_error(0, 6);
                                        //  array != dictionary
                                    
                                    rhs = decode(rhs);
                                    
                                    if (rhs.empty())
                                        undefined_error(empty());
                                    
                                    rhs = encode(rhs);
                                    
                                    string key = rhs;
                                    
                                    rhs = operands.pop();
                                    
                                    rhs = element(rhs);
                                    
                                    size_t k = 0;
                                    while (k < (size_t)floor(p / 2) && v[k * 2] != key)
                                        ++k;
                                        //  search for key
                                    
                                    if (k == (size_t)floor(p / 2)) {
                                        string* tmp = new string[p + 2];
                                        for (size_t l = 0; l < p; ++l)
                                            tmp[l] = v[l];
                                        
                                        delete[] v;
                                        
                                        v = tmp;
                                        
                                        v[p++] = key;
                                        v[p++] = rhs;
                                    } else
                                        v[k * 2 + 1] = rhs;
                                } else if (is_symbol(rhs)) {
                                    //  array, string, or double
                                    
                                    if (io_array(rhs) != -1)
                                        type_error(0, 3);
                                        //  array != int
                                    
                                    int k = io_string(rhs);
                                    if (k == -1) {
                                        double idx = get_number(rhs);
                                        if (!is_int(idx))
                                            type_error(2, 3);
                                            //  double != int
                                        
                                        if (idx < 0 || idx > p)
                                            range_error("index " + rtrim(p) + ", count " + to_string(p));
                                        
                                        rhs = operands.top();
                                        
                                        rhs = element(rhs);
                                        
                                        if (idx == p) {
                                            string* tmp = new string[p + 1];
                                            for (size_t k = 0; k < p; ++k)
                                                tmp[k] = v[k];
                                            
                                            delete[] v;
                                            
                                            v = tmp;
                                            
                                            v[p++] = rhs;
                                        } else
                                            v[(size_t)idx] = rhs;
                                    } else {
                                        if (!is_dictionary(p, v))
                                            type_error(0, 6);
                                            //  array != dictionary
                                        
                                        rhs = std::get<1>(* stringv[k]);
                                        
                                        if (rhs.empty())
                                            null_error();
                                        
                                        if (rhs.length() == 2)
                                            undefined_error(empty());
                                        
                                        string key = rhs;
                                        
                                        rhs = operands.pop();
                                        
                                        rhs = element(rhs);
                                        
                                        size_t l = 0;
                                        while (l < (size_t)floor(p / 2) && v[l * 2] != key)
                                            ++l;
                                            //  search for key
                                        
                                        if (l == (size_t)floor(p / 2)) {
                                            string* tmp = new string[p + 2];
                                            
                                            for (size_t m = 0; m < p; ++m)
                                                tmp[m] = v[m];
                                            
                                            delete[] v;
                                            
                                            v = tmp;
                                            
                                            v[p++] = key;
                                            v[p++] = rhs;
                                        } else
                                            v[l * 2 + 1] = rhs;
                                    }
                                } else {
                                    //  double
                                    
                                    double idx = stod(rhs);
                                    if (!is_int(idx))
                                        type_error(2, 3);
                                        //  double != int
                                    
                                    if (idx < 0 || idx > p)
                                        range_error("index " + rtrim(p) + ", count " + to_string(p));
                                    
                                    rhs = operands.pop();
                                    
                                    rhs = element(rhs);
                                    
                                    if (idx == p) {
                                        string* tmp = new string[p + 1];
                                        for (size_t k = 0; k < p; ++k)
                                            tmp[k] = v[k];
                                        
                                        delete[] v;
                                        
                                        v = tmp;
                                        
                                        v[p++] = rhs;
                                    } else
                                        v[(size_t)idx] = rhs;
                                }
                                
                                rhs = stringify(p, v);
                            }
                        } else {
                            if (!is_symbol(lhs)) {
                                delete[] v;
                                operation_error();
                            }
                            
                            //  const
                            if (data[k] == uov[const_oi]->opcode())
                                ++k;
                            
                            if (data[k] == "array") {
                                if (is_defined(lhs))
                                    defined_error(lhs);
                                    
                                v = new string[rhs.length() + 1];
                                p = parse(v, rhs);
                                
                                if (p == 1) {
                                    delete[] v;
                                    
                                    if (rhs.empty())
                                        set_array(lhs, 0, empty());
                                    
                                    else if (is_string(rhs)) {
                                        rhs = decode(rhs);
                                        rhs = encode(rhs);
                                        set_array(lhs, 0, rhs);
                                        
                                    } else if (is_symbol(rhs)) {
                                        int l = io_array(rhs);
                                        if (l == -1) {
                                            l = io_string(rhs);
                                            if (l == -1) {
                                                rhs = rtrim(get_number(rhs));
                                                
                                                set_array(lhs, 0, rhs);
                                            } else {
                                                rhs = std::get<1>(* stringv[l]);
                                                
                                                set_array(lhs, 0, rhs);
                                                
                                                std::get<2>(* stringv[l]).second = true;
                                            }
                                        } else {
                                            set_array(lhs, 0, std::get<1>(* arrayv[l])[0]);
                                            
                                            l = io_array(rhs);
                                            
                                            std::get<2>(* arrayv[l]).second = true;
                                            
                                            for (size_t m = 1; m < std::get<1>(* arrayv[l]).size(); ++m)
                                                set_array(lhs, m, std::get<1>(* arrayv[l])[m]);
                                            
                                            std::get<1>(* arrayv[io_array(lhs)]).shrink_to_fit();
                                            
                                            rhs = stringify(std::get<1>(* arrayv[l]));
                                        }
                                    } else {
                                        rhs = rtrim(stod(rhs));
                                        
                                        set_array(lhs, 0, rhs);
                                    }
                                } else {
                                    for (size_t m = 0; m < p; ++m)
                                        set_array(lhs, m, v[m]);
                                    
                                    std::get<1>(* arrayv[io_array(lhs)]).shrink_to_fit();
                                    
                                    delete[] v;
                                }
                                
                                if (data[i + 1] == uov[const_oi]->opcode())
                                    std::get<2>(* arrayv[io_array(lhs)]).first = true;
                            } else {
                                int k = io_array(lhs);
                                if (k == -1) {
                                    k = io_string(lhs);
                                    if (k == -1) {
                                        if (is_defined(lhs)) {
                                            if (data[i + 1] == uov[const_oi]->opcode())
                                                defined_error(lhs);
                                            
                                            if (ss::is_array(rhs))
                                                type_error(0, 2);
                                                //  array != double
                                            
                                            if (rhs.empty() || is_string(rhs))
                                                type_error(4, 2);
                                                //  string != double
                                            
                                            double d;
                                            
                                            if (is_symbol(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(0, 2);
                                                    //  array != double
                                                    
                                                if (io_string(rhs) != -1)
                                                    type_error(4, 2);
                                                    //  string != double
                                                
                                                d = get_number(rhs);
                                            } else
                                                d = stod(rhs);
                                            
                                            set_number(lhs, d);
                                            
                                            rhs = rtrim(d);
                                        } else {
                                            if (rhs.empty()) {
                                                set_string(lhs, rhs);
                                                
                                                if (data[i + 1] == uov[const_oi]->opcode())
                                                    std::get<2>(* arrayv[io_array(lhs)]).first = true;
                                            } else {
                                                v = new string[rhs.length() + 1];
                                                p = parse(v, rhs);
                                                
                                                if (p == 1) {
                                                    delete[] v;
                                                    
                                                    if (is_string(rhs)) {
                                                        rhs = encode(decode(rhs));
                                                        
                                                        set_string(lhs, rhs);
                                                        
                                                        if (data[i + 1] == uov[const_oi]->opcode())
                                                            std::get<2>(* stringv[io_string(lhs)]).first = true;
                                                        
                                                    } else if (is_symbol(rhs))  {
                                                        k = io_array(rhs);
                                                        if (k == -1) {
                                                            k = io_string(rhs);
                                                            if (k == -1) {
                                                                double d = get_number(rhs);
                                                                
                                                                set_number(lhs, d);
                                                                
                                                                if (data[i + 1] == uov[const_oi]->opcode())
                                                                    set_read_only(lhs, true);
                                                                
                                                                rhs = rtrim(d);
                                                            } else {
                                                                rhs = std::get<1>(* stringv[k]);
                                                                std::get<2>(* stringv[k]).second = true;
                                                                
                                                                set_string(lhs, rhs);
                                                                
                                                                if (data[i + 1] == uov[const_oi]->opcode())
                                                                    std::get<2>(* stringv[io_string(lhs)]).first = true;
                                                            }
                                                        } else {
                                                            set_array(lhs, 0, std::get<1>(* arrayv[k])[0]);
                                                            
                                                            k = io_array(rhs);
                                                            
                                                            for (size_t l = 1; l < std::get<1>(* arrayv[k]).size(); ++l)
                                                                set_array(lhs, l, std::get<1>(* arrayv[k])[l]);
                                                            
                                                            if (data[i + 1] == uov[const_oi]->opcode()) {
                                                                k = io_array(lhs);
                                                                std::get<1>(* arrayv[k]).shrink_to_fit();
                                                                std::get<2>(* arrayv[k]).first = true;
                                                            }
                                                                
                                                        }
                                                    } else {
                                                        double d = stod(rhs);
                                                        
                                                        set_number(lhs, d);
                                                        
                                                        if (data[i + 1] == uov[const_oi]->opcode())
                                                            set_read_only(lhs, true);
                                                        
                                                        rhs = rtrim(d);
                                                    }
                                                } else {
                                                    set_array(lhs, stringify(p, v));
                                                    
                                                    delete[] v;
                                                
                                                    if (data[i + 1] == uov[const_oi]->opcode()) {
                                                        int k = io_array(lhs);
                                                        std::get<1>(* arrayv[k]).shrink_to_fit();
                                                        std::get<2>(* arrayv[k]).first = true;
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        if (data[i + 1] == uov[const_oi]->opcode())
                                            defined_error(lhs);
                                        
                                        if (ss::is_array(rhs))
                                            type_error(0, 4);
                                            //  array != string
                                        
                                        if (is_string(rhs))
                                            rhs = encode(decode(rhs));
                                            
                                        else if (is_symbol(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(0, 4);
                                                //  array != string
                                            
                                            int k = io_string(rhs);
                                            if (k == -1) {
                                                if (is_defined(rhs))
                                                    type_error(2, 4);
                                                    //  double != string
                                                
                                                undefined_error(rhs);
                                            }
                                            
                                            rhs = std::get<1>(* stringv[k]);
                                            std::get<2>(* stringv[k]).second = true;
                                        } else
                                            type_error(2, 4);
                                            //  double != string
                                        
                                        set_string(lhs, rhs);
                                    }
                                } else {
                                    if (data[i + 1] == uov[const_oi]->opcode())
                                        defined_error(lhs);
                                    
                                    if (std::get<2>(* arrayv[k]).first)
                                        write_error(lhs);
                                    
                                    std::get<1>(* arrayv[k]).clear();
                                    
                                    if (rhs.empty())
                                        std::get<1>(* arrayv[k]).push(rhs);
                                    else {
                                        v = new string[rhs.length() + 1];
                                        p = parse(v, rhs);
                                        
                                        if (p == 1) {
                                            delete[] v;
                                            
                                            if (is_string(rhs)) {
                                                rhs = decode(rhs);
                                                rhs = encode(rhs);
                                                
                                                std::get<1>(* arrayv[k]).push(rhs);
                                                
                                            } else if (is_symbol(rhs)) {
                                                int l = io_array(rhs);
                                                if (l == -1) {
                                                    l = io_string(rhs);
                                                    if (l == -1)
                                                        rhs = rtrim(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[l]);
                                                        std::get<2>(* stringv[l]).second = true;
                                                    }
                                                    
                                                    std::get<1>(* arrayv[k]).push(rhs);
                                                } else {
                                                    p = std::get<1>(* arrayv[l]).size();
                                                    for (size_t m = 0; m < p; ++m)
                                                        std::get<1>(* arrayv[k]).push(std::get<1>(* arrayv[l])[m]);
                                                    
                                                    std::get<2>(* arrayv[l]).second = true;
                                                    
                                                    rhs = stringify(std::get<1>(* arrayv[k]));
                                                }
                                            } else {
                                                rhs = rtrim(stod(rhs));
                                                
                                                std::get<1>(* arrayv[k]).push(rhs);
                                            }
                                        } else
                                            for (size_t m = 0; m < p; ++m)
                                                std::get<1>(* arrayv[k]).push(v[m]);
                                    }
                                }
                            }
                        }
                    } else if (j == conditional_oi) {
                        double flag;
                        
                        if (lhs.empty())
                            flag = 0;
                        else {
                            string v[lhs.length() + 1];
                            size_t p = parse(v, lhs);
                            
                            if (p == 1) {
                                if (is_string(lhs))
                                    flag = decode(lhs) != types[5];
                                else if (is_symbol(lhs)) {
                                    int k = io_array(lhs);
                                    if (k == -1) {
                                        k = io_string(lhs);
                                        if (k == -1)
                                            flag = get_number(lhs);
                                        else {
                                            flag = std::get<1>(* stringv[k]).empty()
                                                    ? 0 : decode(std::get<1>(* stringv[k])) == types[5]
                                                    ? 0 : 1;
                                            std::get<2>(* stringv[k]).second = true;
                                        }
                                    } else {
                                        std::get<2>(* arrayv[k]).second = true;
                                        
                                        if (std::get<1>(* arrayv[k]).size() == 1) {
                                            lhs = std::get<1>(* arrayv[k])[0];
                                            
                                            if (lhs.empty())
                                                flag = 0;
                                            else if (is_string(lhs))
                                                flag = decode(lhs) != types[5];
                                            else
                                                flag = !stod(lhs) ? 0 : 1;
                                        } else
                                            flag = 1;
                                    }
                                } else
                                    flag = stod(lhs);
                            } else
                                flag = 1;
                        }
                        
                        if (flag) {
                            rhs = evaluate(operands.pop());
                            
                            operands.pop();
                        } else {
                            operands.pop();     //  discard lhs
                            
                            rhs = evaluate(operands.pop());
                        }
                        
                        //  implement ternary operators
                        //  hybrid binary-ternary operators must still live in evaluate()
                        //  as must logical operators
                    } else {
                        rhs = operands.pop();
                        rhs =   ((buo *)uov[j])->apply(lhs, rhs);
                    }
                    
                    operands.push(rhs);
                }
            }
        }
        
        if (!operands.size())
            return empty();
        
        while (operands.size() > 1) {
            if (operands.top().empty() ||
                ss::is_array(operands.top()) ||
                is_string(operands.top())) {
                    operands.pop();
                    continue;
            }
            
            if (is_symbol(operands.top())) {
                int i = io_array(operands.top());
                
                if (i == -1) {
                    i = io_string(operands.top());
                    
                    if (i == -1)
                        get_number(operands.top());
                    else
                        std::get<2>(* stringv[i]).second = true;
                } else
                    std::get<2>(* arrayv[i]).second = true;
                
            }
            
            operands.pop();
        }
        
        //  cout << operands.top() << endl;
            
        if (ss::is_array(operands.top()) || operands.top().empty())
            return operands.pop();
        
        if (is_string(operands.top())) {
            operands.push(decode(operands.pop()));
            
            return encode(operands.top());
        }
        
        if (is_symbol(operands.top())) {
            int i = io_array(operands.top());
            
            if (i == -1) {
                i = io_string(operands.top());
                
                if (i == -1)
                    return rtrim(get_number(operands.pop()));
                
                std::get<2>(* stringv[i]).second = true;
                    
                return std::get<1>(* stringv[i]);
            }
            
            std::get<2>(* arrayv[i]).second = true;
            
            return stringify(std::get<1>(* arrayv[i]));
        }
        
        string result = rtrim(stod(operands.pop()));
    
        return result;
    }

    string interpreter::get_array(const string symbol, const size_t index) {
        int i = io_array(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        if (index >= std::get<1>(* arrayv[i]).size())
            range_error(to_string(index));
        
        std::get<2>(* arrayv[i]).second = true;
        
        return std::get<1>(* arrayv[i])[index];
    }

    string interpreter::get_string(const string symbol) {
        int i = io_string(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        std::get<2>(* stringv[i]).second = true;
        
        return std::get<1>(* stringv[i]);
    }

    int interpreter::io_string(const string symbol) const {
        if (string_bst == NULL)
            return -1;
        
        return index_of(string_bst, symbol);
    }

    int interpreter::io_array(const string symbol) const {
        if (array_bst == NULL)
            return -1;
        
        return index_of(array_bst, symbol);
    }

    void interpreter::initialize() {
        //  BEGIN OPERATORS
        
        uov = new operator_t*[59];

        uov[uoc++] = new uuo("count", [this](const string rhs) {
            if (rhs.empty())
                null_error();
            
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty())
                    null_error();
                
                if (is_string(rhs)) {
                    string str = decode(rhs);
                    return to_string(str.length());
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                operation_error();
                            
                            undefined_error(rhs);
                        }
                        
                        if (std::get<1>(* stringv[i]).empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return to_string(decode(std::get<1>(* stringv[i])).length());
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return to_string(std::get<1>(* arrayv[i]).size());
                }
                
                operation_error();
            }
            
            return to_string(valuec);
        });
        //  10
        
        uov[const_oi = uoc++] = new uuo("const", [this](string rhs) {
            unsupported_error("const");
            return nullptr;
        });
        //  2
        
        uov[uoc++] = new uuo("first", [this](const string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    operation_error();
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                operation_error();
                            
                            undefined_error(rhs);
                        }
                            
                        operation_error();
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return std::get<1>(* arrayv[i])[0];
                }
                
                operation_error();
            }
            
            return valuev[0];
        });
        
        uov[uoc++] = new uuo("inverse", [this](string rhs) {
           if (rhs.empty())
               type_error(4, 7);
                //  string != table
            
            string* data = new string[rhs.length() + 1];
            size_t n = parse(data, rhs);
            
            if (n == 1) {
                delete[] data;
                
                if (is_string(rhs))
                    type_error(4, 7);
                    //  string != table
                
                if (!is_symbol(rhs))
                    type_error(2, 7);
                    //  double != table
                
                int i = io_array(rhs);
                if (i == -1) {
                    if (io_string(rhs) != -1)
                        type_error(4, 7);
                        //  string != table
                    
                    if (is_defined(rhs))
                        type_error(2, 7);
                        //  double != table
                    
                    undefined_error(rhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(0, 7);
                    //  array != table
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                size_t r = (std::get<1>(*  arrayv[i]).size() - 1) / c;
                
                data = new string[std::get<1>(* arrayv[i]).size()];
                n = 0;
                
                data[n++] = to_string(r);
                
                for (size_t j = 0; j < c; ++j)
                    for (size_t k = 0; k < r; ++k)
                        data[n++] = std::get<1>(* arrayv[i])[j + (k * c) + 1];
                
                rhs = stringify(n, data);
                delete[] data;
                
                return rhs;
            }
            
            if (!is_table(n, data))
                type_error(0, 7);
                //  array != table
            
            size_t c = stoi(data[0]);
            size_t r = (n - 1) / c;
            
            string result[n];
            size_t size = 0;
            
            result[size++] = to_string(r);
            
            for (size_t i = 0; i < c; ++i)
                for (size_t j = 0; j < r; ++j)
                    result[size++] = data[i + (j * c) + 1];
                    
            return stringify(size, result);
        });
        
        uov[uoc++] = new uuo("isalpha", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(isalpha(rhs[0]) ? 1 : 0);
        });
        //  5
        
        uov[uoc++] = new uuo("isalnum", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(isalnum(rhs[0]) ? 1 : 0);
        });
        //  6
        
        uov[uoc++] = new uuo("isdigit", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(isdigit(rhs[0]) ? 1 : 0);
        });
        //  8
        
        uov[uoc++] = new uuo("islower", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(islower(rhs[0]) ? 1 : 0);
        });
        //  7
        
        uov[uoc++] = new uuo("isspace", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string values[rhs.length() + 1];
            if (parse(values, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(isspace(rhs[0]) ? 1 : 0);
        });
        
        uov[uoc++] = new uuo("isupper", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 1);
                //  array != char
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string(isupper(rhs[0]) ? 1 : 0);
        });
        //  9
        
        uov[uoc++] = new uuo("keys", [this](string rhs) {
            if (rhs.empty())
                type_error(4, 6);
                //  string != dictionary
            
            string v[rhs.length() + 1];
            size_t n = parse(v, rhs);
            
            if (n == 1) {
                if (is_string(rhs))
                    type_error(4, 6);
                    //  string != dictionary
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        if (io_string(rhs) != -1)
                            type_error(4, 6);
                            //  string != dictionary
                        
                        if (is_defined(rhs))
                            type_error(2, 6);
                            //  double != dictionary
                        
                        undefined_error(rhs);
                    }
                    
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(0, 6);
                        //  array != dictionary
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    stringstream ss;
                    size_t j;
                    for (j = 0; j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) - 1; ++j)
                        ss << std::get<1>(* arrayv[i])[j * 2] << pattern();
                    
                    ss << std::get<1>(* arrayv[i])[j * 2];
                    
                    return ss.str();
                }
            }
            
            if (!is_dictionary(n, v))
                type_error(0, 6);
                //  array != dictionary
            
            stringstream ss;
            size_t i;
            for (i = 0; i < (size_t)floor(n / 2) - 1; ++i)
                ss << v[i * 2] << pattern();
            
            ss << v[i * 2];
            
            return ss.str();
        });
        
        uov[uoc++] = new uuo("last", [this](const string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    operation_error();
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                operation_error();
                            
                            undefined_error(rhs);
                        }
                        
                        operation_error();
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return std::get<1>(* arrayv[i])[std::get<1>(* arrayv[i]).size() - 1];
                }
                
                operation_error();
            }
            
            return valuev[valuec - 1];
        });
        
        uov[uoc++] = new uuo("parse", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 4);
                //  array != string
            
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 4);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(0, 4);
                    //  array != string
               
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 4);
                        //  double != string
                   
                    undefined_error(rhs);
                }
               
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = decode(rhs);
            
            if (!is_number(rhs))
                return string("nan");
            
            try {
                return rtrim(stod(rhs));
                
            } catch (invalid_argument& ia) {
                return string("nan");
            }
        });
        
        uov[shrink_oi = uoc++] = new uuo("shrink", [this](string rhs) {
            if (rhs.empty())
                operation_error();
            
            if (!is_symbol(rhs))
                operation_error();
            
            int i = io_array(rhs);
            if (i == -1) {
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        operation_error();
                    
                    undefined_error(rhs);
                }
                
                operation_error();
            }
            
            if (std::get<2>(* arrayv[i]).first)
                write_error(rhs);
            
            rhs = to_string(std::get<1>(* arrayv[i]).capacity());
            
            std::get<1>(* arrayv[i]).shrink_to_fit();
               
            return rhs;
        });
        
        uov[uoc++] = new uuo("sizeof", [this](const string rhs) {
            if (rhs.empty())
                null_error();
            
            if (!is_symbol(rhs))
                operation_error();
            
            int i = io_array(rhs);
            if (i == -1) {
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        operation_error();
                    
                    undefined_error(rhs);
                }
                
                operation_error();
            }
            
            std::get<2>(* arrayv[i]).second = true;
            
            return to_string(std::get<1>(* arrayv[i]).capacity());
        });
        
        uov[uoc++] = new uuo("subtypeof", [this](string rhs) {
            if (rhs.empty())
               return encode(types[4]);
                //  string
            
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec != 1)
                return encode(is_table(valuec, valuev) ?
                              types[7] :
                              is_dictionary(valuec, valuev) ?
                              types[6] :
                              types[0]);
                //  array
            
            if (is_string(rhs)) {
                rhs = decode(rhs);
                
                return encode(rhs.length() == 1 ? types[1] : types[4]);
                //  char | string
            }
            
            if (is_symbol(rhs)) {
                int i = io_array(rhs);
                if (i == -1) {
                    i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs)) {
                            if (is_int(get_number(rhs)))
                                return encode(types[3]);
                                //  int
                                
                            return encode(types[2]);
                            //  double
                        }
                        
                        return encode(types[5]);
                        //  undefined
                    }
                    
                    rhs = std::get<1>(* stringv[i]);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (rhs.empty())
                        return encode(types[4]);
                        //  string
                    
                    return encode(rhs.length() == 3 ? types[1] : types[4]);
                    //  char : string
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                return encode(is_table(std::get<1>(* arrayv[i])) ?
                              types[7] :
                              is_dictionary(std::get<1>(* arrayv[i])) ?
                              types[6] :
                              types[0]);
                //  array
            }
            
            double d = stod(rhs);
            
            if (!is_int(d))
                return encode(types[2]);
                //  double
            
            return encode(types[3]);
            //  int
        });
        //  16
        
        uov[uoc++] = new uuo("toarray", [this](string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty())
                    null_error();
                
                if (is_string(rhs)) {
                    rhs = decode(rhs);
                    
                    stringstream ss;
                    
                    if (rhs.length()) {
                        for (size_t i = 0; i < rhs.length() - 1; ++i) {
                            char str[2];
                            str[0] = rhs[i];
                            str[1] = '\0';
                            
                            ss << encode(string(str)) << pattern();
                        }
                        
                        char str[2];
                        str[0] = rhs[rhs.length() - 1];
                        str[1] = '\0';
                        
                        ss << encode(string(str));
                    }
                    
                    return ss.str();
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        int i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                type_error(2, 4);
                                //  double != string
                            
                            undefined_error(rhs);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        rhs = decode(rhs);
                        
                        stringstream ss;
                        
                        if (rhs.length()) {
                            for (size_t j = 0; j < rhs.length() - 1; ++j) {
                                char str[2];
                                str[0] = rhs[j];
                                str[1] = '\0';
                                
                                ss << encode(string(str)) << pattern();
                            }
                            
                            char str[2];
                            str[0] = rhs[rhs.length() - 1];
                            str[1] = '\0'; 
                            
                            ss << encode(string(str));
                        }
                        
                        return ss.str();
                    }
                    
                    return stringify(std::get<1>(* arrayv[i]));
                }
                
                type_error(2, 4);
                //  double != string
            }
            
            return rhs;
        });
        
        uov[uoc++] = new uuo("toascii", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                operation_error();
            
            int i = -1;
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 1);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(0, 1);
                    //  array != char
                    
                i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 1);
                        //  double != char
                    
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            if (i != -1)
                std::get<2>(* stringv[i]).second = true;
            
            return to_string((size_t)rhs[0]);
        });
        
        uov[uoc++] = new uuo("tochar", [this](string rhs) {
            if (rhs.empty())
                type_error(4, 3);
                //  string != int
            
            string v[rhs.length() + 1];
            if (parse(v, rhs) != 1)
                type_error(0, 3);
                //  array != int
            
            double d;
            
            if (is_string(rhs))
                type_error(4, 3);
                //  string != int
            
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(0, 3);
                    //  array != int
                
                if (io_string(rhs) != -1)
                    type_error(4, 3);
                    //  string != int
                
                d = get_number(rhs);
            } else
                d = stod(rhs);
            
            if (!is_int(d))
                type_error(2, 3);
                //  double != int
            
            if (d < 0 || d >= 128)
                range_error(to_string((size_t)d));
            
            char str[2];
            str[0] = (char)((size_t)d);
            str[1] = '\0';
            
            rhs = string(str);
            
            return encode(rhs);
        });
        //  1
        
        uov[uoc++] = new uuo("tolower", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            if (ss::is_array(rhs))
                type_error(0, 4);
                //  array != string
            
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 4);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(0, 4);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 4);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
            
            rhs = tolower(rhs);
            
            return encode(rhs);
        });
        //  14
        
        uov[uoc++] = new uuo("toupper", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            if (ss::is_array(rhs))
                type_error(0, 4);
                //  array != string
            
            if (!is_string(rhs)) {
                if (!is_symbol(rhs))
                    type_error(2, 4);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(0, 4);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(2, 4);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(4, 1);
                //  string != char
                
            rhs = toupper(rhs);
            
            return encode(rhs);
        });
        //  15
        
        uov[uoc++] = new uuo("typeof", [this](const string rhs) {
            if (ss::is_array(rhs))
                return encode(types[0]);
            
            if (rhs.empty() || is_string(rhs))
                return encode(types[4]);
            
            if (is_symbol(rhs)) {
                int i = io_array(rhs);
                if (i == -1) {
                    i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs)) {
                            get_number(rhs);
                            
                            return encode("number");
                        }
                        
                        return encode(types[5]);
                    }
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    return encode(types[4]);
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                return encode(types[0]);
            }
            
            stod(rhs);
            
            return encode("number");
        });
        
        uov[uoc++] = new uuo("values", [this](string rhs) {
            if (rhs.empty())
                type_error(4, 6);
                //  string != dictionary
            
            string v[rhs.length() + 1];
            size_t n = parse(v, rhs);
            
            if (n == 1) {
                if (is_string(rhs))
                    type_error(4, 6);
                    //  string != dictionary
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        if (io_string(rhs) != -1)
                            type_error(4, 6);
                            //  string != dictionary
                        
                        if (is_defined(rhs))
                            type_error(2, 6);
                            //  double != dictionary
                        
                        undefined_error(rhs);
                    }
                    
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(0, 6);
                        //  array != dictionary
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    stringstream ss;
                    size_t j;
                    for (j = 0; j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) - 1; ++j)
                        ss << std::get<1>(* arrayv[i])[j * 2 + 1] << pattern();
                    
                    ss << std::get<1>(* arrayv[i])[j * 2 + 1];
                    
                    return ss.str();
                }
            }
            
            if (!is_dictionary(n, v))
                type_error(0, 6);
                //  array != dictionary
            
            stringstream ss;
            size_t i;
            for (i = 0; i < (size_t)floor(n / 2) - 1; ++i)
                ss << v[i * 2 + 1] << pattern();
            
            ss << v[i * 2 + 1];
            
            return ss.str();
        });
        
        uov[unary_count = uoc++] = new buo(",", [this](const string lhs, const string rhs) {
            unsupported_error(",");
            return nullptr;
        });
        //  17
        
        uov[indexer_oi = uoc++] = new buo(".", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (is_string(lhs)) {
                    lhs = decode(lhs);
                    
                    if (rhs.empty())
                        type_error(4, 3);
                        //  string != int
                    
                    if (ss::is_array(rhs))
                        type_error(0, 3);
                        //  array != int
                    
                    if (is_string(rhs))
                        type_error(4, 3);
                        //  string != int
                    
                    double num;
                    
                    if (is_symbol(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(0, 3);
                            //  array != int
                        
                        if (io_string(rhs) != -1)
                            type_error(4, 3);
                            //  string != int
                        
                        num = get_number(rhs);
                    } else
                        num = stod(rhs);
                    
                    if (!is_int(num))
                        type_error(2, 3);
                        //  double != int
                    
                    if (num < 0 || num >= lhs.length())
                        range_error("index " + rtrim(num) + ", count " + to_string(lhs.length()));
                    
                    char str[2];
                    
                    str[0] = lhs[(size_t)num];
                    str[1] = '\0';
                    rhs = string(str);
                    
                    return encode(rhs);
                }
                
                if (!is_symbol(lhs))
                    type_error(2, 4);
                    //  double != string
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(2, 4);
                            //  double != string
                        
                        undefined_error(lhs);
                    }
                    
                    lhs = std::get<1>(* stringv[i]);
                    
                    if (lhs.empty())
                        null_error();
                    
                    lhs = decode(lhs);
                    
                    if (rhs.empty())
                        type_error(4, 3);
                        //  string != int
                    
                    if (ss::is_array(rhs))
                        type_error(0, 3);
                        //  array != int
                    
                    if (is_string(rhs))
                        type_error(4, 3);
                        //  string != int
                    
                    double num;
                    
                    if (is_symbol(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(0, 3);
                            //  array != int
                        
                        if (io_string(rhs) != -1)
                            type_error(4, 3);
                            //  string != int
                        
                        num = get_number(rhs);
                    } else
                        num = stod(rhs);
                    
                    if (!is_int(num))
                        type_error(2, 3);
                        //  double != int
                    
                    if (num < 0 || num >= lhs.length())
                        range_error("index " + rtrim(num) + ", count " + to_string(lhs.length()));
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    char str[2];
                    
                    str[0] = lhs[(size_t)num];
                    str[1] = '\0';
                    
                    rhs = string(str);
                    
                    return encode(rhs);
                }
                
                if (rhs.empty()) {
                    if (is_dictionary(std::get<1>(* arrayv[i])))
                        null_error();
                    
                    type_error(4, 3);
                    //  string != int
                }
                
                if (ss::is_array(rhs))
                    type_error(0, 3);
                    //  array != int
                
                if (is_string(rhs))  {
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(0, 6);
                        //  array != dictionary
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = encode(decode(rhs));
                    
                    size_t j = 0;
                    while (j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) && std::get<1>(* arrayv[i])[j * 2] != rhs)
                        ++j;
                    
                    if (j == (size_t)floor(std::get<1>(* arrayv[i]).size() / 2))
                        return encode(types[5]);
                    
                    return std::get<1>(* arrayv[i])[j * 2 + 1];
                }
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 3);
                        //  array != int
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double num = get_number(rhs);
                        
                        if (!is_int(num))
                            type_error(2, 3);
                            //  double != int
                        
                        if (num < 0 || num >= std::get<1>(* arrayv[i]).size())
                            range_error("index " + rtrim(num) + ", count " + to_string(std::get<1>(* arrayv[i]).size()));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        return std::get<1>(* arrayv[i])[(size_t)num];
                    }
                    
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(0, 6);
                        //  array != dictionary
                    
                    rhs = std::get<1>(* stringv[j]);
                    
                    if (rhs.empty())
                        null_error();
                    
                    std::get<2>(* arrayv[i]).second = true;
                    std::get<2>(* stringv[j]).second = true;
                    
                    size_t k = 0;
                    while (k < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) && std::get<1>(* arrayv[i])[k * 2] != rhs)
                        ++k;
                    
                    if (k == (size_t)floor(std::get<1>(* arrayv[i]).size() / 2))
                        return encode(types[5]);
                    
                    return std::get<1>(* arrayv[i])[k * 2 + 1];
                }
                
                double num = stod(rhs);
                
                if (!is_int(num))
                    type_error(2, 3);
                    //  double != int
                
                if (num < 0 || num >= std::get<1>(* arrayv[i]).size())
                    range_error("index " + rtrim(num) + ", count " + to_string(std::get<1>(* arrayv[i]).size()));
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::get<1>(* arrayv[i])[(size_t)num];
            }
            
            if (rhs.empty()) {
                if (is_dictionary(valuec, valuev))
                    null_error();
                
                type_error(4, 3);
                //  string != int
            }
            
            if (ss::is_array(rhs))
                type_error(0, 3);
                //  array != int
            
            if (is_string(rhs)) {
                if (!is_dictionary(valuec, valuev))
                    type_error(0, 6);
                    //  array != dictionary
                
                rhs = encode(decode(rhs));
                
                size_t i = 0;
                while (i < (size_t)floor(valuec / 2) && valuev[i * 2] != rhs)
                    ++i;
                
                if (i == (size_t)floor(valuec / 2))
                    return encode(types[5]);
                
                return valuev[i * 2 + 1];
            }
            
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(0, 3);
                    //  array != int
                
                int i = io_string(rhs);
                if (i == -1) {
                    double num = get_number(rhs);
                    
                    if (!is_int(num))
                        type_error(2, 3);
                        //  double != int
                    
                    if (num < 0 || num >= valuec)
                        range_error("index " + rtrim(num) + ", count " + to_string(valuec));
                    
                    return valuev[(size_t)num];
                }
                
                if (!is_dictionary(valuec, valuev))
                    type_error(0, 6);
                    //  array != dictionary
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < (size_t)floor(valuec / 2) && valuev[j * 2] != rhs)
                    ++j;
                
                if (j == (size_t)floor(valuec / 2))
                    return encode(types[5]);
                
                return valuev[j * 2 + 1];
            }
            
            double num = stod(rhs);
                
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0 || num >= valuec)
                range_error("index " + rtrim(num) + ", count " + to_string(valuec));
            
            return valuev[(size_t)num];
        });
        //  18
        
        uov[aggregate_oi = uoc++] = new tuo("aggregate", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                    
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(4, 0);
                        //  string != array
                    
                    if (is_defined(lhs))
                        type_error(2, 0);
                        //  double != array
                    
                    undefined_error(lhs);
                }
                
                if (!is_symbol(ctr))
                    operation_error();
                
                if (is_defined(ctr))
                    defined_error(ctr);
                
                if (rhs.empty())
                    operation_error();
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < valuec; ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                set_array(ctr, 0, valuev[0]);
                
                for (size_t j = 1; j < valuec; ++j) {
                    set_array(ctr, 1, valuev[j]);
                    
                    string result = evaluate(rhs);
                    
                    if (ss::is_array(result))
                        type_error(0, 4);
                        //  array != string
                    
                    set_array(ctr, 0, result);
                }
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                delete[] valuev;
                
                string result;
                
                result = std::get<1>(* arrayv[io_array(ctr)])[0];
                
                remove_symbol(ctr);
                
                return result;
            }
            
            if (!is_symbol(ctr))
                operation_error();
            
            if (is_defined(rhs))
                defined_error(rhs);
            
            if (rhs.empty())
                operation_error();
            
            set_array(ctr, 0, valuev[0]);
            
            for (size_t i = 1; i < valuec; ++i) {
                set_array(ctr, 1, valuev[i]);
                
                string result = evaluate(rhs);
                
                if (ss::is_array(result))
                    type_error(0, 4);
                    //  array != string
                
                set_array(ctr, 0, result);
            }
            
            string result;
            
            result = std::get<1>(* arrayv[io_array(ctr)])[0];
            
            remove_symbol(ctr);
            
            return result;
        });
        
        uov[cell_oi = uoc++] = new tuo("cell", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1 || is_defined(lhs))
                        operation_error();
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(0, 7);
                    //  array != table
                
                if (ss::is_array(ctr))
                    type_error(0, 3);
                    //  array != int
                
                if (ctr.empty()|| is_string(ctr))
                    type_error(4, 3);
                    //  string != int
                
                double y = stod(ctr);
                
                if (!is_int(y))
                    type_error(2, 3);
                    //  double != int
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (y < 0 || y >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                    range_error("index " + rtrim(y) + ", rows " + to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                
                if (ss::is_array(rhs))
                    type_error(0, 3);
                    //  array != int
                
                if (rhs.empty() || is_string(rhs))
                    type_error(4, 3);
                    //  string != int
                
                double x = stod(rhs);
                
                if (!is_int(x))
                    type_error(2, 3);
                    //  double != int
                
                if (x < 0 || x >= c)
                    range_error("index " + rtrim(x) + ", cols " + to_string(c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::get<1>(* arrayv[i])[(size_t)(y * c + x + 1)];
            }
            
            if (!is_table(valuec, valuev)) {
                delete[] valuev;
                type_error(0, 7);
                //  array != table
            }
            
            if (ss::is_array(ctr))
                type_error(0, 3);
                //  array != int
            
            if (rhs.empty() || is_string(rhs))
                type_error(4, 3);
                //  string != int
            
            double y = stod(ctr);
            if (!is_int(y)) {
                delete[] valuev;
                type_error(2, 3);
            }
            
            size_t c = stoi(valuev[0]);
            
            if (y < 0 || y >= (valuec - 1) / c)
                range_error("index " + rtrim(y) + ", rows " + to_string((valuec - 1) / c));
            
            if (ss::is_array(rhs))
                type_error(0, 3);
                //  array != int
            
            if (rhs.empty() || is_string(rhs))
                type_error(4, 3);
                //  string != int
            
            double x = stod(rhs);
            if (!is_int(x)) {
                delete[] valuev;
                type_error(2, 3);
            }
            
            if (x < 0 || x >= c)
                range_error("index " + rtrim(x) + ", cols " + to_string(c));
            
            string result = valuev[(size_t)(y * c + x + 1)];
            
            delete[] valuev;
            
            return result;
        });
        
        uov[col_oi = uoc++] = new buo("col", [this](string lhs, string rhs) {
            if (lhs.empty())
                type_error(4, 7);
                //  string != table
            
            if (rhs.empty())
                type_error(4, 3);
                //  string != int
            
            string* v = new string[lhs.length() + 1];
            size_t n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    type_error(4, 7);
                    //  string != table
                
                if  (!is_symbol(lhs))
                    type_error(2, 7);
                    //  double != table
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(4, 7);
                        //  string != table
                    
                    if (is_defined(lhs))
                        type_error(2, 7);
                        //  double != table
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(0, 7);
                    //  array != table
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1)
                    type_error(0, 3);
                    //  array != int
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (is_string(rhs)) {
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = decode(rhs);
                    rhs = encode(rhs);
                    
                    size_t j = 0;
                    while (j < c && std::get<1>(* arrayv[i])[j + 1] != rhs)
                        ++j;
                    
                    if (j == c)
                        return encode(types[5]);
                        //  undefined
                    
                    stringstream ss;
                    
                    if ((std::get<1>(* arrayv[i]).size() - 1) / c != 1) {
                        size_t k;
                        for (k = 1; k < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[k * c + j + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[k * c + j + 1];
                    }
                    
                    return ss.str();
                }
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 3);
                        //  array != int
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double idx = get_number(rhs);
                        if (!is_int(idx))
                            type_error(2, 3);
                            //  double != int
                        
                        if (idx < 0 || idx >= c)
                            range_error("index " + rtrim(idx) + ", cols " + to_string(c));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        stringstream ss;
                        
                        size_t k;
                        for (k = 0; k < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[k * c + idx + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[k * c + idx + 1];
                        
                        return ss.str();
                    }
                    
                    rhs = std::get<1>(* stringv[j]);
                    
                    if (rhs.empty())
                        null_error();
                    
                    std::get<2>(* arrayv[i]).second = true;
                    std::get<2>(* stringv[j]).second = true;
                    
                    size_t k = 0;
                    while (k < c && std::get<1>(* arrayv[i])[k + 1] != rhs)
                        ++k;
                    
                    if (k == c)
                        return encode(types[5]);
                        //  undefined
                    
                    stringstream ss;
                    
                    if ((std::get<1>(* arrayv[i]).size() - 1) / c != 1) {
                        size_t l;
                        for (l = 1; l < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++l)
                            ss << std::get<1>(* arrayv[i])[l * c + k + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[l * c + k + 1];
                    }
                    
                    return ss.str();
                }
                
                double idx = stod(rhs);
                if (!is_int(idx))
                    type_error(2, 3);
                    //  double != int
                
                if (idx < 0 || idx >= c)
                    range_error("index " + rtrim(idx) + ", cols " + to_string(c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                
                size_t j;
                for (j = 0; j < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[j * c + idx + 1] << pattern();
                ss << std::get<1>(* arrayv[i])[j * c + idx + 1];
                
                return ss.str();
            }
            
            if (!is_table(n, v)) {
                delete[] v;
                type_error(0, 7);
                //  array != table
            }
            
            string _v[rhs.length() + 1];
            if (parse(_v, rhs) != 1) {
                delete[] v;
                type_error(0, 3);
                //  array != int
            }
            
            size_t c = stoi(v[0]);
            
            if (is_string(rhs)) {
                rhs = decode(rhs);
                rhs = encode(rhs);
                
                size_t i = 0;
                while (i < c && v[i + 1] != rhs)
                    ++i;
                
                if (i == c) {
                    delete[] v;
                    return encode(types[5]);
                    //  undefined
                }
                
                stringstream ss;
                
                if ((n - 1) / c != 1) {
                    size_t j;
                    for (j = 1; j < (n - 1) / c - 1; ++j)
                        ss << v[j * c + i + 1] << pattern();
                    ss << v[j * c + i + 1];
                }
                
                return ss.str();
            }
            
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] v;
                    type_error(0, 3);
                    //  array != int
                }
                
                int i = io_string(rhs);
                if (i == -1) {
                    double idx = get_number(rhs);
                    if (!is_int(idx)) {
                        delete[] v;
                        type_error(2, 3);
                        //  double != int
                    }
                    
                    if (idx < 0 || idx >= c) {
                        delete[] v;
                        range_error("index " + rtrim(idx) + ", cols " + to_string(c));
                    }
                    
                    stringstream ss;
                    
                    size_t j;
                    for (j = 0; j < (n - 1) / c - 1; ++j)
                        ss << v[j * c + (size_t)idx + 1] << pattern();
                    ss << v[j * c + (size_t)idx + 1];
                    
                    return ss.str();
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty()) {
                    delete[] v;
                    null_error();
                }
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < c && v[j + 1] != rhs)
                    ++j;
                
                if (j == c) {
                    delete[] v;
                    return encode(types[5]);
                    //  undefined
                }
                
                stringstream ss;
                
                if ((n - 1) / c != 1) {
                    size_t k;
                    for (k = 1; k < (n - 1) / c - 1; ++k)
                        ss << v[k * c + j + 1] << pattern();
                    ss << v[k * c + j + 1];
                }
                
                return ss.str();
            }
            
            double idx = stod(rhs);
            if (!is_int(idx)) {
                delete[] v;
                type_error(2, 3);
                //  double != int
            }
            
            if (idx < 0 || idx >= c) {
                delete[] v;
                range_error("index " + rtrim(idx) + ", cols " + to_string(c));
            }
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < (n - 1) / c - 1; ++i)
                ss << v[i * c + (size_t)idx + 1] << pattern();
            ss << v[i * c + (size_t)idx + 1];
            
            delete[] v;
            
            return ss.str();
        });
        
        uov[contains_oi = uoc++] = new buo("contains", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    string text = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(0, 4);
                        //  array != string
                    
                    if (rhs.empty())
                        null_error();
                        
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        size_t i = 0;
                        while (i < text.length() && text[i] != pattern[0])
                            ++i;
                        
                        return to_string(i != text.length());
                    }
                    
                    if (!is_symbol(rhs))
                        type_error(2, 4);
                        //  double != string
                    
                    if (io_array(rhs) != -1)
                        type_error(0, 4);
                        //  array != string
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(2, 4);
                            //  double != string
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[i]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode(pattern);
                    
                    if (pattern.length() != 1)
                        type_error(4, 1);
                        //  string != char
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    size_t j = 0;
                    while (j < text.length() && text[j] != pattern[0])
                        ++j;
                    
                    return to_string(j != text.length());
                }
                
                if (!is_symbol(lhs))
                    type_error(2, 4);
                    //  double != string
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (is_defined(lhs))
                                type_error(2, 4);
                                //  double != string
                            
                            undefined_error(lhs);
                        }
                        
                        string text = std::get<1>(* stringv[i]);
                        
                        if (text.empty())
                            null_error();
                        
                        text = decode(text);
                        
                        if (ss::is_array(rhs))
                            type_error(0, 4);
                            //  array != string
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            string pattern = decode(rhs);
                            
                            if (pattern.length() != 1)
                                type_error(4, 1);
                                //  string != char
                            
                            size_t j = 0;
                            while (j < text.length() && text[j] != pattern[0])
                                ++j;
                            
                            return to_string(j != text.length());
                        }
                        
                        if (!is_symbol(rhs))
                            type_error(2, 4);
                            //  double != string
                        
                        if (io_array(rhs) != -1)
                            type_error(0, 4);
                            //  array != string
                        
                        int j = io_string(rhs);
                        if (j == -1) {
                            if (is_defined(rhs))
                                type_error(2, 4);
                                //  double != string
                            
                            undefined_error(rhs);
                        }
                        
                        string pattern = std::get<1>(* stringv[j]);
                        
                        if (pattern.empty())
                            null_error();
                        
                        pattern = decode(pattern);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        std::get<2>(* stringv[i]).second = true;
                        std::get<2>(* stringv[j]).second = true;
                        
                        size_t k = 0;
                        while (k < text.length() && text[k] != pattern[0])
                            ++k;
                        
                        return to_string(k != text.length());
                    }
                }
                
                string pattern = element(rhs);
                
                std::get<2>(* arrayv[i]).second = true;
                
                return to_string(std::get<1>(* arrayv[i]).index_of(pattern));
            }
            string pattern = element(rhs);
            
            size_t i = 0;
            while (i < valuec && valuev[i] != pattern)
                ++i;
            
            return to_string(i != valuec);
        });
        
        uov[fill_oi = uoc++] = new buo("fill", [this](string lhs, string rhs) {
            if (lhs.empty())
                type_error(4, 0);
                //  string != array
            
            string* v = new string[lhs.length() + 1];
            size_t n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    type_error(4, 0);
                    //  string != array
                
                if (!is_symbol(lhs))
                    type_error(2, 0);
                    //  double != array
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(2, 0);
                            //  double != array
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(4, 0);
                    //  string != array
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                n = std::get<1>(* arrayv[i]).size();
                v = new string[n];
            }
            
            if (!rhs.empty()) {
                string _v[rhs.length() + 1];
                size_t _n = parse(_v, rhs);
                            
                if (_n != 1)
                    type_error(0, 4);
                    //  array != string
                
                if (is_string(rhs)) {
                    rhs = decode(rhs);
                    rhs = encode(rhs);
                    
                } else if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 4);
                        //  array != string
                    
                    int j = io_string(rhs);
                    if (j == -1)
                        rhs = rtrim(get_number(rhs));
                    else {
                        rhs = std::get<1>(* stringv[j]);
                        std::get<2>(* stringv[j]).second = true;
                    }
                } else
                    rhs = rtrim(stod(rhs));
            }
            
            for (size_t j = 0; j < n; ++j)
                v[j] = rhs;
            
            rhs = stringify(n, v);
            
            delete[] v;
            
            return rhs;
        });
        
        uov[filter_oi = uoc++] = new tuo("filter", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            operation_error();
                        
                        undefined_error(lhs);
                    }
                    
                    operation_error();
                }
                
                if (!is_symbol(ctr))
                    operation_error();
                
                if (is_defined(ctr))
                    defined_error(ctr);
                
                if (rhs.empty())
                    operation_error();
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < valuec; ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                size_t j = 0;
                while (j < valuec) {
                    if (valuev[j].empty() || is_string(valuev[j]))
                        set_string(ctr, valuev[j]);
                    else
                        set_number(ctr, stod(valuev[j]));
                    
                    string result = evaluate(rhs);
                    
                    if (ss::is_array(result))
                        type_error(0, 2);
                        //  array != double
                    
                    double flag;
                    if (result.empty())
                        flag = 0;
                    else if (is_string(result))
                        flag = !decode(result).empty() && decode(result) != types[5];
                    else
                        flag = !stod(result) ? 0 : 1;
                    
                    if (flag)
                        ++j;
                    else {
                        for (size_t k = j; k < valuec - 1; ++k)
                            swap(valuev[k], valuev[k + 1]);
                        --valuec;
                    }
                    
                    remove_symbol(ctr);
                }
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                string result = stringify(valuec, valuev);
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_symbol(ctr))
                operation_error();
            
            if (rhs.empty())
                operation_error();
            
            size_t i = 0;
            while (i < valuec) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                if (ss::is_array(result))
                    type_error(0, 2);
                    //  array != double
                
                double flag;
                
                flag = result.empty() ? 0 : is_string(result) ? 1 : stod(result) ? 1 : 0;
                
                if (flag)
                    ++i;
                else {
                    for (size_t j = i; j < valuec - 1; ++j)
                        swap(valuev[j], valuev[j + 1]);
                    --valuec;
                }
                
                remove_symbol(ctr);
            }
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        uov[find_oi = uoc++] = new tuo("find", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(4, 0);
                        //  string != array
                    
                    if (is_defined(lhs))
                        type_error(2, 0);
                        //  double != array
                    
                    undefined_error(lhs);
                }
                
                if (!is_symbol(ctr))
                    operation_error();
                
                if (is_defined(ctr))
                    defined_error(ctr);
                
                if (rhs.empty())
                    operation_error();
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < std::get<1>(* arrayv[i]).size(); ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                string result;
                
                size_t j;
                for (j = 0; j < valuec; ++j) {
                    if (valuev[j].empty() || is_string(valuev[j]))
                        set_string(ctr, valuev[j]);
                    else
                        set_number(ctr, stod(valuev[j]));
                    
                    string result = evaluate(rhs);
                    
                    remove_symbol(ctr);
                    
                    if (ss::is_array(result))
                        type_error(0, 2);
                        //  array != double
                    
                    if (!result.empty()) {
                        if (is_string(result)) {
                            result = decode(result);
                            
                            if (!result.empty() && result != types[5])
                                break;
                        } else if (stod(result))
                            break;
                    }
                }
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                if (j == valuec) {
                    delete[] valuev;
                    return encode(types[5]);
                }
                
                result = valuev[j];
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_symbol(ctr)) {
                delete[] valuev;
                operation_error();
            }
            
            if (is_defined(ctr)) {
                delete[] valuev;
                defined_error(ctr);
            }
            
            if (rhs.empty())
                operation_error();
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                remove_symbol(ctr);
                
                if (ss::is_array(result))
                    type_error(0, 2);
                    //  array != double
                
                if (!result.empty()) {
                    if (is_string(result)) {
                        result = decode(result);
                        
                        if (!result.empty() && result != types[5])
                            break;
                    } else if (stod(result))
                        break;
                }
            }
            
            if (i == valuec) {
                delete[] valuev;
                return encode(types[5]);
            }
            
            string result = valuev[i];
            
            delete[] valuev;
            
            return result;
        });
        
        uov[find_index_oi = uoc++] = new tuo("findindex", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(4, 0);
                        //  string != array
                    
                    if (is_defined(lhs))
                        type_error(2, 0);
                        //  double != array
                    
                    undefined_error(lhs);
                }
                
                if (!is_symbol(ctr))
                    operation_error();
                
                if (is_defined(ctr))
                    defined_error(ctr);
                
                if (rhs.empty())
                    operation_error();
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < std::get<1>(* arrayv[i]).size(); ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                string result;
                
                size_t j;
                for (j = 0; j < valuec; ++j) {
                    if (valuev[j].empty() || is_string(valuev[j]))
                        set_string(ctr, valuev[j]);
                    else
                        set_number(ctr, stod(valuev[j]));
                    
                    string result = evaluate(rhs);
                    
                    remove_symbol(ctr);
                    
                    if (ss::is_array(result))
                        type_error(0, 2);
                        //  array != double
                    
                    if (!result.empty()) {
                        if (is_string(result)) {
                            result = decode(result);
                            
                            if (!result.empty() && result != types[5])
                                break;
                        } else if (stod(result))
                            break;
                    }
                }
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                if (j == valuec) {
                    delete[] valuev;
                    return encode(types[5]);
                }
                
                result = valuev[j];
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_symbol(ctr)) {
                delete[] valuev;
                operation_error();
            }
            
            if (is_defined(ctr)) {
                delete[] valuev;
                defined_error(ctr);
            }
            
            if (rhs.empty())
                operation_error();
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                remove_symbol(ctr);
                
                if (ss::is_array(result))
                    type_error(0, 2);
                    //  array != double
                
                if (!result.empty()) {
                    if (is_string(result)) {
                        result = decode(result);
                        
                        if (!result.empty() && result != types[5])
                            break;
                    } else if (stod(result))
                        break;
                }
            }
            
            if (i == valuec) {
                delete[] valuev;
                return to_string(-1);
            }
            
            delete[] valuev;
            
            return to_string(i);
        });
        
        uov[format_oi = uoc++] = new buo("format", [this](const string lhs, const string rhs) {
            if (ss::is_array(lhs))
                type_error(0, 4);
                //  array != string
            
            if (lhs.empty())
                null_error();
            
            if (is_string(lhs)) {
                string text = decode(lhs);
                
                if (text.empty())
                    return encode(text);
                
                string valuev[rhs.length() + 1];
                size_t valuec = parse(valuev, rhs);
                
                if (valuec == 1) {
                    if (rhs.empty()) {
                        size_t i = 0;
                        while (i < text.length() - 1 && (text[i] != '{' || text[i + 1] != '}'))
                            ++i;
                        
                        if (i == text.length() - 1)
                            return encode(text);
                        
                        string pattern = "null";
                        size_t n = text.length() + 1;
                        
                        char str[n + pattern.length() - 2];
                        
                        strcpy(str, text.c_str());
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(str[j], str[j + 1]);
                        
                        --n;
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(str[j], str[j + 1]);
                        
                        --n;
                        
                        for (size_t j = 0; j < pattern.length(); ++j) {
                            str[n] = pattern[j];
                            
                            for (size_t k = n; k > i + j; --k)
                                swap(str[k], str[k - 1]);
                            
                            ++n;
                        }
                        
                        return encode(string(str));
                    }
                    
                    if (is_string(rhs)) {
                        size_t i = 0;
                        while (i < text.length() - 1 && (text[i] != '{' || text[i + 1] != '}'))
                            ++i;
                        
                        if (i == text.length() - 1)
                            return encode(text);
                        
                        string pattern = decode(rhs);
                        size_t n = text.length() + 1;
                        
                        char str[n + pattern.length() - 2];
                        
                        strcpy(str, text.c_str());
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(str[j], str[j + 1]);
                        
                        --n;
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(str[j], str[j + 1]);
                        
                        --n;
                        
                        for (size_t j = 0; j < pattern.length(); ++j) {
                            str[n] = pattern[j];
                            
                            for (size_t k = n; k > i + j; --k)
                                swap(str[k], str[k - 1]);
                            
                            ++n;
                        }
                        
                        return encode(string(str));
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                size_t i = 0;
                                while (i < text.length() - 1 && (text[i] != '{' || text[i + 1] != '}'))
                                    ++i;
                                
                                if (i == text.length() - 1)
                                    return encode(text);
                                
                                string pattern = rtrim(get_number(rhs));
                                size_t n = text.length() + 1;
                                
                                char str[n + pattern.length() - 2];
                                
                                strcpy(str, text.c_str());
                                
                                for (size_t j = i; j < n - 1; ++j)
                                    swap(str[j], str[j + 1]);
                                
                                --n;
                                
                                for (size_t j = i; j < n - 1; ++j)
                                    swap(str[j], str[j + 1]);
                                
                                --n;
                                
                                for (size_t j = 0; j < pattern.length(); ++j) {
                                    str[n] = pattern[j];
                                    
                                    for (size_t k = n; k > i + j; --k)
                                        swap(str[k], str[k - 1]);
                                    
                                    ++n;
                                }
                                
                                return encode(string(str));
                            }
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            size_t j = 0;
                            while (j < text.length() - 1 && (text[j] != '{' || text[j + 1] != '}'))
                                ++j;
                            
                            if (j == text.length() - 1)
                                return encode(text);
                            
                            string pattern = std::get<1>(* stringv[i]);
                            
                            pattern = pattern.empty() ? "null" : decode(pattern);
                            
                            size_t n = text.length() + 1;
                            
                            char str[n + pattern.length() - 2];
                            
                            strcpy(str, text.c_str());
                            
                            for (size_t k = j; k < n - 1; ++k)
                                swap(str[k], str[k + 1]);
                            
                            --n;
                            
                            for (size_t k = j; k < n - 1; ++k)
                                swap(str[k], str[k + 1]);
                            
                            --n;
                            
                            for (size_t k = 0; k < pattern.length(); ++k) {
                                str[n] = pattern[k];
                                
                                for (size_t l = n; l > j + k; --l)
                                    swap(str[l], str[l - 1]);
                                
                                ++n;
                            }
                            
                            return encode(string(str));
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        size_t n = text.length() + 1;
                        char* str = new char[pow2(n)];
                        
                        strcpy(str, text.c_str());
                        
                        size_t j = 0;
                        for (int k = 0; k < (int)n - 1;) {
                            if (str[k] == '{' && str[k + 1] == '}') {
                                if (j == std::get<1>(* arrayv[i]).size())
                                    break;
                                
                                for (size_t l = k; l < n - 1; ++l)
                                    swap(str[l], str[l + 1]);
                                
                                --n;
                                
                                for (size_t l = k; l < n - 1; ++l)
                                    swap(str[l], str[l + 1]);
                                
                                --n;
                                
                                string pattern = std::get<1>(* arrayv[i])[j++];
                                
                                if (pattern.empty())
                                    pattern = "null";
                                
                                else if (is_string(pattern))
                                    pattern = decode(pattern);
                                
                                for (size_t l = 0; l < pattern.length(); ++l) {
                                    if (is_pow(n, 2)) {
                                        char* tmp = new char[n * 2];
                                        
                                        for (size_t m = 0; m < n; ++m)
                                            tmp[m] = str[m];
                                        
                                        delete[] str;
                                        
                                        str = tmp;
                                    }
                                    
                                    str[n] = pattern[l];
                                    
                                    for (size_t m = n; m > k + l; --m)
                                        swap(str[m], str[m - 1]);
                                    
                                    ++n;
                                }
                                
                                k += pattern.length();
                            } else
                                ++k;
                        }
                        
                        string result = encode(string(str));
                        
                        delete[] str;
                        
                        return result;
                    }
                    
                    size_t i = 0;
                    while (i < text.length() - 1 && (text[i] != '{' || text[i + 1] != '}'))
                        ++i;
                    
                    if (i == text.length() - 1)
                        return encode(text);
                    
                    string pattern = rtrim(stod(rhs));
                    size_t n = text.length() + 1;
                    
                    char str[n + pattern.length() - 2];
                    
                    strcpy(str, text.c_str());
                    
                    for (size_t j = i; j < n - 1; ++j)
                        swap(str[j], str[j + 1]);
                    
                    --n;
                    
                    for (size_t j = i; j < n - 1; ++j)
                        swap(str[j], str[j + 1]);
                    
                    --n;
                    
                    for (size_t j = 0; j < pattern.length(); ++j) {
                        str[n] = pattern[j];
                        
                        for (size_t k = n; k > i + j; --k)
                            swap(str[k], str[k - 1]);
                        
                        ++n;
                    }
                    
                    return encode(string(str));
                }
                
                size_t n = text.length() + 1;
                
                char* str = new char[pow2(n)];
                
                strcpy(str, text.c_str());
                
                size_t i = 0;
                for (size_t j = 0; j < n - 1;) {
                    if (str[j] == '{' && str[j + 1] == '}') {
                        if (i == valuec)
                            break;
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        string pattern = valuev[i++];
                        
                        if (pattern.empty())
                            pattern = "null";
                        
                        else if (is_string(pattern))
                            pattern = decode(pattern);
                        
                        for (size_t k = 0; k < pattern.length(); ++k) {
                            if (is_pow(n, 2)) {
                                char* tmp = new char[n * 2];
                                
                                for (size_t m = 0; m < n; ++m)
                                    tmp[m] = str[m];
                                
                                delete[] str;
                                
                                str = tmp;
                            }
                            
                            str[n] = pattern[k];
                            
                            for (size_t l = n; l > j + k; --l)
                                swap(str[l], str[l - 1]);
                            
                            ++n;
                        }
                        
                        j += pattern.length();
                    } else
                        ++j;
                }
                
                string result = encode(string(str));
                
                delete[] str;
                
                return result;
            }
            
            if (is_symbol(lhs)) {
                if (io_array(lhs) != -1)
                    type_error(0, 4);
                    //  array != string
                
                int i = io_string(lhs);
                if (i == -1) {
                    if (is_defined(lhs))
                        type_error(2, 4);
                        //  double != string
                    
                    undefined_error(lhs);
                }
                
                std::get<2>(* stringv[i]).second = true;
                
                string text = std::get<1>(* stringv[i]);
                
                if (text.empty())
                    null_error();
                
                text = decode(text);
                
                if (text.empty())
                    return encode(text);
                
                string valuev[rhs.length() + 1];
                size_t valuec = parse(valuev, rhs);
                
                if (valuec == 1) {
                    if (rhs.empty()) {
                        size_t j = 0;
                        while (j < text.length() - 1 && (text[j] != '{' || text[j + 1] != '}'))
                            ++j;
                        
                        if (j == text.length() - 1)
                            return encode(text);
                        
                        string pattern = "null";
                        size_t n = text.length() + 1;
                        
                        char str[n + pattern.length() - 2];
                        
                        strcpy(str, text.c_str());
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        for (size_t k = 0; k < pattern.length(); ++k) {
                            str[n] = pattern[k];
                            
                            for (size_t l = n; l > j + k; --l)
                                swap(str[l], str[l - 1]);
                            
                            ++n;
                        }
                        
                        return encode(string(str));
                    }
                    
                    if (is_string(rhs)) {
                        size_t j = 0;
                        while (j < text.length() - 1 && (text[j] != '{' || text[j + 1] != '}'))
                            ++j;
                        
                        if (j == text.length() - 1)
                            return encode(text);
                        
                        string pattern = decode(rhs);
                        size_t n = text.length() + 1;
                        
                        char str[n + pattern.length() - 2];
                        
                        strcpy(str, text.c_str());
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        for (size_t k = j; k < n - 1; ++k)
                            swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        for (size_t k = 0; k < pattern.length(); ++k) {
                            str[n] = pattern[k];
                            
                            for (size_t l = n; l > j + k; --l)
                                swap(str[l], str[l - 1]);
                            
                            ++n;
                        }
                        
                        return encode(string(str));
                    }
                    
                    if (is_symbol(rhs)) {
                        int j = io_array(rhs);
                        if (j == -1) {
                            j = io_string(rhs);
                            if (j == -1) {
                                size_t k = 0;
                                while (k < text.length() - 1 && (text[k] != '{' || text[k + 1] != '}'))
                                    ++k;
                                
                                if (k == text.length() - 1)
                                    return encode(text);
                                
                                string pattern = rtrim(get_number(rhs));
                                size_t n = text.length() + 1;
                                
                                char str[n + pattern.length() - 1];
                                
                                strcpy(str, text.c_str());
                                
                                for (size_t l = k; l < n - 1; ++l)
                                    swap(str[l], str[l + 1]);
                                
                                --n;
                                
                                for (size_t l = k; l < n - 1; ++l)
                                    swap(str[l], str[l + 1]);
                                
                                --n;
                                
                                for (size_t l = 0; l < pattern.length(); ++l) {
                                    str[n] = pattern[l];
                                    
                                    for (size_t m = n; m > k + l; --m)
                                        swap(str[m], str[m - 1]);
                                    
                                    ++n;
                                }
                                
                                return encode(string(str));
                            }
                            
                            std::get<2>(* stringv[j]).second = true;
                            
                            size_t k = 0;
                            while (k < text.length() - 1 && (text[k] != '{' || text[k + 1] != '}'))
                                ++k;
                            
                            if (k == text.length() - 1)
                                return encode(text);
                            
                            string pattern = std::get<1>(* stringv[j]);
                            
                            pattern = pattern.empty() ? "null" : decode(pattern);
                            
                            size_t n = text.length() + 1;
                            
                            char str[n + pattern.length() - 2];
                            
                            strcpy(str, text.c_str());
                            
                            for (size_t l = k; l < n - 1; ++l)
                                swap(str[l], str[l + 1]);
                            
                            --n;
                            
                            for (size_t l = k; l < n - 1; ++l)
                                swap(str[l], str[l + 1]);
                            
                            --n;
                            
                            for (size_t l = 0; l < pattern.length(); ++l) {
                                str[n] = pattern[l];
                                
                                for (size_t m = n; m > k + l; --m)
                                    swap(str[m], str[m - 1]);
                                
                                ++n;
                            }
                            
                            return encode(string(str));
                        }
                        
                        std::get<2>(* arrayv[j]).second = true;
                        
                        size_t n = text.length() + 1;
                        
                        char* str = new char[pow2(n)];
                        
                        strcpy(str, text.c_str());
                        
                        size_t k = 0;
                        for (size_t l = 0; l < n - 1;) {
                            if (str[l] == '{' && str[l + 1] == '}') {
                                if (k == std::get<1>(* arrayv[j]).size())
                                    break;
                                
                                for (size_t m = l; m < n - 1; ++m)
                                    swap(str[m], str[m + 1]);
                                
                                --n;
                                
                                for (size_t m = l; m < n - 1; ++m)
                                    swap(str[m], str[m + 1]);
                                
                                --n;
                                
                                string pattern = std::get<1>(* arrayv[j])[k++];
                                
                                if (pattern.empty())
                                    pattern = "null";
                                
                                else if (is_string(pattern))
                                    pattern = decode(pattern);
                                
                                for (size_t m = 0; m < pattern.length(); ++m) {
                                    if (is_pow(n, 2)) {
                                        char* tmp = new char[n * 2];
                                        
                                        for (size_t m = 0; m < n; ++m)
                                            tmp[m] = str[m];
                                        
                                        delete[] str;
                                        
                                        str = tmp;
                                    }
                                    
                                    str[n] = pattern[m];
                                    
                                    for (size_t p = n; p > l + m; --p)
                                        swap(str[p], str[p - 1]);
                                    
                                    ++n;
                                }
                                
                                l += pattern.length();
                            } else
                                ++l;
                        }
                        
                        string result = encode(string(str));
                        
                        delete[] str;
                        
                        return result;
                    }
                    
                    size_t j = 0;
                    while (j < text.length() - 1 && (text[j] != '{' || text[j + 1] != '}'))
                        ++j;
                    
                    if (j == text.length() - 1)
                        return encode(text);
                    
                    string pattern = rtrim(stod(rhs));
                    size_t n = text.length() + 1;
                    
                    char str[n + pattern.length() - 2];
                    
                    strcpy(str, text.c_str());
                    
                    for (size_t k = j; k < n - 1; ++k)
                        swap(str[k], str[k + 1]);
                    
                    --n;
                    
                    for (size_t k = j; k < n - 1; ++k)
                        swap(str[k], str[k + 1]);
                    
                    --n;
                    
                    for (size_t k = 0; k < pattern.length(); ++k) {
                        str[n] = pattern[k];
                        
                        for (size_t l = n; l > j + k; --l)
                            swap(str[l], str[l - 1]);
                        
                        ++n;
                    }
                    
                    return encode(string(str));
                }
                
                size_t n = text.length() + 1;
                
                char* str = new char[pow2(n)];
                
                strcpy(str, text.c_str());
                
                size_t j = 0;
                for (size_t k = 0; k < n - 1;) {
                    if (str[k] == '{' && str[k + 1] == '}') {
                        if (j == valuec)
                            break;
                        
                        for (size_t l = k; l < n - 1; ++l)
                            swap(str[l], str[l + 1]);
                        
                        --n;
                        
                        for (size_t l = k; l < n - 1; ++l)
                            swap(str[l], str[l + 1]);
                        
                        --n;
                        
                        string pattern = valuev[j++];
                        
                        if (pattern.empty())
                            pattern = "null";
                        
                        else if (is_string(pattern))
                            pattern = decode(pattern);
                        
                        for (size_t l = 0; l < pattern.length(); ++l) {
                            if (is_pow(n, 2)) {
                                char* tmp = new char[n * 2];
                                
                                for (size_t m = 0; m < n; ++m)
                                    tmp[m] = str[m];
                                
                                delete[] str;
                                
                                str = tmp;
                            }
                            
                            str[n] = pattern[l];
                            
                            for (size_t m = n; m > k + l; --m)
                                swap(str[m], str[m - 1]);
                            
                            ++n;
                        }
                        
                        k += pattern.length();
                    } else
                        ++k;
                }
                
                string result = encode(string(str));
                
                delete[] str;
                
                return result;
            }
            
            type_error(2, 4);
            //  double != string
            
            return empty();
        });
        
        uov[index_of_oi = uoc++] = new buo("indexof", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    string text = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(0, 4);
                        //  array != string
                    
                    if (rhs.empty())
                        null_error();
                        
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        size_t i = 0;
                        while (i < text.length() && text[i] != pattern[0])
                            ++i;
                        
                        return to_string(i == text.length() ? -1 : (int)i);
                    }
                    
                    if (!is_symbol(rhs))
                        type_error(2, 4);
                        //  double != string
                    
                    if (io_array(rhs) != -1)
                        type_error(0, 4);
                        //  array != string
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(2, 4);
                            //  double != string
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[i]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode(pattern);
                    
                    if (pattern.length() != 1)
                        type_error(4, 1);
                        //  string != char
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    size_t j = 0;
                    while (j < text.length() && text[j] != pattern[0])
                        ++j;
                    
                    return to_string(j == text.length() ? -1 : (int)j);
                }
                
                if (!is_symbol(lhs))
                    type_error(2, 4);
                    //  double != string
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (is_defined(lhs))
                                type_error(2, 4);
                                //  double != string
                            
                            undefined_error(lhs);
                        }
                        
                        string text = std::get<1>(* stringv[i]);
                        
                        if (text.empty())
                            null_error();
                        
                        text = decode(text);
                        
                        if (ss::is_array(rhs))
                            type_error(0, 4);
                            //  array != string
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            string pattern = decode(rhs);
                            
                            if (pattern.length() != 1)
                                type_error(4, 1);
                                //  string != char
                            
                            size_t j = 0;
                            while (j < text.length() && text[j] != pattern[0])
                                ++j;
                            
                            return to_string(j == text.length() ? -1 : (int)j);
                        }
                        
                        if (!is_symbol(rhs))
                            type_error(2, 4);
                            //  double != string
                        
                        if (io_array(rhs) != -1)
                            type_error(0, 4);
                            //  array != string
                        
                        int j = io_string(rhs);
                        if (j == -1) {
                            if (is_defined(rhs))
                                type_error(2, 4);
                                //  double != string
                            
                            undefined_error(rhs);
                        }
                        
                        string pattern = std::get<1>(* stringv[j]);
                        
                        if (pattern.empty())
                            null_error();
                        
                        pattern = decode(pattern);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        std::get<2>(* stringv[i]).second = true;
                        std::get<2>(* stringv[j]).second = true;
                        
                        size_t k = 0;
                        while (k < text.length() && text[k] != pattern[0])
                            ++k;
                        
                        return to_string(k == text.length() ? -1 : (int)k);
                    }
                }
                
                string pattern = element(rhs);
                
                std::get<2>(* arrayv[i]).second = true;
                
                return to_string(std::get<1>(* arrayv[i]).index_of(pattern));
            }
            string pattern = element(rhs);
            
            size_t i = 0;
            while (i < valuec && valuev[i] != pattern)
                ++i;
            
            return to_string(i == valuec ? -1 : (int)i);
        });
        
        uov[insert_oi = uoc++] = new tuo("insert", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            operation_error();
                        
                        undefined_error(lhs);
                    }
                    
                    operation_error();
                }
                
                if (ctr.empty())
                    type_error(4, 3);
                    //  string != int
                
                if (ss::is_array(ctr))
                    type_error(0, 3);
                    //  array != int
                
                if (is_string(ctr))
                    type_error(4, 3);
                    //  string != int
                
                double idx = stod(ctr);
                if (!is_int(idx))
                    type_error(2, 3);
                    //  double != int
                
                if (idx < 0 || idx > std::get<1>(* arrayv[i]).size())
                    range_error("index " + rtrim(idx) + ", count " + to_string(std::get<1>(* arrayv[i]).size()));
                
                if (std::get<2>(* arrayv[i]).first)
                    write_error(lhs);
                
                valuev = new string[rhs.length() + 1];
                valuec = parse(valuev, rhs);
                
                std::get<1>(* arrayv[i]).ensure_capacity(std::get<1>(* arrayv[i]).size() + valuec);
                for (size_t l = 0; l < valuec; ++l)
                    std::get<1>(* arrayv[i]).insert((size_t)idx + l, valuev[l]);
                
                delete[] valuev;
                
                return rhs;
            }
            
            if (ss::is_array(ctr))
                type_error(0, 3);
                //  array != int
            
            if (ctr.empty() || is_string(ctr))
                type_error(4, 3);
                //  string != int
            
            double idx = stod(ctr);
                
            if (!is_int(idx))
                type_error(2, 3);
                //  double != int
            
            if (idx < 0 || idx > valuec)
                range_error(rtrim(idx));
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            string* tmp = new string[valuec + _valuec];
            for (size_t k = 0; k < valuec; ++k)
                tmp[k] = valuev[k];
            
            delete[] valuev;
            
            for (size_t i = 0; i < _valuec; ++i) {
                tmp[valuec] = _valuev[i];
                
                for (size_t j = valuec; j >= (size_t)idx + i + 1; --j)
                    swap(tmp[j], tmp[j - 1]);
                
                ++valuec;
            }
            
            valuev = tmp;
            
            const string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        //  20
        
        uov[join_oi = uoc++] = new buo("join", [this](string lhs, string rhs) {
            string v[lhs.length() + 1];
            size_t p = parse(v, lhs);
            
            if (p == 1) {
                if (lhs.empty() || is_string(lhs))
                    type_error(4, 0);
                    //  string != array
                
                if (!is_symbol(lhs))
                    type_error(2, 0);
                    //  double != array
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            operation_error();
                        
                        undefined_error(lhs);
                    }
                    
                    operation_error();
                }
                
                if (ss::is_array(rhs))
                    type_error(0, 4);
                    //  array != string
                
                if (rhs.empty())
                    null_error();
                
                if (is_string(rhs))
                    rhs = decode(rhs);
                
                else if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 4);
                        //  array != string
                    
                    int j = io_string(rhs);
                    if (j == -1)
                        rhs = rtrim(get_number(rhs));
                    else {
                        rhs = std::get<1>(* stringv[j]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        rhs = decode(rhs);
                        
                        std::get<2>(* stringv[j]).second = true;
                    }
                } else
                    rhs = rtrim(stod(rhs));
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                
                size_t j;
                for (j = 0; j < std::get<1>(* arrayv[i]).size() - 1; ++j) {
                    if (std::get<1>(* arrayv[i])[j].empty())
                        null_error();
                    
                    ss << decode(std::get<1>(* arrayv[i])[j]) << rhs;
                }
                
                if (std::get<1>(* arrayv[i])[j].empty())
                    null_error();
                
                ss << decode(std::get<1>(* arrayv[i])[j]);
                
                return encode(ss.str());
            }
            
            if (ss::is_array(rhs))
                type_error(0, 4);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (is_string(rhs))
                rhs = decode(rhs);
            
            else if (is_symbol(rhs)) {
                int i = io_array(rhs);
                if (i != -1)
                    type_error(0, 4);
                    //  array != string
                
                i = io_string(rhs);
                if (i == -1)
                    rhs = rtrim(get_number(rhs));
                else {
                    rhs = std::get<1>(* stringv[i]);
                    
                    if (rhs.empty())
                        null_error();
                    
                    rhs = decode(rhs);
                    
                    std::get<2>(* stringv[i]).second = true;
                }
            } else
                rhs = rtrim(stod(rhs));
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < p - 1; ++i) {
                if (v[i].empty())
                    null_error();
                
                ss << decode(v[i]) << rhs;
            }
            
            if (v[i].empty())
                null_error();
            
            ss << decode(v[i]);
            
            return encode(ss.str());
        });
        
        uov[last_index_of_oi = uoc++] = new buo("lastindexof", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    if (ss::is_array(rhs))
                        type_error(0, 1);
                        //  array != char
                    
                    if (rhs.empty())
                        null_error();
                    
                    if (is_string(rhs)) {
                        const string pattern = decode(rhs);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        const string text = decode(lhs);
                        
                        int i = (int)text.length() - 1;
                        while (i >= 0 && text[i] != pattern[0])
                            ++i;
                        
                        return to_string(i);
                    }
                    
                    if (is_symbol(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(0, 1);
                            //  array != char
                        
                        int i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                type_error(2, 1);
                                //  double != char
                            
                            undefined_error(rhs);
                        }
                        
                        string pattern = std::get<1>(* stringv[i]);
                        
                        if (pattern.empty())
                            null_error();
                        
                        pattern = decode(pattern);
                        
                        if (pattern.length() != 1)
                            type_error(4, 1);
                            //  string != char
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        const string text = decode(lhs);
                        
                        int j = (int)text.length() - 1;
                        while (j >= 0 && text[i] != pattern[0])
                            --j;
                        
                        return to_string(j);
                    }
                    
                    type_error(2, 1);
                    //  double != char
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (is_defined(lhs))
                                operation_error();
                            
                            undefined_error(lhs);
                        }
                        
                        string text = std::get<1>(* stringv[i]);
                        
                        if (text.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        text = decode(text);
                        
                        if (ss::is_array(rhs))
                            type_error(0, 1);
                            //  array != char
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            const string pattern = decode(rhs);
                            
                            if (pattern.length() != 1)
                                type_error(4, 1);
                                //  string != char
                            
                            int j = (int)text.length() - 1;
                            while (j >= 0 && text[j] != pattern[0])
                                ++j;
                            
                            return to_string(j);
                        }
                        
                        if (is_symbol(rhs)) {
                            if (io_array(rhs) != -1)
                                type_error(0, 1);
                                //  array != char
                            
                            int j = io_string(rhs);
                            if (j == -1) {
                                if (is_defined(rhs))
                                    type_error(2, 1);
                                    //  double != char
                                
                                undefined_error(rhs);
                            }
                            
                            string pattern = std::get<1>(* stringv[j]);
                            
                            if (pattern.empty())
                                null_error();
                            
                            pattern = decode(pattern);
                            
                            if (pattern.length() != 1)
                                type_error(4, 1);
                                //  string != char
                            
                            std::get<2>(* stringv[j]).second = true;
                            
                            int k = (int)text.length() - 1;
                            while (k >= 0 && text[k] != pattern[0])
                                --k;
                            
                            return to_string(k);
                        }
                        
                        type_error(2, 1);
                        //  double != char
                    }
                    
                    string pattern = element(rhs);
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return to_string(std::get<1>(* arrayv[i]).last_index_of(pattern));
                }
                
                operation_error();
            }
            
            string pattern = element(rhs);
            
            int i = (int)valuec - 1;
            while (i >= 0 && valuev[i] != pattern)
                --i;
            
            return to_string(i);
        });
        
        uov[map_oi = uoc++] = new tuo("map", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            operation_error();
                        
                        undefined_error(lhs);
                    }
                    
                    operation_error();
                }
                
                if (!is_symbol(ctr))
                    operation_error();
                
                if (is_defined(ctr))
                    defined_error(ctr);
                
                if (rhs.empty())
                    operation_error();
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < valuec; ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                for (size_t j = 0; j < valuec; ++j) {
                    if (valuev[j].empty() || is_string(valuev[j]))
                        set_string(ctr, valuev[j]);
                    else
                        set_number(ctr, stod(valuev[j]));
                    
                    string result = evaluate(rhs);
                    
                    if (ss::is_array(result))
                        type_error(0, 4);
                        //  array != string
                    
                    valuev[j] = result;
                    
                    remove_symbol(ctr);
                }
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                string result = stringify(valuec, valuev);
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_symbol(ctr))
                operation_error();
            
            if (rhs.empty())
                operation_error();
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                if (!result.empty()) {
                    if (ss::is_array(result)) {
                        delete[] valuev;
                        type_error(0, 4);
                        //  array != string
                    }
                }
                
                valuev[i] = result;
                
                remove_symbol(ctr);
            }
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        uov[reserve_oi = uoc++] = new buo("reserve", [this](const string lhs, const string rhs) {
            if (lhs.empty())
                operation_error();
            
            if (!is_symbol(lhs))
                operation_error();
            
            int i = io_array(lhs);
            if (i == -1) {
                i = io_string(lhs);
                if (i == -1) {
                    if (is_defined(lhs))
                        operation_error();
                    
                    undefined_error(lhs);
                }
                
                operation_error();
                
            }
            
            string v[rhs.length() + 1];
            size_t n = parse(v, rhs);
            
            if (n != 1)
                type_error(0, 3);
                //  array != int
            
            if (is_string(rhs))
                type_error(4, 3);
                //  string != int
            
            double d;
            
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(0, 3);
                    //  array != int
                
                if (io_string(rhs) != -1)
                    type_error(4, 3);
                    //  string != int
                
                d = get_number(rhs);
            } else
                d = stod(rhs);
            
            if (!is_int(d))
                type_error(2, 3);
                //  double != int
            
            if (d < 0)
                range_error(rtrim(d));
            
            const string result = to_string(std::get<1>(* arrayv[i]).capacity());
            
            if (std::get<2>(* arrayv[i]).first)
                write_error(lhs);
            
            std::get<1>(* arrayv[i]).ensure_capacity((size_t)d);
            
            return result;
        });
        //  19
        
        uov[row_oi = uoc++] = new buo("row", [this](string lhs, string rhs) {
            if (lhs.empty())
                type_error(4, 7);
                //  string != table
            
            if (rhs.empty())
                type_error(4, 3);
                //  string != int
            
            string* v = new string[lhs.length() + 1];
            size_t n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    type_error(4, 7);
                    //  string != table
                
                if  (!is_symbol(lhs))
                    type_error(2, 7);
                    //  double != table
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(4, 7);
                        //  string != table
                    
                    if (is_defined(lhs))
                        type_error(2, 7);
                        //  double != table
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(0, 7);
                    //  array != table
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1)
                    type_error(0, 3);
                    //  array != int
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (is_string(rhs)) {
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = decode(rhs);
                    rhs = encode(rhs);
                    
                    size_t j = 0;
                    while (j < (std::get<1>(* arrayv[i]).size() - 1) / c && std::get<1>(* arrayv[i])[j * c + 1] != rhs)
                        ++j;
                    
                    if (j == (std::get<1>(* arrayv[i]).size() - 1) / c)
                        return encode(types[5]);
                        //  undefined
                    
                    stringstream ss;
                    
                    if (c != 1) {
                        size_t k;
                        for (k = 1; k < c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[j * c + k + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[j * c + k + 1];
                    }
                    
                    return ss.str();
                }
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 3);
                        //  array != int
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double idx = get_number(rhs);
                        if (!is_int(idx))
                            type_error(2, 3);
                            //  double != int
                        
                        if (idx < 0 || idx >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                            range_error("index " + rtrim(idx) + ", rows " + to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        stringstream ss;
                        
                        size_t k;
                        for (k = 0; k < c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[idx * c + k + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[idx * c + k + 1];
                        
                        return ss.str();
                    }
                    
                    rhs = std::get<1>(* stringv[j]);
                    
                    if (rhs.empty())
                        null_error();
                    
                    std::get<2>(* arrayv[i]).second = true;
                    std::get<2>(* stringv[j]).second = true;
                    
                    size_t k = 0;
                    while (k < (std::get<1>(* arrayv[i]).size() - 1) / c && std::get<1>(* arrayv[i])[k * c + 1] != rhs)
                        ++k;
                    
                    if (k == (std::get<1>(* arrayv[i]).size() - 1) / c)
                        return encode(types[5]);
                        //  undefined
                    
                    stringstream ss;
                    
                    if (c != 1) {
                        size_t l;
                        for (l = 1; l < c - 1; ++l)
                            ss << std::get<1>(* arrayv[i])[k * c + l + 1] << pattern();
                        ss << std::get<1>(* arrayv[i])[k * c + l + 1];
                    }
                    
                    return ss.str();
                }
                
                double idx = stod(rhs);
                if (!is_int(idx))
                    type_error(2, 3);
                    //  double != int
                
                if (idx < 0 || idx >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                    range_error("index " + rtrim(idx) + ", rows " + to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                
                size_t j;
                for (j = 0; j < c - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[idx * c + j + 1] << pattern();
                ss << std::get<1>(* arrayv[i])[idx * c + j + 1];
                
                return ss.str();
            }
            
            if (!is_table(n, v)) {
                delete[] v;
                type_error(0, 7);
                //  array != table
            }
            
            string _v[rhs.length() + 1];
            if (parse(_v, rhs) != 1) {
                delete[] v;
                type_error(0, 3);
                //  array != int
            }
            
            size_t c = stoi(v[0]);
            
            if (is_string(rhs)) {
                rhs = decode(rhs);
                rhs = encode(rhs);
                
                size_t i = 0;
                while (i < (n - 1) / c && v[i * c + 1] != rhs)
                    ++i;
                
                if (i == (n - 1) / c) {
                    delete[] v;
                    return encode(types[5]);
                    //  undefined
                }
                
                stringstream ss;
                
                if (c != 1) {
                    size_t j;
                    for (j = 1; j < c - 1; ++j)
                        ss << v[i * c + j + 1] << pattern();
                    ss << v[i * c + j + 1];
                }
                
                return ss.str();
            }
            
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] v;
                    type_error(0, 3);
                    //  array != int
                }
                
                int i = io_string(rhs);
                if (i == -1) {
                    double idx = get_number(rhs);
                    if (!is_int(idx)) {
                        delete[] v;
                        type_error(2, 3);
                        //  double != int
                    }
                    
                    if (idx < 0 || idx >= (n - 1) / c) {
                        delete[] v;
                        range_error("index " + rtrim(idx) + ", rows " + to_string((n - 1) / c));
                    }
                    
                    stringstream ss;
                    
                    size_t j;
                    for (j = 0; j < c - 1; ++j)
                        ss << v[(size_t)idx * c + j + 1] << pattern();
                    ss << v[(size_t)idx * c + j + 1];
                    
                    return ss.str();
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty()) {
                    delete[] v;
                    null_error();
                }
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < (n - 1) / c && v[j * c + 1] != rhs)
                    ++j;
                
                if (j == (n - 1) / c) {
                    delete[] v;
                    return encode(types[5]);
                    //  undefined
                }
                
                stringstream ss;
                
                if (c != 1) {
                    size_t k;
                    for (k = 1; k < c - 1; ++k)
                        ss << v[j * c + k + 1] << pattern();
                    ss << v[j * c + k + 1];
                }
                
                return ss.str();
            }
            
            double idx = stod(rhs);
            if (!is_int(idx)) {
                delete[] v;
                type_error(2, 3);
                //  double != int
            }
            
            if (idx < 0 || idx >= (n - 1) / c) {
                delete[] v;
                range_error("index " + rtrim(idx) + ", rows " + to_string((n - 1) / c));
            }
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < c - 1; ++i)
                ss << v[(size_t)idx * c + i + 1] << pattern();
            ss << v[(size_t)idx * c + i + 1];
            
            delete[] v;
            
            return ss.str();
        });
        
        uov[resize_oi = uoc++] = new buo("setcount", [this](const string lhs, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_symbol(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            operation_error();
                        
                        undefined_error(lhs);
                    }
                    
                    operation_error();
                }
                
                if (is_array(rhs))
                    type_error(0, 3);
                    //  array != int
                
                if (rhs.empty() || is_string(rhs))
                    type_error(4, 3);
                    //  string != int
                
                double val;
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 3);
                        //  array != int
                    
                    if (io_string(rhs) != -1)
                        type_error(4, 3);
                        //  string != int
                    
                    val = get_number(rhs);
                } else
                    val = stod(rhs);
                
                if (is_int(val))
                    type_error(2, 3);
                    //  double != int
                
                if (val < 0)
                    range_error(to_string((int)val));
                
                string result = to_string(std::get<1>(* arrayv[i]).size());
                
                if (std::get<2>(*arrayv[i]).first)
                    write_error(lhs);
                
                std::get<1>(* arrayv[i]).resize((int)val);
                
                return result;
            }
            
            if (is_array(rhs)) {
                delete[] valuev;
                type_error(0, 3);
                //  array != int
            }
            
            if (rhs.empty() || is_string(rhs)) {
                delete[] valuev;
                type_error(4, 3);
                //  string != int
            }
            
            double val;
            if (is_symbol(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] valuev;
                    type_error(0, 3);
                    //  array != int
                }
                
                if (io_string(rhs) != -1) {
                    delete[] valuev;
                    type_error(4, 3);
                    //  string != int
                }
                
                val = get_number(rhs);
            } else
                val = stod(rhs);
            
            if (!is_int(val)) {
                delete[] valuev;
                type_error(2, 3);
                //  double != int
            }
            
            if (val < 0) {
                delete[] valuev;
                range_error(to_string((int)val));
            }
            
            while (valuec > val)
                --valuec;
            
            while (valuec < val) {
                if (valuec == lhs.length() + 1) {
                    string* tmp = new string[(size_t)val];
                    
                    for (size_t i = 0; i < valuec; ++i)
                        tmp[i] = valuev[i];
                    
                    delete[] valuev;
                    
                    valuev = tmp;
                }
                
                valuev[valuec++] = empty();
            }
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        
        uov[slice_oi = uoc++] = new tuo("slice", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("slice");
            return nullptr;
        });
        
        uov[splice_oi = uoc++] = new tuo("splice", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("splice");
            return nullptr;
        });
        
        uov[substr_oi = uoc++] = new tuo("substr", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("substr");
            return nullptr;
        });
        
        uov[tospliced_oi = uoc++] = new tuo("tospliced", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("tospliced");
            return nullptr;
        });
        
        uov[concat_oi = uoc++] = new buo("+", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
                        
            if (valuec == 1) {
                if (is_string(lhs)) {
                    if (rhs.empty())
                        null_error();
                    
                    lhs = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(0, 4);
                        //  array != string
                    
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_symbol(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(0, 4);
                            //  array != string
                        
                        int i = io_string(rhs);
                        
                        if (i == -1)
                            rhs = rtrim(get_number(rhs));
                        else {
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            rhs = decode(rhs);
                        }
                    } else
                        rhs = rtrim(stod(rhs));
                    
                    return encode(lhs + rhs);
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        
                        if (i == -1) {
                            double num = get_number(lhs);
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(4, 2);
                                //  string != double
                            
                            if (ss::is_array(rhs))
                                type_error(0, 2);
                                //  array != double
                            
                            if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(0, 2);
                                    //  array != double
                                
                                if (io_string(rhs) != -1)
                                    type_error(4, 2);
                                    //  string != double
                                
                                num += get_number(rhs);
                            } else
                                num += stod(rhs);
                            
                            return rtrim(num);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        lhs = decode(lhs);
                        
                        if (ss::is_array(rhs))
                            type_error(0, 4);
                            //  array != string
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs))
                            rhs = decode(rhs);
                        
                        else if (is_symbol(rhs)) {
                            int j = io_string(rhs);
                            
                            if (j == -1)
                                rhs = rtrim(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode(rhs);
                            }
                        } else
                            rhs = rtrim(stod(rhs));
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return encode(lhs + rhs);
                    }
                    
                    if (!rhs.empty()) {
                        if (!ss::is_array(rhs)) {
                            if (is_string(rhs))
                                rhs = encode(decode(rhs));
                                
                            else if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                
                                if (j == -1) {
                                    j = io_string(rhs);
                                    
                                    if (j == -1)
                                        rhs = rtrim(get_number(rhs));
                                    else {
                                        std::get<2>(* stringv[j]).second = true;
                                        
                                        rhs = std::get<1>(* stringv[j]);
                                    }
                                } else {
                                    std::get<2>(* arrayv[j]).second = true;
                                    
                                    rhs = stringify(std::get<1>(* arrayv[j]));
                                }
                            } else
                                rhs = rtrim(stod(rhs));
                        }
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return stringify(std::get<1>(* arrayv[i])) + pattern() + rhs;
                }
                
                double num = stod(lhs);
                
                if (ss::is_array(rhs))
                    type_error(0, 2);
                    //  array != double
                
                if (rhs.empty() || is_string(rhs))
                    type_error(4, 2);
                    //  string != double
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(0, 2);
                        //  array != double
                    
                    if (io_string(rhs) != -1)
                        type_error(4, 2);
                        //  string != double
                    
                    num += get_number(rhs);
                } else
                    num += stod(rhs);
                
                return rtrim(num);
            }
            
            if (!rhs.empty()) {
                if (!ss::is_array(rhs)) {
                    if (is_string(rhs))
                        rhs = encode(decode(rhs));
                        
                    else if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        
                        if (i == -1) {
                            i = io_string(rhs);
                            
                            if (i == -1)
                                rhs = rtrim(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                std::get<2>(* stringv[i]).second = true;
                            }
                        } else {
                            rhs = stringify(std::get<1>(* arrayv[i]));
                            
                            std::get<2>(* arrayv[i]).second = true;
                        }
                    } else
                        rhs = rtrim(stod(rhs));
                }
            }
            
            return lhs + pattern() + rhs;
        });
        //  22
        
        uov[optional_oi = uoc++]= new buo("??", [this] (string lhs, string rhs) {
            if (ss::is_array(lhs))
                return lhs;
                
            if (lhs.empty()) {
                if (rhs.empty())
                    return empty();
                
                if (is_string(rhs))
                    return encode(decode(rhs));
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    
                    if (i == -1) {
                        i = io_string(rhs);
                        
                        if (i == -1)
                            return rtrim(get_number(rhs));
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return std::get<1>(* stringv[i]);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return stringify(std::get<1>(* arrayv[i]));
                }
                
                return rtrim(stod(rhs));
            }
            
            if (is_string(lhs)) {
                lhs = decode(lhs);
                
                if (lhs == types[5]) {
                    if (rhs.empty())
                        return empty();
                    
                    if (ss::is_array(rhs))
                        return rhs;
                    
                    if (is_string(rhs))
                        return encode(decode(rhs));
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        
                        if (i == -1) {
                            i = io_string(rhs);
                            
                            if (i == -1)
                                return rtrim(get_number(rhs));
                            
                            return std::get<1>(* stringv[i]);
                        }
                        
                        return stringify(std::get<1>(* arrayv[i]));
                    }
                    
                    return rtrim(stod(rhs));
                }
                
                return encode(lhs);
            }
            
            if (is_symbol(lhs)) {
                int i = io_array(lhs);
                
                if (i == -1) {
                    i = io_string(lhs);
                    
                    if (i == -1) {
                        double num = get_number(lhs);
                        
                        if (num)
                            return rtrim(num);
                        
                        if (rhs.empty())
                            return empty();
                        
                        if (is_string(rhs))
                            return encode(decode(rhs));
                        
                        if (is_symbol(rhs)) {
                            int i = io_array(rhs);
                            
                            if (i == -1) {
                                i = io_string(rhs);
                                
                                if (i == -1)
                                    return rtrim(get_number(rhs));
                                
                                std::get<2>(* stringv[i]).second = true;
                                
                                return std::get<1>(* stringv[i]);
                            }
                            
                            std::get<2>(* arrayv[i]).second = true;
                            
                            return stringify(std::get<1>(* arrayv[i]));
                        }
                        
                        return rtrim(stod(rhs));
                    }
                    
                    lhs = std::get<1>(* stringv[i]);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (lhs.empty()) {
                        if (rhs.empty())
                            return empty();
                        
                        if (is_string(rhs))
                            return encode(decode(rhs));
                        
                        if (is_symbol(rhs)) {
                            int i = io_array(rhs);
                            
                            if (i == -1) {
                                i = io_string(rhs);
                                
                                if (i == -1)
                                    return rtrim(get_number(rhs));
                                
                                std::get<2>(* stringv[i]).second = true;
                                
                                return std::get<1>(* stringv[i]);
                            }
                            
                            std::get<1>(* arrayv[i]) = true;
                            
                            return stringify(std::get<1>(* arrayv[i]));
                        }
                        
                        return rtrim(stod(rhs));
                    }
                    
                    return lhs;
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                if (std::get<1>(* arrayv[i]).size() == 1) {
                    if (std::get<1>(* arrayv[i])[0].empty() || (is_string(std::get<1>(* arrayv[i])[0]) &&
                        decode(std::get<1>(* arrayv[i])[0]) == types[5]) || (!is_string(std::get<1>(* arrayv[i])[0]) &&
                         !stod(std::get<1>(* arrayv[i])[0]))) {
                        if (rhs.empty())
                            return rhs;
                        
                        if (is_string(rhs))
                            return encode(decode(rhs));
                        
                        if (is_symbol(rhs)) {
                            i = io_array(rhs);
                            if (i == -1) {
                                i = io_string(rhs);
                                
                                if (i == -1)
                                    return rtrim(get_number(rhs));
                                
                                std::get<2>(* stringv[i]).second = true;
                                
                                return std::get<1>(* stringv[i]);
                            }
                            
                            std::get<2>(* arrayv[i]).second = true;
                            
                            return stringify(std::get<1>(* arrayv[i]));
                        }
                        
                        return rtrim(stod(rhs));
                    }
                    
                    return std::get<1>(* arrayv[i])[0];
                }
                
                return stringify(std::get<1>(* arrayv[i]));
            }
            
            double num = stod(lhs);
            
            if (num)
                return rtrim(num);
            
            if (rhs.empty())
                return empty();
            
            if (is_string(rhs))
                return encode(decode(rhs));
            
            if (is_symbol(rhs)) {
                int i = io_array(rhs);
                
                if (i == -1) {
                    i = io_string(rhs);
                    
                    if (i == -1)
                        return rtrim(get_number(rhs));
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    return std::get<1>(* stringv[i]);
                }
                
                std::get<1>(* arrayv[i]) = true;
                
                return stringify(std::get<1>(* arrayv[i]));
            }
            
            return rtrim(stod(rhs));
        });
        
        uov[relational_oi = uoc++] = new buo("<=", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            if (rhs.empty())
                null_error();
            
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (is_string(lhs)) {
                    if (ss::is_array(rhs))
                        return to_string(1);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return to_string(lhs <= rhs);
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = rtrim(get_number(rhs));
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs <= rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs <= rhs);
                        }
                        
                        return to_string(1);
                    }
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return to_string(1);
                            
                            if (is_string(rhs)) {
                                lhs = rtrim(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a <= b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = rtrim(get_number(lhs));
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs <= rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = rtrim(get_number(lhs));
                                        rhs = decode(rhs);
                                        
                                        return to_string(lhs <= rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = stod(rhs);
                                    
                                    return to_string(a <= b);
                                }
                                
                                return to_string(1);
                            }
                            
                            double a = get_number(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a <= b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (ss::is_array(rhs))
                            return to_string(1);
                        
                        if (is_string(rhs)) {
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs <= rhs);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    lhs = decode(lhs);
                                    rhs = rtrim(get_number(rhs));
                                    
                                    return to_string(lhs <= rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            return to_string(1);
                        }
                        
                        lhs = decode(lhs);
                        rhs = rtrim(stod(rhs));
                        
                        return to_string(lhs <= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    valuev = new string[rhs.length() + 1];
                    valuec = parse(valuev, rhs);
                    
                    if (valuec == 1) {
                        delete[] valuev;
                        
                        if (is_string(rhs)) {
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            return to_string(0);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        double a = stod(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a <= b);
                                    }
                                    
                                    return to_string(0);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs <= rhs);
                                }
                                
                                return to_string(0);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return to_string(0);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return to_string(1);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs > rhs)
                                        return to_string(0);
                                    
                                    if (lhs < rhs)
                                        return to_string(1);
                                } else {
                                    double a = stod(std::get<1>(* arrayv[i])[k]);
                                    double b = stod(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a > b)
                                        return to_string(0);
                                    
                                    if (a < b)
                                        return to_string(1);
                                }
                            }
                            
                            return to_string(1);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs <= rhs);
                            }
                            
                            double a = stod(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a <= b);
                        }
                        
                        return to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                    
                    for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                        if (std::get<1>(* arrayv[i])[k].empty() || valuev[k].empty()) {
                            delete[] valuev;
                            
                            null_error();
                        }
                        
                        if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                            lhs = decode(std::get<1>(* arrayv[i])[k]);
                            rhs = decode(valuev[k]);
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        } else {
                            double a = stod(std::get<1>(* arrayv[i])[k]);
                            double b = stod(valuev[k]);
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return to_string(1);
                }
                
                if (ss::is_array(rhs))
                    return to_string(1);
                
                if (is_string(rhs)) {
                    lhs = rtrim(stod(lhs));
                    rhs = decode(rhs);
                    
                    return to_string(lhs <= rhs);
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = stod(lhs);
                            double b = get_number(rhs);
                            
                            return to_string(a <= b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = rtrim(stod(lhs));
                        rhs = decode(rhs);
                        
                        return to_string(lhs <= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = rtrim(stod(lhs));
                            rhs = decode(rhs);
                            
                            return to_string(lhs <= rhs);
                        }
                        
                        double a = stod(lhs);
                        double b = stod(rhs);
                        
                        return to_string(a <= b);
                    }
                    
                    return to_string(1);
                }
                
                double a = stod(lhs);
                double b = stod(rhs);
                
                return to_string(a <= b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return to_string(0);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return to_string(1);
            }
            
            for (size_t i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || _valuev[i].empty()) {
                    delete[] valuev;
                    
                    null_error();
                }
                
                if (is_string(valuev[i]) || is_string(_valuev[i])) {
                    lhs = decode(valuev[i]);
                    rhs = decode(_valuev[i]);
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                } else {
                    double a = stod(valuev[i]);
                    double b = stod(_valuev[i]);
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                        
                    if (a < b) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return to_string(1);
        });
        //  23
        
        uov[uoc++] = new buo(">=", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            if (rhs.empty())
                null_error();
            
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (is_string(lhs)) {
                    if (ss::is_array(rhs))
                        return to_string(0);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return to_string(lhs >= rhs);
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = rtrim(get_number(rhs));
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs >= rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs >= rhs);
                        }
                        
                        return to_string(0);
                    }
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return to_string(0);
                            
                            if (is_string(rhs)) {
                                lhs = rtrim(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a >= b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = rtrim(get_number(lhs));
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs >= rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = rtrim(get_number(lhs));
                                        rhs = decode(rhs);
                                        
                                        return to_string(lhs >= rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = stod(rhs);
                                    
                                    return to_string(a >= b);
                                }
                                
                                return to_string(0);
                            }
                            
                            double a = get_number(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a >= b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (ss::is_array(rhs))
                            return to_string(0);
                        
                        if (is_string(rhs)) {
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs >= rhs);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    lhs = decode(lhs);
                                    rhs = rtrim(get_number(rhs));
                                    
                                    return to_string(lhs >= rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            return to_string(0);
                        }
                        
                        lhs = decode(lhs);
                        rhs = rtrim(stod(rhs));
                        
                        return to_string(lhs >= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    valuev = new string[rhs.length() + 1];
                    valuec = parse(valuev, rhs);
                    
                    if (valuec == 1) {
                        delete[] valuev;
                        
                        if (is_string(rhs)) {
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            return to_string(1);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        double a = stod(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a >= b);
                                    }
                                    
                                    return to_string(1);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs >= rhs);
                                }
                                
                                return to_string(1);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return to_string(1);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return to_string(0);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs < rhs)
                                        return to_string(0);
                                    
                                    if (lhs > rhs)
                                        return to_string(1);
                                } else {
                                    double a = stod(std::get<1>(* arrayv[i])[k]);
                                    double b = stod(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a < b)
                                        return to_string(0);
                                    
                                    if (a > b)
                                        return to_string(1);
                                }
                            }
                            
                            return to_string(1);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs >= rhs);
                            }
                            
                            double a = stod(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a >= b);
                        }
                        
                        return to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                        if (std::get<1>(* arrayv[i])[k].empty() || valuev[k].empty()) {
                            delete[] valuev;
                            
                            null_error();
                        }
                        
                        if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                            lhs = decode(std::get<1>(* arrayv[i])[k]);
                            rhs = decode(valuev[k]);
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        } else {
                            double a = stod(std::get<1>(* arrayv[i])[k]);
                            double b = stod(valuev[k]);
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return to_string(1);
                }
                
                if (ss::is_array(rhs))
                    return to_string(0);
                
                if (is_string(rhs)) {
                    lhs = rtrim(stod(lhs));
                    rhs = decode(rhs);
                    
                    return to_string(lhs >= rhs);
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = stod(lhs);
                            double b = get_number(rhs);
                            
                            return to_string(a >= b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = rtrim(stod(lhs));
                        rhs = decode(rhs);
                        
                        return to_string(lhs >= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = rtrim(stod(lhs));
                            rhs = decode(rhs);
                            
                            return to_string(lhs >= rhs);
                        }
                        
                        double a = stod(lhs);
                        double b = stod(rhs);
                        
                        return to_string(a >= b);
                    }
                    
                    return to_string(0);
                }
                
                double a = stod(lhs);
                double b = stod(rhs);
                
                return to_string(a >= b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return to_string(1);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return to_string(0);
            }
            
            for (size_t i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || _valuev[i].empty()) {
                    delete[] valuev;
                    
                    null_error();
                }
                
                if (is_string(valuev[i]) || is_string(_valuev[i])) {
                    lhs = decode(valuev[i]);
                    rhs = decode(_valuev[i]);
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                } else {
                    double a = stod(valuev[i]);
                    double b = stod(_valuev[i]);
                    
                    if (a < b) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return to_string(1);
        });
        //  24
        
        uov[uoc++] = new buo("<", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            if (rhs.empty())
                null_error();
            
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (is_string(lhs)) {
                    if (ss::is_array(rhs))
                        return to_string(1);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return to_string(lhs < rhs);
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = rtrim(get_number(rhs));
                                
                                return to_string(lhs < rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs < rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs < rhs);
                        }
                        
                        return to_string(1);
                    }
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return to_string(1);
                            
                            if (is_string(rhs)) {
                                lhs = rtrim(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return to_string(lhs < rhs);
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a < b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = rtrim(get_number(lhs));
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs < rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = rtrim(get_number(lhs));
                                        rhs = decode(rhs);
                                        
                                        return to_string(lhs < rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = stod(rhs);
                                    
                                    return to_string(a < b);
                                }
                                
                                return to_string(1);
                            }
                            
                            double a = get_number(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a < b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (ss::is_array(rhs))
                            return to_string(1);
                        
                        if (is_string(rhs)) {
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs < rhs);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    lhs = decode(lhs);
                                    rhs = rtrim(get_number(rhs));
                                    
                                    return to_string(lhs < rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs < rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs < rhs);
                            }
                            
                            return to_string(1);
                        }
                        
                        lhs = decode(lhs);
                        rhs = rtrim(stod(rhs));
                        
                        return to_string(lhs < rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    valuev = new string[rhs.length() + 1];
                    valuec = parse(valuev, rhs);
                    
                    if (valuec == 1) {
                        delete[] valuev;
                        
                        if (is_string(rhs)) {
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs < rhs);
                            }
                            
                            return to_string(0);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        double a = stod(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a < b);
                                    }
                                    
                                    return to_string(0);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs < rhs);
                                }
                                
                                return to_string(0);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return to_string(0);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return to_string(1);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs > rhs)
                                        return to_string(0);
                                    
                                    if (lhs < rhs)
                                        return to_string(1);
                                } else {
                                    double a = stod(std::get<1>(* arrayv[i])[k]);
                                    double b = stod(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a > b)
                                        return to_string(0);
                                    
                                    if (a < b)
                                        return to_string(1);
                                }
                            }
                            
                            return to_string(0);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs < rhs);
                            }
                            
                            double a = stod(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a < b);
                        }
                        
                        return to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                    
                    for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                        if (std::get<1>(* arrayv[i])[k].empty() || valuev[k].empty()) {
                            delete[] valuev;
                            
                            null_error();
                        }
                        
                        if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                            lhs = decode(std::get<1>(* arrayv[i])[k]);
                            rhs = decode(valuev[k]);
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        } else {
                            double a = stod(std::get<1>(* arrayv[i])[k]);
                            double b = stod(valuev[k]);
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return to_string(0);
                }
                
                if (ss::is_array(rhs))
                    return to_string(1);
                
                if (is_string(rhs)) {
                    lhs = rtrim(stod(lhs));
                    rhs = decode(rhs);
                    
                    return to_string(lhs < rhs);
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = stod(lhs);
                            double b = get_number(rhs);
                            
                            return to_string(a < b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = rtrim(stod(lhs));
                        rhs = decode(rhs);
                        
                        return to_string(lhs < rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = rtrim(stod(lhs));
                            rhs = decode(rhs);
                            
                            return to_string(lhs < rhs);
                        }
                        
                        double a = stod(lhs);
                        double b = stod(rhs);
                        
                        return to_string(a < b);
                    }
                    
                    return to_string(1);
                }
                
                double a = stod(lhs);
                double b = stod(rhs);
                
                return to_string(a < b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return to_string(0);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return to_string(1);
            }
            
            for (size_t i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || _valuev[i].empty()) {
                    delete[] valuev;
                    
                    null_error();
                }
                
                if (is_string(valuev[i]) || is_string(_valuev[i])) {
                    lhs = decode(valuev[i]);
                    rhs = decode(_valuev[i]);
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                } else {
                    double a = stod(valuev[i]);
                    double b = stod(_valuev[i]);
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                        
                    if (a < b) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return to_string(0);
        });
        
        uov[uoc++] = new buo(">", [this](string lhs, string rhs) {
            if (lhs.empty())
                null_error();
            
            if (rhs.empty())
                null_error();
            
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (is_string(lhs)) {
                    if (ss::is_array(rhs))
                        return to_string(0);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return to_string(lhs > rhs);
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = rtrim(get_number(rhs));
                                
                                return to_string(lhs > rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs > rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs > rhs);
                        }
                        
                        return to_string(0);
                    }
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return to_string(0);
                            
                            if (is_string(rhs)) {
                                lhs = rtrim(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return to_string(lhs > rhs);
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a > b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = rtrim(get_number(lhs));
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs > rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = rtrim(get_number(lhs));
                                        rhs = decode(rhs);
                                        
                                        return to_string(lhs > rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = stod(rhs);
                                    
                                    return to_string(a > b);
                                }
                                
                                return to_string(0);
                            }
                            
                            double a = get_number(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a > b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (ss::is_array(rhs))
                            return to_string(0);
                        
                        if (is_string(rhs)) {
                            lhs = decode(lhs);
                            rhs = decode(rhs);
                            
                            return to_string(lhs > rhs);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    lhs = decode(lhs);
                                    rhs = rtrim(get_number(rhs));
                                    
                                    return to_string(lhs > rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs > rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs > rhs);
                            }
                            
                            return to_string(0);
                        }
                        
                        lhs = decode(lhs);
                        rhs = rtrim(stod(rhs));
                        
                        return to_string(lhs > rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    valuev = new string[rhs.length() + 1];
                    valuec = parse(valuev, rhs);
                    
                    if (valuec == 1) {
                        delete[] valuev;
                        
                        if (is_string(rhs)) {
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    null_error();
                                
                                lhs = decode(lhs);
                                rhs = decode(rhs);
                                
                                return to_string(lhs > rhs);
                            }
                            
                            return to_string(1);
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        double a = stod(lhs);
                                        double b = get_number(rhs);
                                        
                                        return to_string(a > b);
                                    }
                                    
                                    return to_string(1);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs > rhs);
                                }
                                
                                return to_string(1);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return to_string(1);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return to_string(0);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs < rhs)
                                        return to_string(0);
                                    
                                    if (lhs > rhs)
                                        return to_string(1);
                                } else {
                                    double a = stod(std::get<1>(* arrayv[i])[k]);
                                    double b = stod(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a < b)
                                        return to_string(0);
                                    
                                    if (a > b)
                                        return to_string(1);
                                }
                            }
                            
                            return to_string(0);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs > rhs);
                            }
                            
                            double a = stod(lhs);
                            double b = stod(rhs);
                            
                            return to_string(a > b);
                        }
                        
                        return to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                        if (std::get<1>(* arrayv[i])[k].empty() || valuev[k].empty()) {
                            delete[] valuev;
                            
                            null_error();
                        }
                        
                        if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                            lhs = decode(std::get<1>(* arrayv[i])[k]);
                            rhs = decode(valuev[k]);
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        } else {
                            double a = stod(std::get<1>(* arrayv[i])[k]);
                            double b = stod(valuev[k]);
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return to_string(0);
                            }
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return to_string(0);
                }
                
                if (ss::is_array(rhs))
                    return to_string(0);
                
                if (is_string(rhs)) {
                    lhs = rtrim(stod(lhs));
                    rhs = decode(rhs);
                    
                    return to_string(lhs > rhs);
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = stod(lhs);
                            double b = get_number(rhs);
                            
                            return to_string(a > b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = rtrim(stod(lhs));
                        rhs = decode(rhs);
                        
                        return to_string(lhs > rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = rtrim(stod(lhs));
                            rhs = decode(rhs);
                            
                            return to_string(lhs > rhs);
                        }
                        
                        double a = stod(lhs);
                        double b = stod(rhs);
                        
                        return to_string(a > b);
                    }
                    
                    return to_string(0);
                }
                
                double a = stod(lhs);
                double b = stod(rhs);
                
                return to_string(a > b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return to_string(1);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return to_string(0);
            }
            
            for (size_t i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || _valuev[i].empty()) {
                    delete[] valuev;
                    
                    null_error();
                }
                
                if (is_string(valuev[i]) || is_string(_valuev[i])) {
                    lhs = decode(valuev[i]);
                    rhs = decode(_valuev[i]);
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                } else {
                    double a = stod(valuev[i]);
                    double b = stod(_valuev[i]);
                    
                    if (a < b) {
                        delete[] valuev;
                        
                        return to_string(0);
                    }
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return to_string(0);
        });
        //  26
        
        uov[equality_oi = uoc++] = new buo("===", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return to_string(1);
                    //  null === null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return to_string(0);
                    //  null === (array)
                    //  null === (string)
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs)) {
                                arithmetic::consume(rhs);
                                
                                return to_string(0);
                                //  null === double
                            }
                            
                            undefined_error(rhs);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return to_string(rhs.empty());
                        //  null === string
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return to_string(0);
                    // null === array
                }
                
                return to_string(0);
                //  null === (double)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs)) {
                    if (rhs.empty())
                        return to_string(0);
                        //  (string) === null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    delete[] v;
                    
                    if (n != 1)
                        return to_string(0);
                        //  (string) === (array)
                    
                    lhs = decode(lhs);
                    
                    if (is_string(rhs)) {
                        rhs = decode(rhs);
                        return to_string(lhs == rhs ? 1 : 0);
                        //  (string) === (string)
                    }
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs)) {
                                    arithmetic::consume(rhs);
                                    return to_string(0);
                                    //  (string) === double
                                }
                                    
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            std::get<2>(* stringv[i]).second = true;
                            
                            if (rhs.empty())
                                return to_string(0);
                                //  (string) === null
                            
                            rhs = decode(rhs);
                            
                            return to_string(lhs == rhs ? 1 : 0);
                            //  (string) === string
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        return to_string(0);
                        //  (string) === array
                    }
                    
                    return to_string(0);
                    //  (str) === double
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            double d = get_number(lhs);
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || rhs.empty() || is_string(rhs))
                                return to_string(0);
                                //  double === null
                                //  double === (array)
                                //  double === (string)
                            
                            if (is_symbol(rhs)) {
                                i = io_array(rhs);
                                if (i != -1) {
                                    std::get<2>(* arrayv[i]).second = true;
                                    return to_string(0);
                                    //  double === array
                                }
                                
                                i = io_string(rhs);
                                if (i != -1) {
                                    std::get<2>(* stringv[i]).second = true;
                                    return to_string(0);
                                    //  double === string
                                }
                                
                                double e = get_number(rhs);
                                if (isnan(d) && isnan(e))
                                    return to_string(1);
                                
                                return to_string(d == e ? 1 : 0);
                                //  double === double
                            }
                            
                            double e = stod(rhs);
                            if (isnan(d) && isnan(e))
                                return to_string(1);
                            
                            return to_string(d == e ? 1 : 0);
                            //  double === (double)
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (lhs.empty()) {
                            if (rhs.empty())
                                return to_string(1);
                                //  null === null
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || is_string(rhs))
                                return to_string(0);
                                //  null === (array)
                                //  null === (string)
                            
                            if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1) {
                                    return to_string(0);
                                    //  null === array
                                }
                                
                                int i = io_string(rhs);
                                if (i == -1) {
                                    if (is_defined(rhs))
                                        return to_string(0);
                                        //  null === double
                                    
                                    undefined_error(rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[i]);
                                std::get<2>(* stringv[i]).second = true;
                                
                                return to_string(rhs.empty() ? 1 : 0);
                                //  null === string
                            }
                            
                            return to_string(0);
                            //  null === (double)
                        }
                        
                        lhs = decode(lhs);
                        
                        if (rhs.empty())
                            return to_string(0);
                            //  string === null
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            return to_string(lhs == rhs ? 1 : 0);
                            //  string === (string)
                        }
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j != -1) {
                                std::get<2>(* arrayv[j]).second = true;
                                return to_string(0);
                                //  str === array
                            }
                            
                            j = io_string(rhs);
                            if (j == -1) {
                                if (is_defined(rhs))
                                    return to_string(0);
                                    //  str === double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[j]);
                            std::get<2>(* stringv[j]).second = true;
                            
                            if (rhs.empty())
                                return to_string(0);
                                //  string === null
                            
                            rhs = decode(rhs);
                            
                            return to_string(lhs == rhs ? 1 : 0);
                            //  string === string
                        }
                        
                        return to_string(0);
                        //  str === (double)
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (rhs.empty())
                        return to_string(0);
                        //  array === null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    
                    if (n == 1) {
                        delete[] v;
                        
                        if (is_string(rhs))
                            return to_string(0);
                            //  array === (str)
                        
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (is_defined(rhs))
                                        return to_string(0);
                                        //  array === double
                                    
                                    undefined_error(rhs);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                return to_string(0);
                                //  array === string
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                return to_string(0);
                                //  array === array
                            
                            size_t k = 0;
                            while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                ++k;
                            
                            return to_string(k == std::get<1>(* arrayv[i]).size() ? 1 : 0);
                            //  array === array
                        }
                        
                        return to_string(0);
                        //  array === (double)
                    }
                        
                    if (std::get<1>(* arrayv[i]).size() != n) {
                        delete[] v;
                        return to_string(0);
                        //  array === (array)
                    }
                    
                    size_t j = 0;
                    while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 1 : 0);
                    //  array === (array)
                }
                
                if (rhs.empty())
                    return to_string(0);
                    //  (double) === null
                
                double d = stod(lhs);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return to_string(0);
                    //  double === (array)
                    //  double === (string)
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1 || io_string(rhs) != -1)
                        return to_string(0);
                        //  double === (array)
                        //  double === (string)
                    
                    double e = get_number(rhs);
                    if (isnan(d) && isnan(e))
                        return to_string(1);
                    
                    return to_string(d == e ? 1 : 0);
                    //  double === double
                }
                
                double e = stod(rhs);
                if (isnan(d) && isnan(e))
                    return to_string(1);
                
                return to_string(d == e ? 1 : 0);
                //  double === (double)
            }
            
            if (rhs.empty()) {
                delete[] v;
                
                return to_string(0);
                //  (array) === null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    
                    return to_string(0);
                    //  (array) === (str)
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return to_string(0);
                                //  (array) === double
                            
                            undefined_error(rhs);
                        }
                        
                        return to_string(0);
                        //  (array) === string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        return to_string(0);
                        //  (array) === array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 1 : 0);
                    //  (array) === array
                }
                
                return to_string(0);
                //  (array) === (double)
            }
            
            if (n != p) {
                delete[] v;
                
                return to_string(0);
                //  (array) === (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return to_string(i == n ? 1 : 0);
            //  (array) === (array)
        });
        //  27
        
        uov[uoc++] = new buo("!==", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return to_string(0);
                    //  null !== null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return to_string(1);
                    //  null !== (array)
                    //  null !== (string)
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1)
                        return to_string(1);
                        //  null !== array
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            return to_string(1);
                            //  null !== double
                        
                        undefined_error(rhs);
                    }
                    
                    rhs = std::get<1>(* stringv[i]);
                    
                    return to_string(rhs.empty() ? 0 : 1);
                    //  null !== str
                }
                
                return to_string(1);
                //  null !== (double)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs)) {
                    if (rhs.empty())
                        return to_string(1);
                        //  (string) !== null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    delete[] v;
                    
                    if (n != 1)
                        return to_string(1);
                        //  (string) !== (array)
                    
                    lhs = decode(lhs);
                    
                    if (is_string(rhs)) {
                        rhs = decode(rhs);
                        return to_string(lhs != rhs ? 1 : 0);
                        //  (string) !== (string)
                    }
                    
                    if (is_symbol(rhs)) {
                        if (io_array(rhs) != -1)
                            return to_string(1);
                            //  (string) !== array
                        
                        int i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return to_string(1);
                                //  (string) !== double
                            
                            undefined_error(rhs);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (rhs.empty())
                            return to_string(1);
                            //  (string) !== null
                        
                        rhs = decode(rhs);
                        
                        return to_string(lhs != rhs ? 1 : 0);
                        //  (string) !== string
                    }
                    
                    return to_string(1);
                    //  (string) !== (d)
                }
                
                if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            double d = get_number(lhs);
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || rhs.empty() || is_string(rhs))
                                return to_string(1);
                                //  double !== null
                                //  double !== (array)
                                //  double !== (string)
                            
                            if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1 || io_string(rhs) != -1)
                                    return to_string(1);
                                    //  double !== array
                                    //  double !== string
                                
                                double e = get_number(rhs);
                                if (isnan(d) && isnan(e))
                                    return to_string(0);
                                
                                return to_string(d != e ? 1 : 0);
                                //  double !== double
                            }
                            
                            double e = stod(rhs);
                            if (isnan(d) && isnan(e))
                                return to_string(0);
                            
                            return to_string(d != e ? 1 : 0);
                            //  double !== (double)
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (lhs.empty()) {
                            if (rhs.empty())
                                return to_string(0);
                                //  null !== null
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || is_string(rhs))
                                return to_string(1);
                                //  null !== (array)
                            
                            if (is_symbol(rhs)) {
                                if (io_array(rhs) != -1)
                                    return to_string(1);
                                    //  null !== array
                                
                                int i = io_string(rhs);
                                if (i == -1) {
                                    if (is_defined(rhs))
                                        return to_string(1);
                                        //  null !== double
                                    
                                    undefined_error(rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[i]);
                                
                                return to_string(rhs.empty() ? 0 : 1);
                                //  null !== string
                            }
                            
                            return to_string(1);
                            //  null !== (double)
                        }
                        
                        lhs = decode(lhs);
                        
                        if (rhs.empty())
                            return to_string(1);
                            //  string !== null
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            return to_string(lhs != rhs ? 1 : 0);
                            //  string !== (string)
                        }
                        
                        if (is_symbol(rhs)) {
                            if (io_array(rhs) != -1)
                                return to_string(1);
                                //  str !== array
                            
                            int j = io_string(rhs);
                            if (j == -1) {
                                if (is_defined(rhs))
                                    return to_string(1);
                                    //  str !== double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[j]);
                            
                            std::get<2>(* stringv[j]).second = true;
                            
                            if (rhs.empty())
                                return to_string(1);
                                //  string !== null
                            
                            rhs = decode(rhs);
                            
                            return to_string(lhs != rhs ? 1 : 0);
                            //  string !== string
                        }
                        
                        return to_string(1);
                        //  string !== (double)
                    }
                    
                    if (rhs.empty())
                        return to_string(1);
                        //  array !== null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    
                    if (n == 1) {
                        delete[] v;
                        
                        if (is_string(rhs))
                            return to_string(1);
                            //  array !== (string)
                            
                        if (is_symbol(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (is_defined(rhs))
                                        return to_string(1);
                                        //  array !== double
                                    
                                    undefined_error(rhs);
                                }
                                
                                return to_string(1);
                                //  array !== string
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                return to_string(1);
                                //  array !== array
                            
                            size_t k = 0;
                            while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                ++k;
                            
                            return to_string(k == std::get<1>(* arrayv[i]).size() ? 0 : 1);
                            //  array !== array
                        }
                        
                        return to_string(1);
                        //  array !== (double)
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() != n) {
                        delete[] v;
                        
                        return to_string(1);
                        //  array !== (array)
                    }
                    
                    size_t j = 0;
                    while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 0 : 1);
                    //  array !== (array)
                }
                
                if (rhs.empty())
                    return to_string(1);
                    //  double !== null
                
                double d = stod(lhs);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return to_string(1);
                    //  double !== (array)
                    //  double !== (string)
                
                if (is_symbol(rhs)) {
                    if (io_array(rhs) != -1 || io_string(rhs) != -1)
                        return to_string(1);
                        // double !== array
                        // double !== string
                    
                    double e = get_number(rhs);
                    if (isnan(d) && isnan(e))
                        return to_string(0);
                    
                    return to_string(d != e ? 1 : 0);
                    //  double !== (double)
                }
                
                double e = stod(rhs);
                if (isnan(d) && isnan(e))
                    return to_string(0);
                
                return to_string(d != e ? 1 : 0);
                //  double !== (double)
            }
            
            if (rhs.empty())
                return to_string(1);
                //  (array) !== null
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    
                    return to_string(1);
                    //  (array) !== (string)
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return to_string(1);
                                //  (array) !== double
                            
                            undefined_error(rhs);
                        }
                        
                        return to_string(1);
                        //  (array) !== string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        
                        return to_string(1);
                        //  (array) !== array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 0 : 1);
                    //  (array) !== array
                }
                
                return to_string(1);
                //  (array) !== (double)
            }
            
            if (n != p) {
                delete[] v;
                return to_string(1);
                //  (array) !== (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return to_string(i == n ? 0 : 1);
            //  (array) !== (array)
        });
        //  28
        
        uov[uoc++] = new buo("==", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return to_string(1);
                    //  null == null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        return to_string(0);
                        //  null == (string)
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs))
                                    return to_string(0);
                                    //  null == double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            return to_string(rhs.empty() ? 1 : 0);
                            //  null == string
                        }
                        
                        return to_string(std::get<1>(* arrayv[i]).size() == 1
                                         && std::get<1>(* arrayv[i])[0].empty()
                                         ? 1 : 0);
                        //  null == array
                    }
                    
                    return to_string(0);
                    //  null == (double)
                }
                
                return to_string(0);
                //  null == (array)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    lhs = decode(lhs);
                
                else if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1)
                            lhs = rtrim(get_number(lhs));
                        else {
                            lhs = std::get<1>(* stringv[i]);
                            
                            if (lhs.empty()) {
                                if (rhs.empty())
                                    return to_string(1);
                                    //  null == null
                                
                                v = new string[rhs.length() + 1];
                                n = parse(v, rhs);
                                delete[] v;
                                
                                if (n == 1) {
                                    if (is_string(rhs))
                                        return to_string(0);
                                        //  null == (string)
                                    
                                    if (is_symbol(rhs)) {
                                        int j = io_array(rhs);
                                        if (j == -1) {
                                            j = io_string(rhs);
                                            if (j == -1) {
                                                if (is_defined(rhs))
                                                    return to_string(0);
                                                    //  null == double
                                                
                                                undefined_error(rhs);
                                            }
                                            
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            return to_string(rhs.empty() ? 1 : 0);
                                            //  null == string
                                        }
                                        
                                        return to_string(std::get<1>(* arrayv[j]).size() == 1
                                                         && std::get<1>(* arrayv[j])[0].empty()
                                                         ? 1 : 0);
                                        //  null == array
                                    }
                                    
                                    return to_string(0);
                                    //  null == (double)
                                }
                                
                                return to_string(0);
                                //  null == (array)
                            }
                            
                            lhs = decode(lhs);
                        }
                    } else {
                        if (rhs.empty())
                            return to_string(std::get<1>(* arrayv[i]).size() == 1
                                             && std::get<1>(* arrayv[i])[0].empty()
                                             ? 1 : 0);
                            //  array == null
                        
                        v = new string[rhs.length() + 1];
                        n = parse(v, rhs);
                        
                        if (n == 1) {
                            delete[] v;
                            
                            if (is_string(rhs)) {
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        return to_string(0);
                                        //  null == (string)
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs == rhs ? 1 : 0);
                                    //  array == (string)
                                }
                                
                                return to_string(0);
                                //  array == (string)
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        if (std::get<1>(* arrayv[i]).size() == 1) {
                                            lhs = std::get<1>(* arrayv[i])[0];
                                            
                                            if (lhs.empty())
                                                return to_string(0);
                                                //  array == (double)
                                            
                                            lhs = decode(lhs);
                                            rhs = rtrim(get_number(rhs));
                                        
                                            return to_string(lhs == rhs ? 1 : 0);
                                            //  array == double
                                        }
                                        
                                        if (is_defined(rhs))
                                            return to_string(0);
                                            //  array == double
                                        
                                        undefined_error(rhs);
                                    }
                                    
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        rhs = std::get<1>(* stringv[j]);
                                        return to_string(lhs == rhs ? 1 : 0);
                                        //  array == string
                                    }
                                    
                                    return to_string(0);
                                    //  array == string
                                }
                                
                                if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                    return to_string(0);
                                    //  array == array
                                
                                size_t k = 0;
                                while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                    ++k;
                            
                                return to_string(k == std::get<1>(* arrayv[i]).size() ? 1 : 0);
                                //  array == array
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    return to_string(0);
                                    //  null == (double)
                                
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs == rhs ? 1 : 0);
                                //  array == (double)
                            }
                            
                            return to_string(0);
                            //  array == (double)
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() != n) {
                            delete[] v;
                            return to_string(0);
                            //  array == (array)
                        }
                        
                        size_t j = 0;
                        while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                            ++j;
                        
                        delete[] v;
                        
                        return to_string(j == n ? 1 : 0);
                        //  array == (array)
                    }
                } else
                    lhs = rtrim(stod(lhs));
                
                if (rhs.empty())
                    return to_string(0);
                    //  (str) == null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1)
                                rhs = rtrim(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                if (rhs.empty())
                                    return to_string(0);
                                    //  (string) == null
                                
                                rhs = decode(rhs);
                            }
                        } else {
                            if (std::get<1>(* arrayv[i]).size() != 1)
                                return to_string(0);
                                //  (string) == array
                            
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                return to_string(0);
                                //  (string) == null
                            
                            rhs = decode(rhs);
                        }
                    } else
                        rhs = rtrim(stod(rhs));
                    
                    return to_string(lhs == rhs ? 1 : 0);
                    //  (string) == (string)
                }
                
                return to_string(0);
                //  (string) == (array)
            }
            
            if (rhs.empty()) {
                delete[] v;
                return to_string(0);
                //  (array) == null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    return to_string(0);
                    //  (array) == (string)
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return to_string(0);
                                //  (array) == d
                            
                            undefined_error(rhs);
                        }
                        
                        return to_string(0);
                        //  (array) == str
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        
                        return to_string(0);
                        //  (array) == array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 1 : 0);
                    //  (array) == array
                }
                
                delete[] v;
                
                return to_string(0);
                //  (array) == (double)
            }
            
            if (n != p) {
                delete[] v;
                return to_string(0);
                //  (array) == (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return to_string(i == n ? 1 : 0);
            //  (array) == (array)
        });
        //  29
        
        uov[uoc++] = new buo("!=", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return to_string(0);
                    //  null != null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        return to_string(1);
                        //  null != (str)
                    
                    if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs))
                                    return to_string(1);
                                    //  null != double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            return to_string(rhs.empty() ? 0 : 1);
                            //  null != string
                        }
                        
                        return to_string(std::get<1>(* arrayv[i]).size() == 1
                                         && std::get<1>(* arrayv[i])[0].empty()
                                         ? 0 : 1);
                        //  null != array
                    }
                    
                    return to_string(1);
                    //  null != (double)
                }
                
                return to_string(1);
                //  null != (array)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    lhs = decode(lhs);
                
                else if (is_symbol(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1)
                            lhs = rtrim(get_number(lhs));
                        else {
                            lhs = std::get<1>(* stringv[i]);
                            
                            if (lhs.empty()) {
                                if (rhs.empty())
                                    return to_string(0);
                                    //  null != null
                                
                                v = new string[rhs.length() + 1];
                                n = parse(v, rhs);
                                delete[] v;
                                
                                if (n == 1) {
                                    if (is_string(rhs))
                                        return to_string(1);
                                        //  null != (string)
                                    
                                    if (is_symbol(rhs)) {
                                        int j = io_array(rhs);
                                        if (j == -1) {
                                            j = io_string(rhs);
                                            if (j == -1) {
                                                if (is_defined(rhs))
                                                    return to_string(1);
                                                    //  null != double
                                                
                                                undefined_error(rhs);
                                            }
                                            
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            return to_string(rhs.empty() ? 0 : 1);
                                            //  null != string
                                        }
                                        
                                        return to_string(std::get<1>(* arrayv[j]).size() == 1
                                                         && std::get<1>(* arrayv[j])[0].empty()
                                                         ? 0 : 1);
                                        //  null != array
                                    }
                                    
                                    return to_string(1);
                                    //  null == (double)
                                }
                                
                                return to_string(1);
                                //  null != (array)
                            }
                            
                            lhs = decode(lhs);
                        }
                    } else {
                        if (rhs.empty())
                            return to_string(std::get<1>(* arrayv[i]).size() == 1
                                             && std::get<1>(* arrayv[i])[0].empty()
                                             ? 0 : 1);
                            //  array != null
                        
                        v = new string[rhs.length() + 1];
                        n = parse(v, rhs);
                        
                        if (n == 1) {
                            delete[] v;
                            
                            if (is_string(rhs)) {
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        return to_string(1);
                                        //  null != (string)
                                    
                                    lhs = decode(lhs);
                                    rhs = decode(rhs);
                                    
                                    return to_string(lhs != rhs ? 1 : 0);
                                    // array != (string)
                                }
                                
                                return to_string(1);
                                //  array != (string)
                            }
                            
                            if (is_symbol(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        if (std::get<1>(* arrayv[i]).size() == 1) {
                                            lhs = std::get<1>(* arrayv[i])[0];
                                            
                                            if (lhs.empty())
                                                return to_string(1);
                                                //  null != double
                                            
                                            lhs = decode(lhs);
                                            rhs = rtrim(get_number(rhs));
                                            
                                            return to_string(lhs != rhs ? 1 : 0);
                                            //  array != double
                                        }
                                        
                                        if (is_defined(rhs))
                                            return to_string(1);
                                            //  array != double
                                        
                                        undefined_error(rhs);
                                    }
                                    
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        rhs = std::get<1>(* stringv[j]);
                                        
                                        return to_string(lhs != rhs ? 1 : 0);
                                        //  array != string
                                    }
                                    
                                    return to_string(1);
                                    //  array != string
                                }
                                
                                if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                    return to_string(1);
                                    //  array != array
                                
                                size_t k = 0;
                                while (k < std::get<1>(* arrayv[i]).size()
                                       && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                    ++k;
                                
                                return to_string(k == std::get<1>(* arrayv[i]).size() ? 0 : 1);
                                //  array != array
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    return to_string(1);
                                    //  null != (double)
                                
                                lhs = decode(lhs);
                                rhs = rtrim(stod(rhs));
                                
                                return to_string(lhs != rhs ? 1 : 0);
                                //  array != (double)
                            }
                            
                            return to_string(1);
                            //  array != (double)
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() != n) {
                            delete[] v;
                            return to_string(1);
                            //  array != (array)
                        }
                        
                        size_t j = 0;
                        while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                            ++j;
                        
                        return to_string(j == n ? 0 : 1);
                        //  array != (array)
                    }
                } else
                    lhs = rtrim(stod(lhs));
                
                if (rhs.empty())
                    return to_string(1);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_symbol(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1)
                                rhs = rtrim(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                if (rhs.empty())
                                    return to_string(1);
                                    //  (string) != null
                                
                                rhs = decode(rhs);
                            }
                        } else {
                            if (std::get<1>(* arrayv[i]).size() != 1)
                                return to_string(1);
                                //  (string) != array
                            
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                return to_string(1);
                                //  (string) != null
                            
                            rhs = decode(rhs);
                        }
                    } else
                        rhs = rtrim(stod(rhs));
                    
                    return to_string(lhs != rhs ? 1 : 0);
                    //  (string) != (string)
                }
                
                return to_string(1);
                //  (str) != (array)
            }
            
            if (rhs.empty()) {
                delete[] v;
                return to_string(1);
                //  (array) != null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    
                    return to_string(1);
                    //  (array) != (string)
                }
                
                if (is_symbol(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return to_string(1);
                                //  (array) != double
                            
                            undefined_error(rhs);
                        }
                        
                        return to_string(1);
                        //  (array) != string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        
                        return to_string(1);
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return to_string(j == n ? 0 : 1);
                    //  (array) != array
                }
                
                delete[] v;
                
                return to_string(1);
                //  (array) != (double)
            }
            
            if (n != p) {
                delete[] v;
                return to_string(1);
                //  (array) != (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return to_string(i == n ? 0 : 1);
            //  (array) != (array)
        });
        //  30
        
        uov[assignment_oi = uoc++] = new buo("+=", [this](string lhs, string rhs) {
            unsupported_error("+=");
            return nullptr;
        });
        //  31
        
        uov[uoc++] = new buo("=", [this](const string lhs, string rhs) {
            unsupported_error("=");
            return nullptr;
        });
        //  32
        
        uov[conditional_oi = uoc++] = new tuo("?", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("?");
            return nullptr;
        });
        //  33
        
        buov = new buo**[7];
        
        buoc[0] = 21;
        buov[0] = new buo*[buoc[0]];
        buov[0][0] = (buo *)uov[aggregate_oi];           //  aggregate
        buov[0][1] = (buo *)uov[cell_oi];                //  cell
        buov[0][2] = (buo *)uov[col_oi];                 //  column
        buov[0][3] = (buo *)uov[contains_oi];            //  contains
        buov[0][4] = (buo *)uov[reserve_oi];             //  ensure
        buov[0][5] = (buo *)uov[fill_oi];                //  fill
        buov[0][6] = (buo *)uov[find_oi];                //  find
        buov[0][7] = (buo *)uov[find_index_oi];          //  find_index
        buov[0][8] = (buo *)uov[filter_oi];              //  filter
        buov[0][9] = (buo *)uov[format_oi];              //  format
        buov[0][10] = (buo *)uov[index_of_oi];           //  index
        buov[0][11] = (buo *)uov[insert_oi];             //  insert
        buov[0][12] = (buo *)uov[join_oi];               //  join
        buov[0][13] = (buo *)uov[last_index_of_oi];      //  last_index_of
        buov[0][14] = (buo *)uov[map_oi];                //  map
        buov[0][15] = (buo *)uov[splice_oi];             //  remove
        buov[0][16] = (buo *)uov[resize_oi];             //  resize
        buov[0][17] = (buo *)uov[row_oi];                //  row
        buov[0][18] = (buo *)uov[slice_oi];              //  slice
        buov[0][19] = (buo *)uov[substr_oi];             //  substr
        buov[0][20] = (buo *)uov[tospliced_oi];          //  tospliced
                
        buoc[1] = 1;
        buov[1] = new buo*[buoc[1]];
        buov[1][0] = (buo *)uov[concat_oi];              //  +
        //  additive
        
        buoc[2] = 1;
        buov[2] = new buo*[buoc[2]];
        buov[2][0] = (buo *)uov[optional_oi];            //  ??
        //  optional
        
        buoc[3] = 4;
        buov[3] = new buo*[buoc[3]];
        buov[3][0] = (buo *)uov[relational_oi];         //  <=
        buov[3][1] = (buo *)uov[relational_oi + 1];     //  >=
        buov[3][2] = (buo *)uov[relational_oi + 2];     //  <
        buov[3][3] = (buo *)uov[relational_oi + 3];     //  >
        //  relational
        
        buoc[4] = 4;
        buov[4] = new buo*[buoc[4]];
        buov[4][0] = (buo *)uov[equality_oi];           //  ===
        buov[4][1] = (buo *)uov[equality_oi + 1];       //  !==
        buov[4][2] = (buo *)uov[equality_oi + 2];       //  ==
        buov[4][3] = (buo *)uov[equality_oi + 3];       //  !=
        //  equality
        
        buoc[5] = 3;
        buov[5] = new buo*[buoc[5]];
        buov[5][0] = (buo *)uov[conditional_oi];        //  ? ,
        buov[5][1] = (buo *)uov[assignment_oi];         //  +=
        buov[5][2] = (buo *)uov[assignment_oi + 1];     //  =
        //  assignment
        
        //  END OPERATORS
        
        arrayv = new tuple<string, ss::array<string>, pair<bool, bool>>*[1];
        bu_arrayv = new pair<size_t, tuple<string, ss::array<string>*,  pair<bool, bool>>**>*[1];
        bu_numberv = new string[1];
        bu_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>>**>*[1];
        stringv = new tuple<string, string, pair<bool, bool>>*[1];
        
        //  BEGIN FUNCTIONS
        
        bu_functionv = new pair<size_t, function_t**>*[1];
        functionv = new function_t*[1];
        
        set_function(new ss::function("array", [this](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (argc == 1) {
                if (ss::is_array(argv[0]))
                    type_error(0, 3);
                    //  array != int
                
                if (argv[0].empty() || is_string(argv[0]))
                    type_error(4, 3);
                    //  string != int
                
                double num = stod(argv[0]);
                
                if (!is_int(num))
                    type_error(2, 3);
                    //  double != int
                
                if (num < 1)
                    range_error(rtrim(num));
                
                return string((size_t)num - 1, pattern()[0]);
            }
            
            for (size_t i = 0; i < argc; ++i)
                if (ss::is_array(argv[i]))
                    type_error(0, 4);
            
            return stringify(argc, argv);
        }));
        
        set_function(new ss::function("client", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
            
            string str = decode(argv[0]);
            
            if (str.empty())
                undefined_error(empty());
            
            if (ss::is_array(argv[1]))
                type_error(0, 3);
                //  array != int
            
            if (argv[1].empty() || is_string(argv[1]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[1]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            int fildes = api::socket_client(str.c_str(), (int)num);
            
            return to_string(fildes);
        }));
        
        set_function(new ss::function("closeConnection", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            try {
                api::mysql_close((int)num);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("closeFile", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            size_t i = 0;
            while (i < filev.size() && filev[i].first != num)
                ++i;
            
            if (i == filev.size())
                undefined_error(rtrim(num));
            
            filev[i].second->close();
            
            delete filev[i].second;

            filev.erase(filev.begin() + i);
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("closeSocket", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            return rtrim(api::socket_close((int)num));
        }));
        
        set_function(new ss::function("connect", [this](const size_t argc, string* argv) {
            if (argc != 3)
                expect_error("3 argument(s), got " + to_string(argc));
            
            for (size_t i = 0; i < 3; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 4);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                if (!is_string(argv[i]))
                    type_error(2, 4);
                    //  double != string
                
                argv[i] = decode(argv[i]);
            }
            
            size_t res;
            
            try {
                res = api::mysql_connect(argv[0], argv[1], argv[2]);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return to_string(res);
        }));
        
        set_function(new ss::function("delete", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
            
            argv[0] = decode(argv[0]);
                
            return to_string(remove(argv[0].c_str()) != -1);
        }));
        
        set_function(new ss::function("exists", [this](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got 0");
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
                
            string str = decode(argv[0]);
            
            if (str.empty())
                undefined_error(empty());
            
            ifstream file;
            
            file.open(str);
            
            bool exists = !!file;
            
            file.close();
            
            return to_string(exists);
        }));
        
        set_function(new ss::function("file", [this](const  size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
            
            argv[0] = decode(argv[0]);
                
            ofstream* file = new ofstream(argv[0]);
            
            if (!file->is_open())
                return to_string(-1);
            
            filev.push_back(pair<int, ofstream*>(filec, file));
            
            return to_string(filec++);
        }));
        
        set_function(new ss::function("gmt", [this](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + to_string(argc));
            
            time_t now = time(0);
            
            tm *gmtm = gmtime(&now);
            char* dt = asctime(gmtm);
            
            return encode(string(dt));
        }));
        
        set_function(new ss::function("input", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + to_string(argc));
            
            string str;
            getline(cin, str);
            
            return raw(str);
        }));
        
        set_function(new ss::function("lap", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            size_t i = 0;
            while (i < stopwatchv.size() && stopwatchv[i].first != num)
                ++i;
            
            if (i == stopwatchv.size())
                return to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatchv[i].second).count();
            
            return rtrim(num);
        }));
        
        set_function(new ss::function("local", [this](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + to_string(argc));
            
            time_t now = time(0);
            char* dt = ctime(&now);
            
            return encode(string(dt));
        }));
        
        set_function(new ss::function("listen", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            for (size_t i = 0; i < 2; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 3);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(4, 3);
                    //  string != int
                
                double num = stod(argv[i]);
                
                if (!is_int(num))
                    type_error(2, 3);
                    //  double != int
                
                if (num < 0)
                    range_error(rtrim(num));
            }
            
            api::socket_listen(stoi(argv[0]), stoi(argv[1]));
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("poll", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            vector<int> val = api::socket_poll((int)num);
            ostringstream ss;
            
            if (val.size()) {
                for (size_t i = 0; i < val.size() - 1; ++i)
                    ss << val[i] << pattern();
                
                ss << val[val.size() - 1];
            }
            
            return ss.str();
        }));
        
        set_function(new ss::function("preparedQuery", [this](size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  double != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != double
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            string str = decode(argv[1]);
            
            for (size_t i = 0; i < 2; ++i) {
                for (size_t j = 0; j < argc - 1; ++j)
                    swap(argv[j], argv[j + 1]);
                
                --argc;
            }
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 4);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                argv[i] = is_string(argv[i]) ? decode(argv[i]) : rtrim(stod(argv[i]));
            }
            
            sql::ResultSet* res = NULL;
            
            try {
                res = api::mysql_prepared_query((size_t)num, str, argc, argv);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            if (res == NULL)
                return encode(types[5]);
                
            size_t ncols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(ncols + 1);
            
            arr.push(to_string(ncols));
            
            for (int i = 0; i < ncols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < ncols; ++i) {
                    string value = res->getString(i + 1);
                    arr.push(is_number(value) ? rtrim(stod(value)) : encode(value));
                }
            }
              
            delete res;
            
            return stringify(arr);
        }));
        
        set_function(new ss::function("preparedUpdate", [this](size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  double != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != double
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            string str = decode(argv[1]);
            
            for (size_t i = 0; i < 2; ++i) {
                for (size_t j = 0; j < argc - 1; ++j)
                    swap(argv[j], argv[j + 1]);
                
                --argc;
            }
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 4);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                    
                argv[i] = is_string(argv[i]) ? decode(argv[i]) : rtrim(stod(argv[i]));
            }
            
            int res;
            
            try {
                res = api::mysql_prepared_update((size_t)num, str, argc, argv);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return to_string(res);
        }));
        
        set_function(new ss::function("query", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            string str = decode(argv[1]);
            sql::ResultSet *res = NULL;
            
            try {
                res = api::mysql_query((size_t)num, str);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            if (res == NULL)
                return encode(types[5]);
                
            size_t ncols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(ncols + 1);
            
            arr.push(to_string(ncols));
            
            for (int i = 0; i < ncols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < ncols; ++i) {
                    string value = res->getString(i + 1);
                    arr.push(is_number(value) ? rtrim(stod(value)) : encode(value));
                }
            }
              
            delete res;
            
            return stringify(arr);
        }));
        
        set_function(new ss::function("rand", [this](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + to_string(argc));
            
            random_device rd;
            mt19937_64 gen(rd());
            uniform_int_distribution<> uid;
            
            return to_string(uid(gen));
        }));
        
        set_function(new ss::function("read", [this](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got 0");
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
            
            string str = decode(argv[0]);
            
            if (argc >= 2) {
                if (ss::is_array(argv[1]))
                    type_error(0, 4);
                    //  array != string
                
                if (argv[1].empty())
                    null_error();
                
                if (!is_string(argv[1]))
                    type_error(2, 4);
                    //  double != string
                
                argv[1] = decode(argv[1]);
                
                string data[1024];
                
                int n = ss::read(data, str, argv[1]);
                
                if (n == -1)
                    return encode(types[5]);
                    //  undefined
                
                return stringify(n, data);
            }
            
            return encode(ss::read(str));
        }));
        
        set_function(new ss::function("recv", [this](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + to_string(argc));
            
            double numv[argc];
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 3);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(4, 3);
                    //  string != int
                
                numv[i] = stod(argv[i]);
                
                if (!is_int(numv[i]))
                    type_error(2, 3);
                    //  double != int
                
                if (numv[i] < 0)
                    range_error(rtrim(numv[i]));
            }
            
            if (argc == 1)
                numv[1] = 0;
            
            string str = api::socket_recv((int)numv[0], (int)numv[1]);
            
            return str.empty() ? encode(types[5]) : encode(str);
        }));
        
        set_function(new ss::function("rename", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            for (size_t i = 0; i < 2; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 4);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                if (!is_string(argv[i]))
                    type_error(2, 4);
                    //  double != string
                
                argv[i] = decode(argv[i]);
            }
            
            rename(argv[0].c_str(), argv[1].c_str());
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("send", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            argv[1] = decode(argv[1]);
            
            num = api::socket_send((int)num, argv[1]);
            
            return rtrim(num);
        }));
        
        set_function(new ss::function("server", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            for (size_t i = 0; i < 2; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(0, 3);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(4, 3);
                    //  string != int
                
                double num = stod(argv[i]);
                
                if (!is_int(num))
                    type_error(2, 3);
                    //  double != int
                
                if (num < 0)
                    range_error(rtrim(num));
            }
            
            int fildes = api::socket_server(stoi(argv[0]), stoi(argv[1]));
            
            return to_string(fildes);
        }));
        
        set_function(new ss::function("setAskBeforeExit", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(0, 3);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            set_read_only("askBeforeExit", false);
            set_number("askBeforeExit", !!num);
            set_read_only("askBeforeExit", true);
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("setSchema", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(0, 3);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            argv[1] = decode(argv[1]);
            
            bool res;
            
            try {
                res = api::mysql_set_schema((size_t)num, argv[1]);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return encode(to_string(res));
        }));
        
        set_function(new ss::function("setTimeout", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(0, 3);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            set_read_only("timeout", false);
            set_number("timeout", num);
            set_read_only("timeout", true);
            
            size_t i = 0;
            while (i < listenerv.size() && std::get<1>(listenerv[i]) != "timeout")
                ++i;
            
            if (i != listenerv.size())
                std::get<2>(listenerv[i])(rtrim(num));
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("setTimeoutMessage", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(2, 4);
                //  double != string
        
            std::get<1>(* stringv[io_string("timeoutMessage")]) = argv[0];
            
            return encode(types[5]);
        }));
        
        set_function(new ss::function("start", [this](const size_t argc, string* argv) {
            if (argc != 0)
                expect_error("0 argument(s), got " + to_string(argc));
            
            size_t num = stopwatchc++;
            
            stopwatchv.push_back(pair<size_t, time_point<steady_clock>>(num, steady_clock::now()));
            
            return to_string(num);
        }));
        
        set_function(new ss::function("stop", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            size_t i = 0;
            while (i < stopwatchv.size() && stopwatchv[i].first != num)
                ++i;
            
            if (i == stopwatchv.size())
                return to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatchv[i].second).count();
            
            stopwatchv.erase(stopwatchv.begin() + i);
            
            return rtrim(num);
        }));
        
        set_function(new ss::function("update", [this](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 3);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(4, 3);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(2, 3);
                //  double != int
            
            if (num < 0)
                range_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            argv[1] = decode(argv[1]);
            
            int res;
            
            try {
                res = api::mysql_update((size_t)num, argv[1]);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return to_string(res);
        }));
            
        set_function(new ss::function("write", [this](const size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            if (argc >= 4)
                expect_error("3 argument(s), got " + to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(0, 4);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (is_string(argv[0])) {
                string str = decode(argv[0]);
                
                if (str.empty())
                    undefined_error(empty());
                
                string valuev[argv[1].length() + 1];
                size_t valuec = parse(valuev, argv[1]);
                
                if (valuec == 1) {
                    if (argv[1].empty())
                        null_error();
                    
                    if (!is_string(argv[1]))
                        type_error(2, 4);
                        //  double != string
                    
                    argv[1] = decode(argv[1]);
                    
                    ss::write(str, argv[1]);
                } else {
                    if (argc != 3)
                        expect_error("3 argument(s), got " + to_string(argc));
                    
                    if (ss::is_array(argv[2]))
                        type_error(0, 4);
                        //  array != string
                    
                    if (argv[2].empty())
                        null_error();
                    
                    if (!is_string(argv[2]))
                        type_error(2, 4);
                        //  double != string
                    
                    argv[2] = decode(argv[2]);
                    
                    ss::write(str, valuec, valuev, argv[2]);
                }
                
                return encode(types[5]);
            }
            
            if (argc != 2)
                expect_error("2 argument(s), got " + to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(4, 3);
                //  double != int
            
            size_t i = 0;
            while (i < filev.size() && filev[i].first != num)
                ++i;
            
            if (i == filev.size())
                undefined_error(rtrim(num));
            
            if (ss::is_array(argv[1]))
                type_error(0, 4);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(2, 4);
                //  double != string
            
            argv[1] = decode(argv[1]);
            
            (* filev[i].second) << argv[1];
            
            return encode(types[5]);
        }));
        
        //  END FUNCTIONS
    }

    int interpreter::io_function(const string symbol) const {
        if (function_bst == NULL)
            return -1;
        
        return index_of(function_bst, symbol);
    }

    bool interpreter::is_mutating(const string expression) const {
        string data[expression.length() + 1];
        size_t n = prefix(data, expression);
        
        size_t i = 0;
        while (i < n && data[i] == "(")
            ++i;
        
        string term = data[i];
        
        size_t j = 0;
        while (j < aoc - 2 && ((j >= 16 && j < 22) || term != aov[j]->opcode()))
            ++j;
        
        if (j == aoc - 2) {
            j = 0;
            while (j < loc - 1 && term != lov[j]->opcode())
                ++j;
            
            if (j == loc - 1) {
                j = 0;
                while (j < uoc && term != uov[j]->opcode())
                    ++j;
                
                if (j == uoc)
                    return n != 1 && is_symbol(term);
                
                return j >= assignment_oi ||
                    j == reserve_oi ||
                    j == insert_oi ||
                    j == splice_oi ||
                    j == resize_oi ||
                    j == shrink_oi;
            }
            
            return false;
        }
        
        return j >= 25;
    }

    void interpreter::kill() {
        while (stack.size() > 1)
            stack.pop();
        
        if (stack.size())
            stack.top()->kill();
    }

    int interpreter::merge_numbers(int n, string* data) const {
        if (n == 1)
            return n;
        
        if (data[0] == ".") {
            if (is_number(data[1])) {
                data[0] += data[1];
                
                for (size_t j = 1; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
            }
        }
        
        for (size_t i = 1; i < n; ++i) {
            if (data[i] == ".") {
                if (data[i - 1] == "(") {
                    if (is_number(data[i + 1])) {
                        data[i] += data[i + 1];
                        
                        for (size_t j = i + 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --n;
                    }
                } else {
                    if (is_number(data[i - 1])) {
                        data[i - 1] += data[i];
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --n;
                        --i;
                        
                        if (i < n - 1) {
                            if (is_number(data[i + 1])) {
                                data[i] += data[i + 1];
                                
                                for (size_t j = i + 1; j < n - 1; ++j)
                                    swap(data[j], data[j + 1]);
                                
                                --n;
                            }
                        }
                    } else {
                        size_t j = 0;
                        while (j < aoc && data[i - 1] != aov[j]->opcode())
                            ++j;
                        
                        if (j == aoc) {
                            j = 0;
                            while (j < loc - 1 && data[i - 1] != lov[j]->opcode())
                                ++j;
                            
                            if (j == loc - 1) {
                                j = 0;
                                while (j < assignment_oi + 1 && data[i - 1] != uov[j]->opcode())
                                    ++j;
                                
                                if (j != assignment_oi + 1) {
                                    if (is_number(data[i + 1])) {
                                        for (size_t k = i + 1; k < n - 1; ++k)
                                            swap(data[k], data[k + 1]);
                                        
                                        --n;
                                    }
                                }
                            } else {
                                if (is_number(data[i + 1])) {
                                    for (size_t k = i + 1; k < n - 1; ++k)
                                        swap(data[k], data[k + 1]);
                                    
                                    --n;
                                }
                            }
                        } else {
                            if (is_number(data[i + 1])) {
                                data[i] += data[i + 1];
                                
                                for (size_t k = i + 1; k < n - 1; ++k)
                                    swap(data[k], data[k + 1]);
                                
                                --n;
                            }
                        }
                    }
                }
            }
        }
        
        return n;
    }

    int interpreter::merge(int n, string* data) const {
        n = merge_numbers(n, data);
        
        //  merge inequality operators
        
        for (int i = 0; i < n - 1; ++i) {
            if (data[i] == "!" && (data[i + 1] == "=" || data[i + 1] == "==")) {
                data[i] += data[i + 1];
                
                for (size_t j = i + 1; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
            }
        }
        
        //  merge logical expressions
        
        for (int i = 1; i < n - 1; ++i) {
            size_t j = 1;
            while (j < loc - 1 && data[i] != lov[j]->opcode())
                ++j;
            
            if (j != loc - 1) {
                int l = i - 1;   int p = -1;
                do {
                    if (data[l] == "(") {
                        ++p;
                        
                        if (!p)
                            break;
                    } else if (data[l] == ")")
                        --p;
                    
                    else if (p == -1) {
                        size_t k = 1;
                        while (k < loc - 1 && data[l] != lov[k]->opcode())
                            ++k;
                        
                        if (k == loc - 1) {
                            k = assignment_oi;
                            while (k < uoc && data[l] != uov[k]->opcode())
                                ++k;
                            
                            if (k != uoc || data[l] == uov[unary_count]->opcode())
                                break;
                        } else
                            break;
                    }
                    
                    --l;
                    
                } while (l >= 0);
                
                for (int k = i - 1; k > l + 1; --k) {
                    data[l + 1] += " " + data[l + 2];
                                        
                    for (size_t m = l + 2; m < n - 1; ++m)
                        swap(data[m], data[m + 1]);
                    --n;
                    --i;
                }
                //  end
                   
                size_t r = i + 1;   p = 1;
                do {
                    if (data[r] == "(")
                        ++p;
                    else if (data[r] == ")") {
                        --p;
                        
                        if (!p)
                            break;
                    } else if (p == 1) {
                        size_t k = 1;
                        while (k < loc - 1 && data[r] != lov[k]->opcode())
                            ++k;
                        
                        if (k != loc - 1 ||
                            data[r] == uov[unary_count]->opcode() ||
                            data[r] == uov[conditional_oi]->opcode())
                                break;
                    }
                    
                    ++r;
                    
                } while (r < n);
                
                for (size_t m = i + 1; m < r - 1; ++m) {
                    data[i + 1] += " " + data[i + 2];
                    
                    for (size_t s = i + 2; s < n - 1; ++s)
                        swap(data[s], data[s + 1]);
                    
                    --n;
                }
                
                ++i;    //  ???
            }
        }
            
        //  merge functionional expressions
        
        //  BEGIN TERNARY OPERATORS
        
        int i = 0;
        while (i < n - 1) {
            string term = tolower(data[i]);
            
            if (term == uov[aggregate_oi]->opcode() ||
                term == uov[cell_oi]->opcode() ||
                term == uov[conditional_oi]->opcode() ||
                term == uov[filter_oi]->opcode() ||
                term == uov[find_oi]->opcode() ||
                term == uov[find_index_oi]->opcode() ||
                term == uov[insert_oi]->opcode() ||
                term == uov[map_oi]->opcode()) {
                size_t j = ++i;       int p = 0;
                do {
                    if (data[j] == "(")
                        ++p;
                    else if (data[j] == ")")
                        --p;
                    
                    if (!p && (data[j + 1] == ")" || data[j + 1] == uov[unary_count]->opcode()))
                        break;
                    
                    ++j;
                    
                } while (j < n - 1);
                
                for (; j > i; --j) {
                    data[i] += " " + data[i + 1];
                    
                    for (size_t k = i + 1; k < n - 1; ++k)
                        swap(data[k], data[k + 1]);
                    
                    --n;
                }
                
                ++i;
                
                p = 0;
                for (j = ++i; j < n; ++j) {
                    if (data[j] == "(")
                        ++p;
                    else if (data[j] == ")")
                        --p;
                    
                    if (p == -1)
                        break;
                }
                            
                for (; j > i + 1; --j) {
                    data[i] += " " + data[i + 1];
                    
                    for (size_t k = i + 1; k < n - 1; ++k)
                        swap(data[k], data[k + 1]);
                    
                    --n;
                }
            }
            
            ++i;
        }
        
        //  END TERNARY OPERATORS
        
        //  merge function arguments
        
        for (i = 0; i < n - 2; ++i) {
            size_t j = 0;
            while (j < functionc && data[i] != functionv[j]->name())
                ++j;
            
            if (j != functionc && data[i + 1] == "(") {
                size_t k = ++i + 1, s = k;   int p = 1;
                while (k < n) {
                    if (data[k] == "(")
                        ++p;
                    else if (data[k] == ")")
                        --p;
                    
                    if (!p)
                        break;
                    ++k;
                }
                
                size_t e = i;   p = 0;
                do {
                    ++e;
                    
                    if (data[e] == "(")
                        ++p;
                    else if (data[e] == ")")
                        --p;
                    else if (!p && data[e] == ",") {
                        for (size_t l = e; l > s + 1; --l) {
                            data[s] += " " + data[s + 1];
                            
                            for (size_t m = s + 1; m < n - 1; ++m)
                                swap(data[m], data[m + 1]);
                            
                            --n;    --k;    --e;
                        }
                        
                        s = e + 1;
                    }
                } while (e < k - 1);
                
                for (size_t l = s; l < e; ++l) {
                    data[s] += " " + data[s + 1];
                    
                    for (size_t m = s + 1; m < n - 1; ++m)
                        swap(data[m], data[m + 1]);
                    --n;
                }
            }
        }
        
        i = 0;
        while (i < n - 2) {
            string term = tolower(data[i]);
            
            if (term == uov[splice_oi]->opcode()
                || term == uov[slice_oi]->opcode()
                || term == uov[substr_oi]->opcode()
                || term == uov[tospliced_oi]->opcode()) {
                size_t j = ++i;       int p = 0;
                do {
                    if (data[j] == "(")
                        ++p;
                    else if (data[j] == ")")
                        --p;
                    
                    if (!p && (data[j + 1] == ")" || data[j + 1] == ","))
                        break;
                    
                    ++j;
                    
                } while (j < n - 1);
                
                for (; j > i; --j) {
                    data[i] += " " + data[i + 1];
                    
                    for (size_t k = i + 1; k < n - 1; ++k)
                        swap(data[k], data[k + 1]);
                    --n;
                }
                
                if (i != n - 1 && data[i + 1] == ",") {
                    ++i;
                    
                    p = 0;
                    for (j = ++i; j < n; ++j) {
                        if (data[j] == "(")
                            ++p;
                        else if (data[j] == ")")
                            --p;
                        
                        if (p == -1)
                            break;
                    }
                                
                    for (; j > i + 1; --j) {
                        data[i] += " " + data[i + 1];
                        
                        for (size_t k = i + 1; k < n - 1; ++k)
                            swap(data[k], data[k + 1]);
                        --n;
                    }
                }
            }
            
            ++i;
        }
        
        return n;
    }

    int interpreter::prefix(string* dst, const string src) const {
        int n = (int)split(dst, src);
        
        //  array indexer
        
        for (int i = 1; i < n - 1; ++i) {
            if (dst[i] == uov[indexer_oi]->opcode()) {
                size_t l = i;   int p = 0;
                do {
                    --l;
                    
                    if (dst[l] == "(")
                        ++p;
                    
                    else if (dst[l] == ")")
                        --p;
                        
                } while (l > 0 && p);
                
                size_t j = 0;
                while (j < functionc && dst[l - 1] != functionv[j]->name())
                    ++j;
                
                if (j != functionc)
                    --l;
                
                dst[n] = "(";
                
                for (size_t j = n; j > l; --j)
                    swap(dst[j], dst[j - 1]);
                
                ++n;
                
                size_t r;
                for (r = ++i; r < n - 1; ++r) {
                    string term = toupper(dst[r + 1]);
                    
                    size_t j = 0;
                    while (j < arithmetic::unary_count && term != aov[j]->opcode())
                        ++j;
                    
                    if (j == arithmetic::unary_count && dst[r + 1] != lov[0]->opcode()) {
                        term = tolower(dst[r + 1]);
                        
                        j = 0;
                        while (j < unary_count && term != uov[j]->opcode())
                            ++j;
                        
                        if (j == unary_count) {
                            j = 0;
                            
                            while (j < functionc && dst[r + 1] != functionv[j]->name())
                                ++j;
                            
                            if (j == functionc)
                                break;
                        }
                    }
                }
                
                p = 0;
                do {
                    ++r;
                    
                    if (dst[r] == "(")
                        ++p;
                    
                    else if (dst[r] == ")")
                        --p;
                    
                } while (p);
                
                dst[n] = ")";
                
                for (size_t j = n; j > r + 1; --j)
                    swap(dst[j], dst[j - 1]);
                ++n;
                
                for (size_t j = i; j > l + 1; --j)
                    swap(dst[j], dst[j - 1]);
                ++i;
            }
        }
        
        //  arithmetic binary operators
        
        for (size_t i = 0; i < 9; ++i) {
            if (i == 5 || i == 6)
                continue;
                //  relational and equality operators
            
            for (int j = 1; j < n - 1; ++j) {
                if (dst[j] == uov[concat_oi]->opcode() ||
                    dst[j] == uov[assignment_oi + 1]->opcode() ||
                    dst[j] == uov[assignment_oi]->opcode())
                    continue;
                
                string term = toupper(dst[j]);
                
                size_t k = 0;
                while (k < baoc[i] && term != baov[i][k]->opcode())
                    ++k;
                
                if (k != baoc[i]) {
                    size_t l = j;   int p = 0;
                    do {
                        --l;
                        
                        if (dst[l] == "(")
                            ++p;
                        
                        else if (dst[l] == ")")
                            --p;
                        
                    } while (p);
                    
                    while (l > 0) {
                        term = toupper(dst[l - 1]);
                        
                        size_t m = 0;
                        while (m < arithmetic::unary_count && term != aov[m]->opcode())
                            ++m;
                        
                        if (m == arithmetic::unary_count && dst[l - 1] != lov[0]->opcode()) {
                            term = tolower(dst[l - 1]);
                            
                            m = 0;
                            while (m < unary_count && term != uov[m]->opcode())
                                ++m;
                            
                            if (m == unary_count) {
                                m = 0;
                                while (m < functionc && dst[l - 1] != functionv[m]->name())
                                    ++m;
                                
                                if (m == functionc)
                                    break;
                            }
                        }
                        
                        --l;
                    }
                    
                    dst[n] = "(";
                    
                    for (size_t m = n; m > l; --m)
                        swap(dst[m], dst[m - 1]);
                    ++n;
                    
                    size_t r = ++j;
                                    
                    while (r < n - 1) {
                        term = toupper(dst[r + 1]);
                        
                        size_t m = 0;
                        while (m < arithmetic::unary_count && term != aov[m]->opcode())
                            ++m;
                        
                        if (m == arithmetic::unary_count && dst[r + 1] != lov[0]->opcode()) {
                            term = tolower(dst[r + 1]);
                            
                            m = 0;
                            while (m < unary_count && term != uov[m]->opcode())
                                ++m;
                            
                            if (m == unary_count) {
                                m = 0;
                                while (m < functionc && dst[r + 1] != functionv[m]->name())
                                    ++m;
                                
                                if (m == functionc)
                                    break;
                            }
                        }
                        
                        ++r;
                    }
                    
                    p = 0;
                    
                    do {
                        ++r;
                        
                        if (dst[r] == "(")
                            ++p;
                        
                        else if (dst[r] == ")")
                            --p;
                        
                    } while (p);
                    
                    dst[n] = ")";
                    
                    for (size_t m = n; m > r + 1; --m)
                        swap(dst[m], dst[m - 1]);
                    
                    ++n;
                    
                    for (size_t m = j; m > l + 1; --m)
                        swap(dst[m], dst[m - 1]);
                    
                    ++j;
                }
            }
        }
        
        //  logical binary operators
        
        for (size_t i = 1; i < loc - 1; ++i) {
            for (int j = 1; j < n - 1; ++j) {
                if (dst[j] == lov[i]->opcode()) {
                    size_t l = j;   int p = 0;
                    
                    do {
                        --l;
                        
                        if (dst[l] == "(")
                            ++p;
                        
                        else if (dst[l] == ")")
                            --p;
                        
                    } while (l > 0 && p);
                    
                    while (l > 0) {
                        if (dst[l - 1] != lov[0]->opcode()) {
                            string term = toupper(dst[l - 1]);
                            
                            size_t k = 0;
                            while (k < arithmetic::unary_count && term != aov[k]->opcode())
                                ++k;
                            
                            if (k == arithmetic::unary_count) {
                                term = tolower(dst[l - 1]);
                                
                                k = 0;
                                while (k < unary_count && term != uov[k]->opcode())
                                    ++k;
                                
                                if (k == unary_count) {
                                    k = 0;
                                    while (k < functionc && dst[l - 1] != functionv[k]->name())
                                        ++k;
                                    
                                    if (k == functionc)
                                        break;
                                }
                            }
                        }
                        
                        --l;
                    }
                    
                    dst[n] = "(";
                    
                    for (size_t k = n; k > l; --k)
                        swap(dst[k], dst[k - 1]);
                    
                    n++;
                    
                    size_t r = ++j;
                    
                    while (r < n - 1) {
                        if (dst[r + 1] != lov[0]->opcode()) {
                            string term = toupper(dst[r + 1]);
                            
                            size_t k = 0;
                            while (k < arithmetic::unary_count && term != aov[k]->opcode())
                                ++k;
                            
                            if (k == arithmetic::unary_count) {
                                term = tolower(dst[r + 1]);
                                
                                k = 0;
                                while (k < unary_count && term != uov[k]->opcode())
                                    ++k;
                                
                                if (k == unary_count) {
                                    k = 0;
                                    while (k < functionc && dst[r + 1] != functionv[k]->name())
                                        ++k;
                                    
                                    if (k == functionc)
                                        break;
                                }
                            }
                        }
                        
                        ++r;
                    }
                    
                    p = 0;
                    
                    do {
                        ++r;
                        
                        if (dst[r] == "(")
                            ++p;
                        
                        else if (dst[r] == ")")
                            --p;
                        
                    } while (r < n - 1 && p);
                    
                    dst[n] = ")";
                    
                    for (size_t k = n; k > r + 1; --k)
                        swap(dst[k], dst[k - 1]);
                    
                    ++n;
                    
                    for (size_t k = j; k > l + 1; --k)
                        swap(dst[k], dst[k - 1]);
                    
                    ++j;
                }
            }
        }
            
        for (int i = 0; i < 6; ++i) {
            for (int j = 1; j < n - 1; ++j) {
                string term = tolower(dst[j]);
                
                int k = 0;
                while (k < buoc[i] && buov[i][k]->opcode() != term)
                    ++k;
                
                if (k != buoc[i]) {
                    int l = j, p = 0;
                    
                    do {
                        --l;
                        
                        if (dst[l] == "(")
                            ++p;
                        
                        else if (dst[l] == ")")
                            --p;
                        
                    } while (l > 0 && p);
                    
                    while (l > 0) {
                        term = tolower(dst[l - 1]);
                        
                        int m = 0;
                        while (m < unary_count && uov[m]->opcode() != term)
                            ++m;
                        
                        if (m == unary_count && dst[l - 1] != lov[0]->opcode()) {
                            term = toupper(dst[l - 1]);
                            
                            m = 0;
                            while (m < arithmetic::unary_count && term != aov[m]->opcode())
                                ++m;
                            
                            if (m == arithmetic::unary_count) {
                                m = 0;
                                while (m < functionc && dst[l - 1] != functionv[m]->name())
                                    ++m;
                                
                                if (m == functionc)
                                    break;
                            }
                        }
                        
                        --l;
                    }
                    
                    dst[n] = "(";
                    
                    for (size_t m = n; m > l; --m)
                        swap(dst[m], dst[m - 1]);
                    
                    ++n;
                    
                    int r = ++j;
                    
                    while (r < n - 1) {
                        term = tolower(dst[r + 1]);
                        
                        int m = 0;
                        while (m < unary_count && uov[m]->opcode() != term)
                            ++m;
                            //  check for unary string operator
                        
                        if (m == unary_count && dst[r + 1] != lov[0]->opcode()) {
                            term = toupper(dst[r + 1]);
                            
                            m = 0;
                            while (m < arithmetic::unary_count && term != aov[m]->opcode())
                                ++m;
                            
                            if (m == arithmetic::unary_count) {
                                m = 0;
                                while (m < functionc && dst[r + 1] != functionv[m]->name())
                                    ++m;
                                
                                if (m == functionc)
                                    break;
                            }
                        }
                        
                        ++r;
                    }
                    
                    p = 0;
                    
                    do {
                        ++r;
                        
                        if (dst[r] == "(")
                            ++p;
                        
                        else if (dst[r] == ")")
                            --p;
                        
                    } while (r < n - 1 && p);
                    
                    dst[n] = ")";
                    
                    for (size_t m = n; m > r + 1; --m)
                        swap(dst[m], dst[m - 1]);
                    ++n;
                    
                    for (size_t m = j; m > l + 1; --m)
                        swap(dst[m], dst[m - 1]);
                    ++j;
                }
            }
        }
        
        return n;
    }

    void interpreter::reload() {
        restore(buid, false);
        buid = backup();
    }

    void interpreter::remove_listener(const string symbol) {
        size_t i = 0;
        while (i < listenerv.size() &&  std::get<1>(listenerv[i]) != symbol)
            ++i;
        
        if (i == -1)
            undefined_error(symbol);
        
        listenerv.erase(listenerv.begin() + i);
    }

    void interpreter::remove_symbol(const string symbol) {
        int i = io_function(symbol);
        
        if (i == -1) {
            i = io_array(symbol);
            
            if (i == -1) {
                i = io_string(symbol);
                
                if (i == -1) {
                    arithmetic::remove_symbol(symbol);
                    return;
                }
                
                for (size_t j = i; j < stringc - 1; ++j)
                    swap(stringv[j], stringv[j + 1]);
                
                delete stringv[--stringc];
                
                arithmetic::remove_symbol(symbol);
                
                string_bst->close();
                
                string symbolv[stringc];
                
                for (size_t j = 0; j < stringc; ++j)
                    symbolv[j] = std::get<0>(* stringv[j]);
                
                string_bst = stringc ? build(symbolv, 0, (int)stringc) : NULL;
                
                return;
            }
            
            for (size_t j = i; j < arrayc - 1; ++j)
                swap(arrayv[j], arrayv[j + 1]);
            
            delete arrayv[--arrayc];
            
            arithmetic::remove_symbol(symbol);
            
            array_bst->close();
            
            string symbolv[arrayc];
            
            for (size_t j = 0; j < arrayc; ++j)
                symbolv[j] = std::get<0>(* arrayv[j]);
            
            array_bst = arrayc ? build(symbolv, 0, (int)arrayc) : NULL;
        }
        
        for (size_t j = i; j < functionc - 1; ++j)
            swap(functionv[j], functionv[j + 1]);
        
        --functionc;
        
        if (function_bst != NULL)
            function_bst->close();
        
        string symbolv[functionc];
        
        for (size_t j = 0; j < functionc; ++j)
            symbolv[j] = functionv[j]->name();
        
        function_bst = functionc ? build(symbolv, 0, (int)functionc) : NULL;
    }

    void interpreter::restore(const string uuid, bool verbose, bool update, size_t symbolc, string* symbolv) {
        //  find backup
        size_t i = 0;
        while (i < backupc && bu_numberv[i] != uuid)
            ++i;
        
        //  no such backup
        if (i == backupc)
            undefined_error(uuid);
                
        //  restore arrays
        for (size_t j = 0; j < arrayc; ++j) {
            
            //  find array
            size_t k = 0;
            while (k < bu_arrayv[i]->first && std::get<0>(* bu_arrayv[i]->second[k]) != std::get<0>(* arrayv[j]))
                ++k;
            
            //  no such array
            if (k == bu_arrayv[i]->first) {
                if (verbose)
                    if (!std::get<2>(* arrayv[j]).second)
                        logger_write("Unused variable '" + std::get<0>(* arrayv[j]) + "'\n");
            } else if (update) {
                //  find exception
                size_t l = 0;
                while (l < symbolc && std::get<0>(* arrayv[j]) != symbolv[l])
                    ++l;
                
                //  no such exception
                if (l == symbolc) {
                    //  update array
                    delete std::get<1>(* bu_arrayv[i]->second[k]);
                    
                    std::get<1>(* bu_arrayv[i]->second[k]) = new ss::array<string>(std::get<1>(* arrayv[j]));
                    std::get<2>(* bu_arrayv[i]->second[k]).second = std::get<2>(* arrayv[j]).second;
                } else {
                    //  remove exception
                    for (; l < symbolc - 1; ++l)
                        swap(symbolv[l], symbolv[l + 1]);
                    
                    --symbolc;
                }
            }
        }
        
        //  deallocate arrays
        for (size_t j = 0; j < arrayc; ++j)
            delete arrayv[j];
        
        delete[] arrayv;
        
        //  copy arrays
        arrayc = bu_arrayv[i]->first;
        arrayv = new tuple<string, ss::array<string>, pair<bool, bool>>*[pow2(arrayc)];
        
        for (size_t j = 0; j < arrayc; ++j) {
            string first = std::get<0>(* bu_arrayv[i]->second[j]);
            ss::array<string> second = *std::get<1>(* bu_arrayv[i]->second[j]);
            pair<bool, bool> third = std::get<2>(* bu_arrayv[i]->second[j]);
            
            arrayv[j] = new tuple<string, ss::array<string>, pair<bool, bool>>(first, second, third);
            
            delete std::get<1>(* bu_arrayv[i]->second[j]);
            delete bu_arrayv[i]->second[j];
        }
        
        delete[] bu_arrayv[i]->second;
        delete bu_arrayv[i];
        
        //  remove arrays backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_arrayv[j], bu_arrayv[j + 1]);
        
        //  rebuild arrays tree
        if (array_bst != NULL)
            array_bst->close();
        
        string* _symbolv = new string[arrayc];
        
        for (size_t i = 0; i < arrayc; ++i)
            _symbolv[i] = std::get<0>(* arrayv[i]);
        
        array_bst = arrayc ? build(_symbolv, 0, (int)arrayc) : NULL;
        
        delete[] _symbolv;
        
        //  restore functions
        if (verbose) {
            for (size_t j = 0; j < functionc; ++j) {
                
                //  find function
                size_t k = 0;
                while (k < bu_functionv[i]->first && bu_functionv[i]->second[k]->name() != functionv[j]->name())
                    ++k;
                
                //  no such function
                if (k == bu_functionv[i]->first && !functionv[j]->count())
                    logger_write("Unused function '" + functionv[j]->name() + "'\n");
            }
        }
        
        //  deallocate functions
        delete[] functionv;
        
        //  copy functions
        functionc = bu_functionv[i]->first;
        functionv = bu_functionv[i]->second;
        
        delete bu_functionv[i];
        
        //  remove functions backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_functionv[j], bu_functionv[j + 1]);
        
        //  rebuild functions tree
        if (function_bst != NULL)
            function_bst->close();
        
        _symbolv = new string[functionc];
        
        for (size_t j = 0; j < functionc; ++j)
            _symbolv[j] = functionv[j]->name();
        
        function_bst = functionc ? build(_symbolv, 0, (int)functionc) : NULL;
        
        delete[] _symbolv;
        
        //  restore numbers
        arithmetic::restore(bu_numberv[i], verbose, update, symbolc, symbolv);
        
        //  remove numbers backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_numberv[j], bu_numberv[j + 1]);
        
        //  restore strings
        for (size_t j = 0; j < stringc; ++j) {
            //  find string
            size_t k = 0;
            while (k < bu_stringv[i]->first && std::get<0>(* stringv[j]) != std::get<0>(* bu_stringv[i]->second[k]))
                ++k;
            
            //  no such string
            if (k == bu_stringv[i]->first) {
                if (verbose)
                    if (!std::get<2>(* stringv[j]).second)
                        logger_write("Unused variable '" + std::get<0>(* stringv[j]) + "'\n");
            } else if (update) {
                //  find exception
                size_t l = 0;
                while (l < symbolc && std::get<0>(* stringv[j]) != symbolv[l])
                    ++l;
                
                //  no such exception
                if (l == symbolc) {
                    std::get<1>(* bu_stringv[i]->second[k]) = std::get<1>(* stringv[j]);
                    std::get<2>(* bu_stringv[i]->second[k]).second = std::get<2>(* stringv[j]).second;
                } else {
                    //  remove exception
                    for (; l < symbolc - 1; ++l)
                        swap(symbolv[l], symbolv[l + 1]);
                    
                    --symbolc;
                }
            }
        }
        
        //  deallocate strings
        for (size_t j = 0; j < stringc; ++j)
            delete stringv[j];
        
        delete[] stringv;
        
        //  copy strings
        stringc = bu_stringv[i]->first;
        stringv = bu_stringv[i]->second;
        
        delete bu_stringv[i];
        
        //  remove strings backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_stringv[j], bu_stringv[j + 1]);
        
        //  rebuild strings tree
        if (string_bst != NULL)
            string_bst->close();
        
        _symbolv = new string[stringc];
        
        for (size_t i = 0; i < stringc; ++i)
            _symbolv[i] = std::get<0>(* stringv[i]);
        
        string_bst = stringc ? build(_symbolv, 0, (int)stringc) : NULL;
        
        delete[] _symbolv;
        
        --backupc;
    }

    void interpreter::set_array(const string symbol, const size_t index, const string value) {
        if (!is_symbol(symbol))
            expect_error("symbol: " + symbol);
        
        if (!value.empty() && !is_string(value) && !is_number(value))
            throw error("Unexpected token: " + value);
        
        int i = io_array(symbol);
        
        if (i == -1) {
            if (is_defined(symbol))
                defined_error(symbol);
            
            if (is_pow(arrayc, 2)) {
                tuple<string, ss::array<string>, pair<bool, bool>>** _arrayv = new tuple<string, ss::array<string>, pair<bool, bool>>*[arrayc * 2];
                
                for (size_t j = 0; j < arrayc; ++j)
                    _arrayv[j] = arrayv[j];
                
                delete[] arrayv;
                arrayv = _arrayv;
            }
            
            arrayv[arrayc] = new tuple<string, ss::array<string>, pair<bool, bool>>(symbol, ss::array<string>(), pair<bool, bool>(false, false));
            
            size_t j = arrayc++;
            while (j > 0 && std::get<0>(* arrayv[j]) < std::get<0>(* arrayv[j - 1])) {
                swap(arrayv[j], arrayv[j - 1]);
                --j;
            }
            
            std::get<1>(* arrayv[j]).push(value);
            
            add_symbol(symbol);
            
            if (array_bst != NULL)
                array_bst->close();
            
            string symbolv[arrayc];
            
            for (size_t j = 0; j < arrayc; ++j)
                symbolv[j] = std::get<0>(* arrayv[j]);
            
            array_bst = build(symbolv, 0, (int)arrayc);
        } else {
            if (std::get<2>(* arrayv[i]).first)
                write_error(symbol);
            
            if (index > std::get<1>(* arrayv[i]).size())
                range_error(to_string(index));
            
            if (index == std::get<1>(* arrayv[i]).size())
                std::get<1>(* arrayv[i]).push(value);
            else
                std::get<1>(* arrayv[i])[index] = value;
        }
    }

    void interpreter::set_array(const string symbol, const string value, bool global) {
        if (!is_symbol(symbol))
            expect_error("symbol: " + symbol);
        
        string valuev[value.length() + 1];
        size_t valuec = parse(valuev, value);
        
        for (size_t i = 0; i < valuec; ++i) {
            if (valuev[i].empty())
                continue;
            
            if (is_string(valuev[i]))
                valuev[i] = encode(decode(valuev[i]));
            
            else if (is_number(valuev[i]))
                valuev[i] = rtrim(stod(valuev[i]));
            else
                throw error("Unexpected token: " + value);
        }
        
        std::function<void(void)> _set_array = [&]() {
            int i = io_array(symbol);
            
            if (i == -1) {
                if (is_defined(symbol))
                    defined_error(symbol);
                
                if (is_pow(arrayc, 2)) {
                    tuple<string, ss::array<string>, pair<bool, bool>>** _arrayv = new tuple<string, ss::array<string>, pair<bool, bool>>*[arrayc * 2];
                    
                    for (size_t j = 0; j < arrayc; ++j)
                        _arrayv[j] = arrayv[j];
                    
                    delete[] arrayv;
                    arrayv = _arrayv;
                }
                
                arrayv[arrayc] = new tuple<string, ss::array<string>, pair<bool, bool>>(symbol, ss::array<string>(), pair<bool, bool>(false, false));
                
                size_t j = arrayc++;
                while (j > 0 && std::get<0>(* arrayv[j]) < std::get<0>(* arrayv[j - 1])) {
                    swap(arrayv[j], arrayv[j - 1]);
                    --j;
                }
                
                std::get<1>(* arrayv[j]).resize(valuec);
                
                for (size_t k = 0; k < valuec; ++k)
                    std::get<1>(* arrayv[j])[k] = valuev[k];
                
                add_symbol(symbol);
                
                if (array_bst != NULL)
                    array_bst->close();
                
                string symbolv[arrayc];
                
                for (j = 0; j < arrayc; ++j)
                    symbolv[j] = std::get<0>(* arrayv[j]);
                
                array_bst = build(symbolv, 0, (int)arrayc);
            } else {
                if (std::get<2>(* arrayv[i]).first)
                    write_error(symbol);
                
                std::get<1>(* arrayv[i]).resize(valuec);
                
                for (size_t j = 0; j < valuec; ++j)
                    std::get<1>(* arrayv[i])[j] = valuev[j];
            }
        };
        
        if (global) {
            string buid = backup();
            
            for (size_t i = 0; i < backupc; ++i) {
                string _buid = bu_numberv[0];
                
                restore(_buid, false, false);
                
                _set_array();
                
                std::get<2>(* arrayv[io_array(symbol)]) = pair<bool, bool>(true, false);
                
                backup(_buid);
            }
            
            restore(buid, false, false);
        } else
            _set_array();
    }

    void interpreter::set_function(function_t* new_function) {
        if (is_defined(new_function->name()))
            defined_error(new_function->name());
        
        if (is_pow(functionc, 2)) {
            function_t** _functionv = new function_t*[functionc * 2];
            
            for (size_t i = 0; i < functionc; ++i)
                _functionv[i] = functionv[i];
            
            delete[] functionv;
            
            functionv = _functionv;
        }
        
        functionv[functionc] = new_function;
        
        add_symbol(functionv[functionc]->name());
        
        size_t i = functionc++;
        
        while (i > 0 && functionv[i]->name() < functionv[i - 1]->name()) {
            swap(functionv[i], functionv[i - 1]);
            --i;
        }
        
        if (function_bst != NULL)
            function_bst->close();
        
        string symbolv[functionc];
        
        for (size_t i = 0; i < functionc; ++i)
            symbolv[i] = functionv[i]->name();
        
        function_bst = build(symbolv, 0, (int)functionc);
    }

    void interpreter::set_listener(const string symbol, std::function<void(string)> callback) {
        assert(is_symbol(symbol));
        
        size_t i = 0;
        while (i < listenerv.size() && std::get<1>(listenerv[i]) != symbol)
            ++i;
        
        assert(i == listenerv.size());
        
        listenerv.push_back(tuple<size_t, string, std::function<void(string)>>(0, symbol, callback));
    }

    void interpreter::set_number(const string symbol, const double value, bool global) {
        if (global) {
            string buid = backup();
            
            for (size_t i = 0; i < backupc; ++i) {
                string _buid = bu_numberv[0];
                
                restore(_buid, false, false);
                
                arithmetic::set_number(symbol, value);
                
                set_read_only(symbol, true);
                backup(_buid);
            }
            
            restore(buid, false, false);
        } else
            arithmetic::set_number(symbol, value);
    }

    void interpreter::set_string(const string symbol, const string value, bool global) {
        if (!is_symbol(symbol))
            expect_error("symbol: " + symbol);
        
        if (!value.empty() && !is_string(value))
            throw error("Unexpected token: " + value);
        
        std::function<void(void)> _set_string = [&]() {
            int i = io_string(symbol);
            
            if (i == -1) {
                if (is_defined(symbol))
                    defined_error(symbol);
                
                if (is_pow(stringc, 2)) {
                    tuple<string, string, pair<bool, bool>>** tmp = new tuple<string, string, pair<bool, bool>>*[stringc * 2];
                    
                    for (size_t j = 0; j < stringc; ++j)
                        tmp[j] = stringv[j];
                    
                    delete[] stringv;
                    
                    stringv = tmp;
                }
                
                stringv[stringc] = new tuple<string, string, pair<bool, bool>>(symbol, value, pair<bool, bool>(false, false));
                
                size_t j = stringc++;
                while (j > 0 && std::get<0>(* stringv[j]) < std::get<0>(* stringv[j - 1])) {
                    swap(stringv[j], stringv[j - 1]);
                    --j;
                }
                
                add_symbol(symbol);
                
                if (string_bst != NULL)
                    string_bst->close();
                 
                string symbolv[stringc];
                
                for (size_t j = 0; j < stringc; ++j)
                    symbolv[j] = std::get<0>(* stringv[j]);
                
                string_bst = build(symbolv, 0, (int)stringc);
            } else {
                if (std::get<2>(* stringv[i]).first)
                    write_error(symbol);
                
                std::get<1>(* stringv[i]) = value;
            }
        };
        
        if (global) {
            string buid = backup();
            
            for (size_t i = 0; i < backupc; ++i) {
                string _buid = bu_numberv[0];
                
                restore(_buid, false, false);
                
                _set_string();
                
                std::get<2>(* stringv[io_string(symbol)]) = pair<bool, bool>(true, false);
                
                backup(_buid);
            }
            
            restore(buid, false, false);
        } else
            _set_string();
    }

    int interpreter::split(string* dst, string src) const {
        return merge((int)tokens(dst, src), dst);
    }

    void interpreter::stack_push(function_t* function) {
        assert(!stack.size());
        
        stack.push(function);
    }

    string interpreter::stack_trace() {
        ostringstream ss;
        
        ss << "  " << current_expression << endl;

        string padding = "    ";
        
        while (!stack.empty()) {
            ss << padding << stack.pop()->name() << "()\n";
            padding += "  ";
        }
        
        return ss.str();
    }

    void interpreter::type_error(const size_t lhs, const size_t rhs) {
        assert(lhs < 8 && rhs < 8);
        
        ss::type_error(types[lhs], types[rhs]);
    }
}
