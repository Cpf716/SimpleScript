//
//  command_processor.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.
//

#include "command_processor.h"

namespace ss {
    //  CONSTRUCTORS

    command_processor::command_processor() {
        initialize();
        
        set_number("SIGINT", SIGINT);
        
        logic::set_read_only("SIGINT", true);
        
        set_number("SIGTERM", SIGTERM);
        
        logic::set_read_only("SIGTERM", true);
        
        set_string("cwd", encode(filesystem::current_path()));
        
        std::get<2>(* stringv[io_string("cwd")]).first = true;
        
        set_number("nan", NAN);
        set_read_only("nan", true);
        
        set_string("null", null());
        
        std::get<2>(* stringv[io_string("null")]).first = true;
        
        set_array("types", 0, encode(to_string(array_t)));
        set_array("types", 1, encode(to_string(char_t)));
        set_array("types", 2, encode(to_string(dictionary_t)));
        set_array("types", 3, encode(to_string(number_t)));
        set_array("types", 4, encode(to_string(int_t)));
        set_array("types", 5, encode(to_string(string_t)));
        set_array("types", 6, encode(to_string(table_t)));
        
        int pos = io_array("types");
        
        std::get<1>(* arrayv[pos]).shrink_to_fit();
        std::get<2>(* arrayv[pos]).first = true; 
        
        save_state();
    }

    command_processor::~command_processor() {
        while (statec)
            set_state(state_numberv[statec - 1], false, false);
        
        delete[] state_arrayv;
        delete[] state_functionv;
        delete[] state_numberv;
        delete[] state_stringv;
        
        //  deallocate arrays
        for (size_t i = 0; i < arrayc; ++i)
            delete arrayv[i];
        
        delete[] arrayv;
        
        //  close functions
        for (size_t i = 0; i < functionc; ++i)
            functionv[i]->close();
        
        delete[] functionv;
        
        //  deallocate strings
        for (size_t i = 0; i < stringc; ++i)
            delete stringv[i];
        
        delete[] stringv;
        
        //  deallocate operators
        for (size_t i = 0; i < buocc; ++i)
            delete[] buov[i];
        
        delete[] buov;
        delete[] buocv;
        
        for (size_t i = 0; i < uoc; ++i)
            uov[i]->close();
        
        delete[] uov;
    }

    //  MEMBER FUNCTIONS

    string command_processor::_call(function_t* function, const size_t argc, string* argv) {
#if DEBUG_LEVEL == 2
        assert(function != NULL);
#endif
        this->stack.push(function);
        this->mutex.unlock();
        
        string result = function->call(argc, argv);

        this->mutex.lock();
        
        this->stack.pop();
        this->mutex.unlock();
        
        return result;
    }

    int command_processor::_get_state(const string key) const {
        int i;
        if ((i = io_function(key)) == -1) {
            if ((i = io_array(key)) == -1) {
                if ((i = io_string(key)) == -1)
                    return logic::_get_state(key);
                
                return (int)std::get<3>(* stringv[i]);
            }
            
            return (int)std::get<3>(* arrayv[i]);
        }
        
        return -1;
    }

    void command_processor::_set_array(const string key, const size_t valuec, string* valuev) {
        int i = io_array(key);
        
        if (i == -1) {
#if DEBUG_LEVEL
            if (is_defined(key))
                defined_error(key);
#endif
            if (is_pow(arrayc, 2)) {
                tuple<string, ss::array<string>, pair<bool, bool>, size_t>** _arrayv = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>*[arrayc * 2];
                
                for (size_t j = 0; j < arrayc; ++j)
                    _arrayv[j] = arrayv[j];
                
                delete[] arrayv;
                arrayv = _arrayv;
            }
            
            arrayv[arrayc] = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>(key, ss::array<string>(valuec), pair<bool, bool>(false, false), _statec);
            
            size_t j;
            for (j = arrayc++; j > 0 && std::get<0>(* arrayv[j]) < std::get<0>(* arrayv[j - 1]); --j)
                swap(arrayv[j], arrayv[j - 1]);
            
            std::get<1>(* arrayv[j]).resize(valuec);
            
            for (size_t k = 0; k < valuec; ++k)
                std::get<1>(* arrayv[j])[k] = valuev[k];
            
            add_key(key);
        } else {
            if (std::get<2>(* arrayv[i]).first)
                write_error(key);
            
            std::get<1>(* arrayv[i]).resize(valuec);
            
            for (size_t j = 0; j < valuec; ++j)
                std::get<1>(* arrayv[i])[j] = valuev[j];
        }
    }

    void command_processor::apply(const std::function<void(const bool)> cb) {
        size_t state = get_state();
        
        for (size_t i = 0; i < statec; ++i) {
            size_t _state = state_numberv[0];
            
            set_state(_state, false, false);
            
            cb(_state == this->state);
            
            get_state(_state);
        }
        
        set_state(state, false, false);
    }

    string command_processor::call(const string key, const size_t argc, string* argv) {
        this->mutex.lock();
        
        int pos = this->io_function(key);
        
        if (pos == -1) {
            this->mutex.unlock();
            
            undefined_error(key);
        }
        
#if DEBUG_LEVEL
        for (size_t i = 0; i < argc; ++i) {
            if (argv[i].empty())
                continue;
            
            if (is_string(argv[i]))
                argv[i] = encode(decode(argv[i]));
            
            else if (is_number(argv[i]))
                argv[i] = trim_end(parse_number(argv[i]));
            else
                throw error("Unexpected token: " + stringify(argc, argv));
        }
#endif
        return _call(this->functionv[pos], argc, argv);
    }

    void command_processor::consume(const string key) {
        int i;
        if ((i = io_function(key)) == -1) {
            if ((i = io_array(key)) == -1) {
                if ((i = io_string(key)) == -1)
                    logic::consume(key);
                else
                    std::get<2>(* stringv[i]).second = true;
            } else
                std::get<2>(* arrayv[i]).second = true;
        } else
            functionv[i]->consume();
    }

    string command_processor::element(const string val) {
        if (ss::is_array(val))
            type_error(array_t, item_t);
        
        if (val.empty())
            return null();
        
        if (is_string(val))
            return encode(decode(val));
        
        if (is_key(val)) {
            if (io_array(val) != -1)
                type_error(array_t, item_t);
            
            int i = io_string(val);
            if (i == -1)
                return encode(get_number(val));
            
            std::get<2>(* stringv[i]).second = true;
            
            return std::get<1>(* stringv[i]);
        }
        
        return encode(parse_number(val));
    }

    string command_processor::evaluate(const string expression) {
#if DEBUG_LEVEL == 3
        logger_write("(" + expression + "), ");
        
        time_point<steady_clock> beg = steady_clock::now();;
#endif
        this->expression = expression;
        
        string data[expression.length() * 2 + 1];
        int n = prefix(data, expression);
        
        /*
        for (size_t i = 0; i < n; ++i)
            cout << data[i] << "\t\t";
        
        cout << endl;
         // */
        
        ss::stack<string> operands = ss::stack<string>();
        
        for (int i = n - 1; i >= 0; --i) {
            if (data[i] == "(" || data[i] == ")" ||
                data[i] == uov[sequencer_pos]->opcode() ||
                data[i] == uov[conditional_pos + 1]->opcode())
                continue;
            
            if (data[i] == "...")
                operation_error();
            
            string term = tolower(data[i]);
            
            if (term == uov[const_pos]->opcode() ||
                term == uov[var_pos]->opcode())
                continue;
            
            size_t j = 0;
            while (j < arithmetic_pos && term != uov[j]->opcode())
                ++j;
            
            if (j == arithmetic_pos) {
                if (term == uov[additive_pos]->opcode())
                    j = additive_pos;
                else {
                    j = aggregate_pos;
                    while (j < bitwise_pos && term != uov[j]->opcode())
                        ++j;
                    
                    if (j == bitwise_pos) {
                        if (term == uov[additive_assignment_pos]->opcode())
                            j = additive_assignment_pos;
                        else {
                            j = direct_assignment_pos;
                            while (j < uoc - 1 && term != uov[j]->opcode())
                                ++j;
                        }
                    }
                }
            }
            
            if (j == uoc - 1) {
                j = 0;
                while (j < loc - 1 && term != lov[j]->opcode())
                    ++j;
                
                if (j == loc - 1) {
                    j = 0;
                    while (j < logic::additive_pos && term != aov[j]->opcode())
                        ++j;
                    
                    if (j == logic::additive_pos) {
                        ++j;
                        
                        while (j < logic::relational_pos && term != aov[j]->opcode())
                            ++j;
                        
                        if (j == logic::relational_pos) {
                            j = logic::bitwise_pos;
                            while (j < aoc - 2 && term != aov[j]->opcode())
                                ++j;
                        }
                    }
                    
                    if (j == aoc - 2) {
                        int j = io_function(data[i]);
                        if (j == -1) {
                            //  validate operand
                            
                            string tokenv[data[i].length() + 1];
                            
                            if (merge_numbers((int)tokens(tokenv, data[i], sizeof(SEPARATORS) / sizeof(SEPARATORS[0]), SEPARATORS), tokenv) == 1 && !is_string(tokenv[0]) &&
                                !is_key(tokenv[0]) && !is_number(tokenv[0]))
                                throw error("Unexpected token: " + data[i]);
                            
                            operands.push(data[i]);
                        } else {
                            if (functionv[j]->name() == to_string(array_t)) {
                                size_t k = i;
                                while (k > 0 && (data[k - 1] == uov[const_pos]->opcode() || data[k - 1] == uov[var_pos]->opcode()))
                                    --k;
                                
                                if (k > 0 && data[k - 1] == uov[direct_assignment_pos]->opcode())
                                    continue;
                            }
                            
                            if (i == n - 1 || data[i + 1] != "(")
                                expect_error("'(' in 'function' call: " + functionv[j]->name());
                            
                            size_t argc = 0;   int p = 1;   bool flag = false;
                            for (size_t k = i + 2; k < n; ++k) {
                                if (data[k] == "(")
                                    ++p;
                                
                                else if (data[k] == ")") {
                                    if (p == 1)
                                        break;
                                    
                                    --p;
                                } else if (p == 1 && data[k] == uov[sequencer_pos]->opcode())
                                    ++argc;
                                
                                else if (data[k] != "(" && data[k] != ")")
                                    flag = true;
                            }
                            
                            if (flag)
                                ++argc;
                            
                            string* argv = new string[pow2(argc)];
                            
                            size_t k = 0;
                            while (k < argc) {
                                // spread operator
                                if (operands.top().length() > 6) {
                                    size_t l = 0;
                                    while (l < 6 && operands.top()[l] == (l % 2 ? ' ' : '.'))
                                        ++l;
                                    
                                    if (l == 6) {
                                        string res = evaluate(operands.pop().substr(6));
                                        
                                        string valuev[res.length() + 1];
                                        size_t valuec = parse(valuev, res);
                                        
                                        argv[k] = valuev[0];
                                        
                                        size_t m;
                                        for (m = 1; m < valuec; ++m) {
                                            if (is_pow(argc, 2)) {
                                                string* tmp = new string[argc * 2];
                                                
                                                for (size_t n = 0; n < argc; ++n)
                                                    tmp[n] = argv[n];
                                                
                                                delete[] argv;
                                                
                                                argv = tmp;
                                            }
                                            
                                            argv[argc] = valuev[m];
                                            
                                            for (size_t n = argc; n > k + m; --n)
                                                swap(argv[n], argv[n - 1]);
                                            
                                            ++argc;
                                        }
                                        
                                        k += m;
                                    } else
                                        argv[k++] = evaluate(operands.pop());
                                } else
                                    argv[k++] = evaluate(operands.pop());
                            }
                            
                            this->mutex.lock();
                            
                            operands.push(this->_call(this->functionv[j], argc, argv));
                            
                            delete[] argv;
                        }
                    } else {
                        string rhs;
                        
                        string* dst = NULL;
                        size_t s;
                        
                        if (j >= logic::assignment_pos) {
                            string lhs = operands.pop();
                            
                            dst = new string[lhs.length() + 1];
                            s = parse(dst, lhs);
                            
                            if (s == 1) {
                                delete[] dst;
                                
                                if (!is_key(lhs))
                                    operation_error();
                                
                                int k = io_array(lhs);
                                if (k == -1) {
                                    if (io_string(lhs) != -1)
                                        type_error(string_t, number_t);
                                    
                                    size_t l = i + 1;
                                    while (l < n && data[l] == "(")
                                        ++l;
                                    
                                    if (data[l] == uov[indexer_pos]->opcode())
                                        operation_error();
                                    
                                    rhs = operands.pop();
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, number_t);
                                        //  array != double
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(string_t, number_t);
                                        //  string != double
                                        
                                    double b;
                                    
                                    if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = parse_number(rhs);
                                    
                                    double a = get_number(lhs);
                                    
                                    a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                    
                                    set_number(lhs, a);
                                    
                                    rhs = encode(a);
                                } else {
                                    size_t l = i + 1;
                                    while (l < n && data[l] == "(")
                                        ++l;
                                    
                                    if (data[l] != uov[indexer_pos]->opcode())
                                        operation_error();
                                    
                                    rhs = operands.pop();
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (rhs.empty()) {
                                        if (is_dictionary(std::get<1>(* arrayv[k])))
                                            null_error();
                                        
                                        type_error(string_t, int_t);
                                        //  string != int
                                    }
                                    
                                    if (is_string(rhs)) {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(array_t, dictionary_t);
                                            //  array != dictionary
                                        
                                        rhs = decode(rhs);
                                        
                                        if (rhs.empty())
                                            undefined_error(encode(null()));
                                        
                                        rhs = encode(rhs);
                                        
                                        size_t l = 0;
                                        while (l < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[l * 2] != rhs)
                                            ++l;
                                        
                                        if (l == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                            undefined_error(rhs);
                                        
                                        if (std::get<1>(* arrayv[k])[l * 2 + 1].empty() || is_string(std::get<1>(* arrayv[k])[l * 2 + 1]))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        dst = new string[rhs.length() + 1];
                                        s = parse(dst, rhs);
                                        delete[] dst;
                                        
                                        if (s != 1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (is_string(rhs))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        double b;
                                        if (is_key(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = parse_number(rhs);
                                        
                                        double a = parse_number(std::get<1>(* arrayv[k])[l * 2 + 1]);
                                        
                                        a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                        
                                        rhs = encode(a);
                                        
                                        std::get<1>(* arrayv[k])[l * 2 + 1] = rhs;
                                        
                                    } else if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, int_t);
                                            //  array != int
                                        
                                        int l = io_string(rhs);
                                        if (l == -1) {
                                            double idx = get_number(rhs);
                                            
                                            if (!is_int(idx))
                                                type_error(number_t, int_t);
                                                //  double != int
                                            
                                            if (idx < 0 || idx >= std::get<1>(* arrayv[k]).size())
                                                range_error("index " + encode(idx) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                            
                                            if (std::get<1>(* arrayv[k])[(size_t)idx].empty() || is_string(std::get<1>(* arrayv[k])[(size_t)idx]))
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            rhs = operands.top();
                                            
                                            if (rhs.empty())
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            dst = new string[rhs.length() + 1];
                                            s = parse(dst, rhs);
                                            delete[] dst;
                                            
                                            if (s != 1)
                                                type_error(array_t, number_t);
                                                //  array !=  double
                                            
                                            if (is_string(rhs))
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            double b;
                                            if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                
                                                b = get_number(rhs);
                                            } else
                                                b = parse_number(rhs);
                                            
                                            double a = parse_number(std::get<1>(* arrayv[k])[(size_t)idx]);
                                            
                                            a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                            
                                            rhs = encode(a);
                                            
                                            std::get<1>(* arrayv[k])[(size_t)idx] = rhs;
                                        } else {
                                            if (!is_dictionary(std::get<1>(* arrayv[k])))
                                                type_error(array_t, dictionary_t);
                                                //  array != dictionary
                                            
                                            rhs = std::get<1>(* stringv[l]);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (rhs.length() == 2)
                                                undefined_error(rhs);
                                            
                                            size_t m = 0;
                                            while (m < (size_t)floor(std::get<1>(* arrayv[k]).size() / 2) && std::get<1>(* arrayv[k])[m * 2] != rhs)
                                                ++m;
                                            
                                            if (m == (size_t)floor(std::get<1>(* arrayv[k]).size() / 2))
                                                undefined_error(rhs);
                                            
                                            if (std::get<1>(* arrayv[k])[m * 2 + 1].empty() || is_string(std::get<1>(* arrayv[k])[m * 2 + 1]))
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            rhs = operands.top();
                                            
                                            if (rhs.empty())
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            dst = new string[rhs.length() + 1];
                                            s = parse(dst, rhs);
                                            delete[] dst;
                                            
                                            if (s != 1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (is_string(rhs))
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            double b;
                                            if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                
                                                b = get_number(rhs);
                                            } else
                                                b = parse_number(rhs);
                                            
                                            double a = parse_number(std::get<1>(* arrayv[k])[m * 2 + 1]);
                                            
                                            a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                            
                                            rhs = encode(a);
                                            
                                            std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        }
                                    } else {
                                        double idx = parse_number(rhs);
                                        
                                        if (!is_int(idx))
                                            type_error(number_t, int_t);
                                            //  double != int
                                        
                                        if (idx < 0 || idx >= std::get<1>(* arrayv[k]).size())
                                            range_error("index " + encode(idx) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                        
                                        if (std::get<1>(* arrayv[k])[(size_t)idx].empty() || is_string(std::get<1>(* arrayv[k])[(size_t)idx]))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        dst = new string[rhs.length() + 1];
                                        s = parse(dst, rhs);
                                        delete[] dst;
                                        
                                        if (s != 1)
                                            type_error(array_t, number_t);
                                            //  array !=  double
                                        
                                        if (is_string(rhs))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        double b;
                                        if (is_key(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = parse_number(rhs);
                                        
                                        double a = parse_number(std::get<1>(* arrayv[k])[(size_t)idx]);
                                        
                                        a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                        
                                        rhs = encode(a);
                                        
                                        std::get<1>(* arrayv[k])[(size_t)idx] = rhs;
                                    }
                                }
                            } else {
                                size_t k = i + 1;
                                while (k < n && data[k] == "(")
                                    ++k;
                                
                                if (data[k] != uov[indexer_pos]->opcode()) {
                                    delete[] dst;
                                    operation_error();
                                }
                                
                                rhs = operands.pop();
                                
                                if (rhs.empty()) {
                                    delete[] dst;
                                    
                                    if (is_dictionary(s, dst))
                                        null_error();
                                    
                                    type_error(string_t, int_t);
                                    //  string != int
                                }
                                
                                string* _dst = new string[rhs.length() + 1];
                                size_t _s = parse(_dst, rhs);
                                
                                if (_s != 1) {
                                    delete[] dst;
                                    type_error(array_t, int_t);
                                    //  array != int
                                }
                                
                                if (is_string(rhs)) {
                                    if (!is_dictionary(s, dst)) {
                                        delete[] dst;
                                        type_error(array_t, dictionary_t);
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
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    _dst = new string[rhs.length() + 1];
                                    _s = parse(_dst, rhs);
                                    delete[] _dst;
                                    
                                    if (_s != 1)
                                        type_error(array_t, number_t);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    double b;
                                    if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = parse_number(rhs);
                                    
                                    double a = parse_number(dst[k * 2 + 1]);
                                        
                                    a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                    
                                    dst[k * 2 + 1] = encode(a);
                                    
                                    rhs = stringify(s, dst);
                                    
                                } else if (is_key(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    int k = io_string(rhs);
                                    if (k == -1) {
                                        double idx = get_number(rhs);
                                        
                                        if (!is_int(idx)) {
                                            delete[] dst;
                                            type_error(number_t, int_t);
                                            //  double != int
                                        }
                                        
                                        if (idx < 0 || idx >= s)
                                            range_error("index " + encode(idx) + ", count " + std::to_string(s));
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        _dst = new string[rhs.length() + 1];
                                        _s = parse(_dst, rhs);
                                        delete[] _dst;
                                        
                                        if (_s != 1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (is_string(rhs))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        double b;
                                        if (is_key(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = parse_number(rhs);
                                        
                                        double a = parse_number(dst[(size_t)idx]);
                                            
                                        a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                        
                                        dst[(size_t)idx] = encode(a);
                                        
                                        rhs = stringify(s, dst);
                                    } else {
                                        if (!is_dictionary(s, dst)) {
                                            delete[] dst;
                                            type_error(array_t, dictionary_t);
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
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        rhs = operands.top();
                                        
                                        if (rhs.empty())
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        _dst = new string[rhs.length() + 1];
                                        _s = parse(_dst, rhs);
                                        delete[] _dst;
                                        
                                        if (_s != 1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (is_string(rhs))
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        double b;
                                        if (is_key(rhs)) {
                                            if (io_array(rhs) != -1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (io_string(rhs) != -1)
                                                type_error(string_t, number_t);
                                                //  string != double
                                            
                                            b = get_number(rhs);
                                        } else
                                            b = parse_number(rhs);
                                        
                                        double a = parse_number(dst[l * 2 + 1]);
                                            
                                        a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                        
                                        dst[l * 2 + 1] = encode(a);
                                        
                                        rhs = stringify(s, dst);
                                    }
                                } else {
                                    double idx = parse_number(rhs);
                                    
                                    if (!is_int(idx)) {
                                        delete[] dst;
                                        type_error(number_t, int_t);
                                        //  double != int
                                    }
                                    
                                    if (idx < 0 || idx >= s)
                                        range_error("index " + encode(idx) + ", count " + std::to_string(s));
                                    
                                    rhs = operands.top();
                                    
                                    if (rhs.empty())
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    _dst = new string[rhs.length() + 1];
                                    _s = parse(_dst, rhs);
                                    delete[] _dst;
                                    
                                    if (_s != 1)
                                        type_error(array_t, number_t);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    double b;
                                    if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        b = get_number(rhs);
                                    } else
                                        b = parse_number(rhs);
                                    
                                    double a = parse_number(dst[(size_t)idx]);
                                        
                                    a = ((binary_arithmetic_operator *)aov[j])->apply(a, b);
                                    
                                    dst[(size_t)idx] = encode(a);
                                    
                                    rhs = stringify(s, dst);
                                }
                            }
                        } else {
                            rhs = operands.pop();
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, number_t);
                                //  array != double
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, number_t);
                                //  string != double
                            
                            double a;
                            if (is_key(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(array_t, number_t);
                                    //  array != double
                                
                                if (io_string(rhs) != -1)
                                    type_error(string_t, number_t);
                                    //  string != double
                                
                                a = get_number(rhs);
                            } else
                                a = parse_number(rhs);
                            
                            if (j < logic::unary_count)
                                a = ((unary_arithmetic_operator *)aov[j])->apply(a);
                            else {
                                rhs = operands.pop();
                                
                                if (ss::is_array(rhs))
                                    type_error(array_t, number_t);
                                    //  array != double
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(string_t, number_t);
                                    //  string != double
                                
                                double b;
                                if (is_key(rhs)) {
                                    if (io_array(rhs) != -1)
                                        type_error(array_t, number_t);
                                        //  array != double
                                    
                                    if (io_string(rhs) != -1)
                                        type_error(string_t, number_t);
                                        //  string != douoble
                                    
                                    b = get_number(rhs);
                                } else
                                    b = parse_number(rhs);
                                
                                a = ((binary_arithmetic_operator_t *)aov[j])->apply(a, b);
                            }
                            
                            rhs = encode(a);
                        }
                        
                        operands.push(rhs);
                    }
                } else {
                    string rhs;
                    
                    if (j == 0) {
                        rhs = operands.top();
                        
                        double num;
                        
                        if (ss::is_array(rhs))
                            num = 1;
                        else if (rhs.empty())
                            num = 0;
                        else if (is_string(rhs))
                            num = !decode_raw(rhs).empty();
                        else if (is_key(rhs)) {
                            int k = io_array(rhs);
                            
                            if (k == -1) {
                                k = io_string(rhs);
                                
                                if (k == -1)
                                    num = get_number(rhs);
                                else {
                                    num = rhs.empty() ? 0 : !decode_raw(std::get<1>(* stringv[k])).empty();
                                    
                                    std::get<2>(* stringv[k]).second = true;
                                }
                            } else {
                                if (std::get<1>(* arrayv[k]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[k])[0];
                                    num = rhs.empty() ? 0 : is_string(rhs) ? !decode_raw(rhs).empty() : parse_number(rhs);
                                } else
                                    num = 1;
                                
                                std::get<2>(* arrayv[k]).second = true;
                            }
                        } else
                            num = parse_number(rhs);
                        
                        rhs = std::to_string(((unary_logical_operator *)lov[j])->apply(std::to_string(num)));
                    } else {
                        rhs = evaluate(operands.pop());
                        
                        if (ss::evaluate(rhs)) {
                            if (j == 1)
                                rhs = std::to_string(ss::evaluate(evaluate(operands.top())));
                        } else
                            rhs = j == 1 ? std::to_string(0) : evaluate(operands.top());
                    }
                    
                    operands.pop();
                    operands.push(rhs);
                }
            } else {
                string rhs;
                
                if (j < unary_count) {
                    rhs = ((unary_universal_operator *)uov[j])->apply(operands.pop());
                    
                    operands.push(rhs);
                } else {
                    string lhs = operands.pop();
                    
                    bool flag;
                    
                    if (j == indexer_pos) {
                        if (i == 1)
                            flag = false;
                        else {
                            size_t k = i - 1;
                            while (k > 0 && data[k] == "(")
                                --k;
                            
                            if (data[k] == "(")
                                flag = false;
                            
                            else if (data[k] == uov[additive_assignment_pos]->opcode() || data[k] == uov[direct_assignment_pos]->opcode())
                                flag = true;
                            else {
                                size_t l = logic::assignment_pos;
                                while (l < aoc - 2 && data[k] != aov[l]->opcode())
                                    ++l;
                                
                                flag = l != aoc - 2;
                            }
                        }
                    } else
                        flag = false;
                    
                    if (flag)
                        rhs = lhs;
                    else if (j == aggregate_pos ||
                             j == filter_pos ||
                             j == find_pos ||
                             j == find_index_pos ||
                             j == map_pos) {
                        string ctr = operands.pop();
                        
                        rhs = operands.pop();
                        rhs = ((ternary_universal_operator *)uov[j])->apply(lhs, ctr, rhs);
                        
                    } else if (j == cell_pos ||
                               j == insert_pos) {
                        string ctr = evaluate(operands.pop());
                        
                        rhs = evaluate(operands.pop());
                        rhs = ((ternary_universal_operator *)uov[j])->apply(lhs, ctr, rhs);
                        
                    } else if (j == slice_pos) {
                        string valuev[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                string text = decode(lhs);
                                
                                rhs = evaluate(operands.pop());
                                
                                if (ss::is_array(rhs))
                                    type_error(array_t, int_t);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(string_t, int_t);
                                    //  string != int
                                
                                double s = stod(rhs);
                                
                                if (!is_int(s))
                                    type_error(number_t, int_t);
                                    //  double != int
                                
                                if (s < 0 || s > text.length())
                                    range_error("start " + encode(s) + ", count " + std::to_string(text.length()));
                                
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
                                
                                if (data[k] == uov[sequencer_pos]->opcode()) {
                                    rhs = evaluate(operands.pop());
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(string_t, int_t);
                                        //  string != int
                                    
                                    double l = stod(rhs);
                                    
                                    if (!is_int(l))
                                        type_error(number_t, int_t);
                                        //  double != int
                                    
                                    if (l < 0 || s + l > text.length())
                                        range_error("start " + encode(s) + ", length " + encode(l) + ", count " + std::to_string(text.length()));
                                    
                                    rhs = text.substr((size_t)s, (size_t)l);
                                } else
                                    rhs = text.substr((size_t)s);
                                
                                rhs = encode(rhs);
                            } else {
                                if (!is_key(lhs))
                                    type_error(number_t, array_t);
                                    //  double != array
                                
                                int k = io_array(lhs);
                                if (k == -1) {
                                    k = io_string(lhs);
                                    if (k == -1) {
                                        if (is_defined(lhs))
                                            type_error(number_t, array_t);
                                            //  double != array
                                        
                                        undefined_error(lhs);
                                    }
                                    
                                    string text = std::get<1>(* stringv[k]);
                                    
                                    if (text.empty())
                                        null_error();
                                    
                                    text = decode_raw(text);
                                    
                                    rhs = evaluate(operands.pop());
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(string_t, int_t);
                                        //  string != int
                                    
                                    double s = stod(rhs);
                                    
                                    if (!is_int(s))
                                        type_error(number_t, int_t);
                                        //  double != int
                                    
                                    if (s < 0 || s > text.length())
                                        range_error("start " + encode(s) + ", count " + std::to_string(text.length()));
                                    
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
                                    
                                    if (data[l] == uov[sequencer_pos]->opcode()) {
                                        rhs = evaluate(operands.pop());
                                        
                                        if (ss::is_array(rhs))
                                            type_error(array_t, int_t);
                                            //  array != int
                                        
                                        if (rhs.empty() || is_string(rhs))
                                            type_error(string_t, int_t);
                                            //  string != int
                                        
                                        double l = stod(rhs);
                                        
                                        if (!is_int(l))
                                            type_error(number_t, int_t);
                                            //  double != int
                                        
                                        if (l < 0 || s + l > text.length())
                                            range_error("start " + encode(s) + ", length " + encode(l) + ", count " + std::to_string(text.length()));
                                        
                                        rhs = text.substr((size_t)s, (size_t)l);
                                    } else
                                        rhs = text.substr((size_t)s);
                                    
                                    rhs = encode(rhs);
                                    
                                    std::get<2>(* stringv[io_string(lhs)]).second = true;
                                } else {
                                    array<string> arr = std::get<1>(* arrayv[k]);
                                    
                                    rhs = evaluate(operands.pop());
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(string_t, int_t);
                                        //  string != int
                                    
                                    double s = stod(rhs);
                                    
                                    if (!is_int(s))
                                        type_error(number_t, int_t);
                                        //  double != int
                                    
                                    if (s > arr.size())
                                        range_error("start " + encode(s) + ", count " + std::to_string(arr.size()));
                                    
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
                                    
                                    if (data[l] == uov[sequencer_pos]->opcode()) {
                                        rhs = evaluate(operands.pop());
                                        
                                        if (ss::is_array(rhs))
                                            type_error(array_t, int_t);
                                            //  array != int
                                        
                                        if (rhs.empty() || is_string(rhs))
                                            type_error(string_t, int_t);
                                            //  string != int
                                        
                                        double e = stod(rhs);
                                        
                                        if (!is_int(e))
                                            type_error(number_t, int_t);
                                            //  double != int
                                        
                                        if (s + e > arr.size())
                                            range_error("start " + encode(s) + ", end " + encode(e) + ", count " + std::to_string(arr.size()));
                                        
                                        rhs = stringify(arr, (size_t)s, (size_t)e);
                                    } else
                                        rhs = stringify(arr, (size_t)s);
                                    
                                    std::get<2>(* arrayv[io_array(lhs)]).second = true;
                                }
                            }
                        } else {
                            rhs = evaluate(operands.pop());
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, int_t);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, int_t);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(number_t, int_t);
                                //  double != int
                            
                            if (s > valuec)
                                range_error("start " + encode(s) + ", count " + std::to_string(valuec));
                            
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

                            if (data[k] == uov[sequencer_pos]->opcode()) {
                                rhs = evaluate(operands.pop());
                                
                                if (ss::is_array(rhs))
                                    type_error(array_t, int_t);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(string_t, int_t);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(number_t, int_t);
                                    //  double != int
                                
                                if (s + e > valuec)
                                    range_error("start " + encode(s) + ", end " + encode(e) + ", count " + std::to_string(valuec));
                                
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
                    } else if (j == splice_pos) {
                        string* valuev = new string[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            if (lhs.empty() || is_string(lhs))
                                type_error(string_t, array_t);
                            
                            if (!is_key(lhs))
                                type_error(number_t, array_t);
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                if (io_string(lhs) != -1)
                                    type_error(string_t, array_t);
                                    //  string != array
                                    
                                if (is_defined(lhs))
                                    type_error(number_t, array_t);
                                    //  double != array
                                
                                undefined_error(lhs);
                            }
                            
                            rhs = evaluate(operands.pop());
                            
                            k = io_array(lhs);
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, item_t);
                            
                            if (rhs.empty()) {
                                if (is_dictionary(std::get<1>(* arrayv[k])))
                                    null_error();
                                
                                type_error(string_t, int_t);
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
                                
                                if (data[l] == uov[sequencer_pos]->opcode())
                                    operation_error();
                                
                                if (!is_dictionary(std::get<1>(* arrayv[k])))
                                    type_error(string_t, int_t);
                                    //  string != int
                                
                                rhs = decode_raw(rhs);
                                
                                if (rhs.empty())
                                    undefined_error(encode(null()));
                                
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
                                    std::get<1>(* arrayv[k]).push(null());
                                    
                                rhs = stringify(valuec, valuev);
                                
                                delete[] valuev;
                            } else {
                                double s = stod(rhs);
                                
                                if (!is_int(s))
                                    type_error(number_t, int_t);
                                
                                if (s >= std::get<1>(* arrayv[k]).size())
                                    range_error("start " + encode(s) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                
                                size_t m = i + 1;   int q = 1;
                                do {
                                    if (data[m] == "(")
                                        ++q;
                                    else if (data[m] == ")")
                                        --q;
                                    
                                    ++m;
                                    
                                    if (!q)
                                        break;
                                } while (m < n);
                                
                                if (data[m] == uov[sequencer_pos]->opcode()) {
                                    rhs = evaluate(operands.pop());
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                    
                                    if (rhs.empty() || is_string(rhs))
                                        type_error(string_t, int_t);
                                    
                                    double l = stod(rhs);
                                    
                                    if (!is_int(l))
                                        type_error(number_t, int_t);
                                    
                                    k = io_array(lhs);
                                    
                                    if (l < 0 || s + l > std::get<1>(* arrayv[k]).size())
                                        range_error("start " + encode(s) + ", length " + encode(l) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                    
                                    valuec = (size_t)l;
                                    valuev = new string[valuec];
                                } else {
                                    valuec = std::get<1>(* arrayv[k]).size() - (size_t)s;
                                    valuev = new string[valuec];
                                }
                                
                                for (m = 0; m < valuec; ++m) {
                                    valuev[m] = std::get<1>(* arrayv[k])[(size_t)s];
                                    
                                    std::get<1>(* arrayv[k]).remove((size_t)s);
                                }
                                
                                rhs = stringify(valuec, valuev);
                                
                                delete[] valuev;
                            }
                        } else {
                            rhs = evaluate(operands.pop());
                            
                            if (ss::is_array(rhs)) {
                                delete[] valuev;
                                type_error(array_t, item_t);
                            }
                            
                            if (rhs.empty()) {
                                delete[] valuev;
                                
                                if (is_dictionary(valuec, valuev))
                                    null_error();
                                
                                type_error(string_t, int_t);
                            }
                            
                            if (is_string(rhs)) {
                                if (!is_dictionary(valuec, valuev)) {
                                    delete[] valuev;
                                    
                                    type_error(string_t, int_t);
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
                                
                                if (data[k] == uov[sequencer_pos]->opcode()) {
                                    delete[] valuev;
                                    operation_error();
                                }
                                
                                rhs = decode_raw(rhs);
                                
                                if (rhs.empty()) {
                                    delete[] valuev;
                                    undefined_error(encode(null()));
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
                                    type_error(number_t, int_t);
                                }
                                
                                if (s >= valuec) {
                                    delete[] valuev;
                                    range_error("start " + encode(s) + ", count " + std::to_string(valuec));
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
                                
                                if (data[k] == uov[sequencer_pos]->opcode()) {
                                    rhs = evaluate(operands.pop());
                                    
                                    if (ss::is_array(rhs)) {
                                        delete[] valuev;
                                        type_error(array_t, int_t);
                                    }
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] valuev;
                                        type_error(string_t, int_t);
                                    }
                                    
                                    double l = stod(rhs);
                                    
                                    if (!is_int(l)) {
                                        delete[] valuev;
                                        type_error(number_t, int_t);
                                    }
                                    
                                    if (l < 0 || s + l > valuec) {
                                        delete[] valuev;
                                        
                                        range_error("start " + encode(s) + ", length " + encode(l) + ", count " + std::to_string(valuec));
                                    }
                                    
                                    for (k = 0; k < l; ++k) {
                                        for (size_t m = s; m < valuec - 1; ++m)
                                            swap(valuev[m], valuev[m + 1]);
                                        
                                        --valuec;
                                    }
                                } else {
                                    for (k = 0; k < valuec; ++k) {
                                        for (size_t m = s; m < valuec - 1; ++m)
                                            swap(valuev[m], valuev[m + 1]);
                                        
                                        --valuec;
                                    }
                                }
                            }
                            
                            rhs = stringify(valuec, valuev);
                            
                            delete[] valuev;
                        }
                    } else if (j == substr_pos) {
                        if (ss::is_array(lhs))
                            type_error(array_t, string_t);
                            //  array != string
                        
                        if (lhs.empty())
                            null_error();
                        
                        if (!is_string(lhs)) {
                            if (!is_key(lhs))
                                type_error(number_t, string_t);
                                //  double != string
                            
                            if (io_array(lhs) != -1)
                                type_error(array_t, string_t);
                                //  array != string
                            
                            int k = io_string(lhs);
                            if (k == -1) {
                                if (is_defined(lhs))
                                    type_error(number_t, string_t);
                                    //  double != string
                                
                                undefined_error(lhs);
                            }
                            
                            lhs = std::get<1>(* stringv[k]);
                            
                            if (lhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[k]).second = true;
                        }
                        
                        lhs = decode(lhs);
                        rhs = evaluate(operands.pop());
                        
                        if (ss::is_array(rhs))
                            type_error(array_t, int_t);
                            //  array != int
                        
                        if (rhs.empty() || is_string(rhs))
                            type_error(string_t, int_t);
                            //  string != int
                        
                        double s = stod(rhs);
                        
                        if (!is_int(s))
                            type_error(number_t, int_t);
                            //  double != int
                        
                        if (s < 0 || s > lhs.length())
                            range_error("start " + encode(s) + ", count " + std::to_string(lhs.length()));
                        
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
                        
                        if (data[k] == uov[sequencer_pos]->opcode()) {
                            rhs = evaluate(operands.pop());
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, int_t);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, int_t);
                                //  string != int
                                
                            double e = stod(rhs);
                            
                            if (!is_int(e))
                                type_error(number_t, int_t);
                                //  double != int
                            
                            if (e < s || e > lhs.length())
                                range_error("start " + encode(s) + ", end " + encode(e) + ", count " + std::to_string(lhs.length()));
                            
                            rhs = lhs.substr((size_t)s, (size_t)(e - s));
                        } else
                            rhs = lhs.substr((size_t)s);
                        
                        rhs = encode(rhs);
                        
                    } else if (j == tospliced_pos) {
                        string* valuev = new string[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            delete[] valuev;
                            
                            if (lhs.empty() || is_string(lhs))
                                type_error(string_t, array_t);
                            
                            if (!is_key(lhs))
                                type_error(number_t, array_t);
                            
                            int k = io_array(lhs);
                            if (k == -1) {
                                if (io_string(lhs) != -1)
                                    type_error(string_t, array_t);
                                
                                if (is_defined(lhs))
                                    type_error(number_t, array_t);
                                
                                undefined_error(lhs);
                            }
                            
                            rhs = evaluate(operands.pop());
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, int_t);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, int_t);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(number_t, int_t);
                                //  double != int
                            
                            k = io_array(lhs);
                            
                            if (s < 0 || s > std::get<1>(* arrayv[k]).size())
                                range_error("start " + encode(s) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                            
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
                            
                            if (data[l] == uov[sequencer_pos]->opcode()) {
                                rhs = evaluate(operands.pop());
                                
                                if (ss::is_array(rhs))
                                    type_error(array_t, int_t);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(string_t, int_t);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(number_t, int_t);
                                    //  double != int
                                
                                k = io_array(lhs);
                                
                                if (e < 0 || s + e > std::get<1>(* arrayv[k]).size())
                                    range_error("start " + encode(s) + " length " + encode(e) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                
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
                            rhs = evaluate(operands.pop());
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, int_t);
                                //  array != int
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, int_t);
                                //  string != int
                            
                            double s = stod(rhs);
                            
                            if (!is_int(s))
                                type_error(number_t, int_t);
                                //  double != int
                            
                            if (s < 0 || s > valuec)
                                range_error("start " + encode(s) + ", count " + std::to_string(valuec));
                            
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
                            
                            if (data[k] == uov[sequencer_pos]->opcode()) {
                                rhs = evaluate(operands.pop());
                                
                                if (ss::is_array(rhs))
                                    type_error(array_t, int_t);
                                    //  array != int
                                
                                if (rhs.empty() || is_string(rhs))
                                    type_error(string_t, int_t);
                                    //  string != int
                                
                                double e = stod(rhs);
                                
                                if (!is_int(e))
                                    type_error(number_t, int_t);
                                    //  double != int
                                
                                if (e < 0 || s + e > valuec)
                                    range_error("start " + encode(s) + ", end " + encode(e) + ", count " + std::to_string(valuec));

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
                    } else if (j == additive_assignment_pos) {
                        string* v = new string[lhs.length() + 1];
                        size_t p = parse(v, lhs);
                        
                        if (p == 1) {
                            delete[] v;
                            
                            if (!is_key(lhs))
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
                                    
                                    if (data[l] == uov[indexer_pos]->opcode())
                                        operation_error();
                                    
                                    if (rhs.empty())
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    v = new string[rhs.length() + 1];
                                    p = parse(v, rhs);
                                    delete[] v;
                                    
                                    if (p != 1)
                                        type_error(array_t, number_t);
                                        //  array != double
                                    
                                    if (is_string(rhs))
                                        type_error(string_t, number_t);
                                        //  string != double
                                    
                                    if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, number_t);
                                            //  array != double
                                        
                                        if (io_string(rhs) != -1)
                                            type_error(string_t, number_t);
                                            //  string != double
                                        
                                        d += get_number(rhs);
                                    } else
                                        d += parse_number(rhs);
                                    
                                    set_number(lhs, d);
                                    
                                    rhs = encode(d);
                                } else {
                                    size_t l = i + 1;
                                    while (l < n && data[l] == "(")
                                        ++l;
                                    
                                    if (data[l] == uov[indexer_pos]->opcode())
                                        operation_error();
                                    
                                    string tmp = rhs;
                                    
                                    rhs = std::get<1>(* stringv[k]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[k]).second = true;
                                    
                                    rhs = decode_raw(rhs);
                                    
                                    swap(rhs, tmp);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    v = new string[rhs.length() + 1];
                                    p = parse(v, rhs);
                                    delete[] v;
                                    
                                    if (p != 1)
                                        type_error(array_t, string_t);
                                        //  array != string
                                    
                                    if (is_string(rhs))
                                        rhs = decode(rhs);
                                    
                                    else if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, string_t);
                                            //  array != string
                                        
                                        int j = io_string(rhs);
                                        if (j == -1)
                                            rhs = encode(get_number(rhs));
                                        else {
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            std::get<2>(* stringv[j]).second = true;
                                            
                                            rhs = decode_raw(rhs);
                                        }
                                    } else
                                        rhs = encode(parse_number(rhs));
                                    
                                    rhs = tmp + rhs;
                                    rhs = encode(rhs);
                                    
                                    set_string(lhs, rhs);
                                }
                            } else {
                                size_t l = i + 1;
                                while (l < n && data[l] == "(")
                                    ++l;
                                
                                if (data[l] == uov[indexer_pos]->opcode()) {
                                    if (rhs.empty()) {
                                        if (is_dictionary(std::get<1>(* arrayv[k])))
                                            null_error();
                                        
                                        type_error(string_t, int_t);
                                        //  string != int
                                    }
                                    
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (is_string(rhs)) {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(string_t, int_t);
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
                                                type_error(array_t, string_t);
                                                //  array != string
                                            
                                            lhs = decode_raw(lhs);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (is_string(rhs))
                                                rhs = decode(rhs);
                                            
                                            else if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, string_t);
                                                    //  array != string
                                                
                                                int q = io_string(rhs);
                                                if (q == -1)
                                                    rhs = encode(get_number(rhs));
                                                else {
                                                    rhs = std::get<1>(* stringv[q]);
                                                    
                                                    if (rhs.empty())
                                                        null_error();
                                                    
                                                    std::get<2>(* stringv[q]).second = true;
                                                    
                                                    rhs = decode_raw(rhs);
                                                }
                                            } else
                                                rhs = encode(parse_number(rhs));
                                            
                                            rhs = lhs + rhs;
                                            rhs = encode(rhs);
                                            
                                            std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        } else {
                                            double d = parse_number(lhs);
                                            
                                            if (p != 1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (is_string(rhs))
                                                type_error(string_t, number_t);
                                                //  string != double
                                                
                                            if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                
                                                d += get_number(rhs);
                                            } else
                                                d += parse_number(rhs);
                                            
                                            rhs = encode(d);
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            std::get<1>(* arrayv[k])[m * 2 + 1] = rhs;
                                        }
                                    } else if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, int_t);
                                            //  array != int
                                        
                                        int m = io_string(rhs);
                                        if (m == -1) {
                                            double index = get_number(rhs);
                                            
                                            if (!is_int(index))
                                                type_error(number_t, int_t);
                                                //  double != int
                                            
                                            if (index < 0 || index >= std::get<1>(* arrayv[k]).size())
                                                range_error("index " + encode(index) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                            
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
                                                    type_error(array_t, string_t);
                                                    //  array != string
                                                
                                                lhs = decode_raw(lhs);
                                                
                                                if (rhs.empty())
                                                    null_error();
                                                
                                                if (is_string(rhs))
                                                    rhs = decode(rhs);
                                                
                                                else if (is_key(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(array_t, string_t);
                                                        //  array != string
                                                    
                                                    l = io_string(rhs);
                                                    if (l == -1)
                                                        rhs = encode(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[l]);
                                                        
                                                        if (rhs.empty())
                                                            null_error();
                                                        
                                                        std::get<2>(* stringv[l]).second = true;
                                                        
                                                        rhs = decode_raw(rhs);
                                                    }
                                                } else
                                                    rhs = encode(parse_number(rhs));
                                                
                                                rhs = lhs + rhs;
                                                rhs = encode(rhs);
                                                
                                                std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                            } else {
                                                double d = parse_number(std::get<1>(* arrayv[k])[(size_t)index]);
                                                
                                                if (p != 1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (is_string(rhs))
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                    
                                                if (is_key(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(array_t, number_t);
                                                        //  array != double
                                                    
                                                    if (io_string(rhs) != -1)
                                                        type_error(string_t, number_t);
                                                        //  string != double
                                                    
                                                    d += get_number(rhs);
                                                } else
                                                    d += parse_number(rhs);
                                                
                                                rhs = encode(d);
                                                
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                            }
                                        } else {
                                            if (!is_dictionary(std::get<1>(* arrayv[k])))
                                                type_error(string_t, int_t);
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
                                                    type_error(array_t, string_t);
                                                    //  array != string
                                                
                                                lhs = decode_raw(lhs);
                                                
                                                if (rhs.empty())
                                                    null_error();
                                                
                                                if (is_string(rhs))
                                                    rhs = decode(rhs);
                                                
                                                else if (is_key(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(array_t, string_t);
                                                        //  array != string
                                                    
                                                    size_t r = io_string(rhs);
                                                    if (r == -1)
                                                        rhs = encode(get_number(rhs));
                                                    else {
                                                        rhs = std::get<1>(* stringv[r]);
                                                        
                                                        if (rhs.empty())
                                                            null_error();
                                                        
                                                        std::get<2>(* stringv[r]).second = true;
                                                        
                                                        rhs = decode_raw(rhs);
                                                    }
                                                } else
                                                    rhs = encode(parse_number(rhs));
                                                
                                                rhs = lhs + rhs;
                                                rhs = encode(rhs);
                                                
                                                std::get<1>(* arrayv[k])[q * 2 + 1] = rhs;
                                            } else {
                                                double d = parse_number(lhs);
                                                
                                                if (p != 1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (is_string(rhs))
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                    
                                                if (is_key(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(array_t, number_t);
                                                        //  array != double
                                                    
                                                    if (io_string(rhs) != -1)
                                                        type_error(string_t, number_t);
                                                        //  string != double
                                                    
                                                    d += get_number(rhs);
                                                } else
                                                    d += parse_number(rhs);
                                                
                                                rhs = encode(d);
                                                
                                                if (std::get<2>(* arrayv[k]).first)
                                                    write_error(lhs);
                                                
                                                std::get<1>(* arrayv[k])[q * 2 + 1] = rhs;
                                            }
                                        }
                                    } else {
                                        double index = parse_number(rhs);
                                        
                                        if (!is_int(index))
                                            type_error(number_t, int_t);
                                            //  double != int
                                        
                                        if (index < 0 || index >= std::get<1>(* arrayv[k]).size())
                                            range_error("index " + encode(index) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                        
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
                                                type_error(array_t, string_t);
                                                //  array != string
                                            
                                            lhs = decode_raw(lhs);
                                            
                                            if (rhs.empty())
                                                null_error();
                                            
                                            if (is_string(rhs))
                                                rhs = decode(rhs);
                                            
                                            else if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, string_t);
                                                    //  array != string
                                                
                                                int m = io_string(rhs);
                                                if (m == -1)
                                                    rhs = encode(get_number(rhs));
                                                else {
                                                    rhs = std::get<1>(* stringv[m]);
                                                    
                                                    if (rhs.empty())
                                                        null_error();
                                                    
                                                    std::get<2>(* stringv[m]).second = true;
                                                    
                                                    rhs = decode_raw(rhs);
                                                }
                                            } else
                                                rhs = encode(parse_number(rhs));
                                            
                                            rhs = lhs + rhs;
                                            rhs = encode(rhs);
                                            
                                            std::get<1>(* arrayv[k])[(size_t)index] = rhs;
                                        } else {
                                            double d = parse_number(std::get<1>(* arrayv[k])[(size_t)index]);
                                            
                                            if (p != 1)
                                                type_error(array_t, number_t);
                                                //  array != double
                                            
                                            if (is_string(rhs))
                                                type_error(string_t, number_t);
                                                //  string != double
                                                
                                            if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (io_string(rhs) != -1)
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                
                                                d += get_number(rhs);
                                            } else
                                                d += parse_number(rhs);
                                            
                                            rhs = encode(d);
                                            
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
                                                
                                            } else if (is_key(rhs)) {
                                                int l = io_array(rhs);
                                                if (l == -1) {
                                                    l = io_string(rhs);
                                                    if (l == -1)
                                                        rhs = encode(get_number(rhs));
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
                                                
                                                rhs = encode(parse_number(rhs));
                                                
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
                                    
                                    rhs = std::to_string(std::get<1>(* arrayv[k]).size());
                                }
                            }
                        } else {
                            size_t k = i + 1;
                            while (k < n && data[k] == "(")
                                ++k;
                            
                            if (data[k] != uov[indexer_pos]->opcode())
                                operation_error();
                            
                            rhs = operands.pop();
                            
                            //  empty indexer argument
                            if (rhs.empty()) {
                                delete[] v;
                                
                                //  null dictionary key
                                if (is_dictionary(p, v))
                                    null_error();
                                
                                type_error(string_t, int_t);
                                //  string != int
                            }
                            
                            if (ss::is_array(rhs)) {
                                delete[] v;
                                type_error(array_t, int_t);
                                //  array != int
                            }
                            
                            if (is_string(rhs)) {
                                if (!is_dictionary(p, v)) {
                                    delete[] v;
                                    type_error(array_t, dictionary_t);
                                    //  array != dictionary
                                }
                                
                                rhs = decode(rhs);
                                
                                if (rhs.empty()) {
                                    delete[] v;
                                    undefined_error(encode(null()));
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
                                    double number = parse_number(v[l * 2 + 1]);
                                    
                                    rhs = operands.pop();
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] v;
                                        type_error(string_t, number_t);
                                        //  string != double
                                    }
                                    
                                    number += parse_number(rhs);
                                    
                                    v[l * 2 + 1] = encode(number);
                                }
                            } else if (is_key(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(array_t, int_t);
                                    //  array != int
                                
                                int l = io_string(rhs);
                                if (l == -1) {
                                    double idx = get_number(rhs);
                                    
                                    if (!is_int(idx)) {
                                        delete[] v;
                                        type_error(number_t, int_t);
                                        //  double != int
                                    }
                                    
                                    if (idx < 0 || idx >= p)
                                        range_error("index " + encode(p) + ", count " + std::to_string(p));
                                    
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
                                        double number = parse_number(v[(size_t)idx]);
                                        
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty() || is_string(rhs)) {
                                            delete[] v;
                                            type_error(string_t, number_t);
                                            //  string != double
                                        }
                                        
                                        number += parse_number(rhs);
                                        
                                        v[(size_t)idx] = encode(number);
                                    }
                                } else {
                                    if (!is_dictionary(p, v)) {
                                        delete[] v;
                                        type_error(array_t, dictionary_t);
                                        //  array != dictionary
                                    }
                                    
                                    rhs = std::get<1>(* stringv[l]);
                                    
                                    if (rhs.empty()) {
                                        delete[] v;
                                        null_error();
                                    }
                                    
                                    if (rhs.length() == 2) {
                                        delete[] v;
                                        undefined_error(rhs);
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
                                        double number = parse_number(v[m * 2 + 1]);
                                        
                                        rhs = operands.pop();
                                        
                                        if (rhs.empty() || is_string(rhs)) {
                                            delete[] v;
                                            type_error(string_t, number_t);
                                            //  string != double
                                        }
                                        
                                        number += parse_number(rhs);
                                        
                                        v[m * 2 + 1] = encode(number);
                                    }
                                }
                            } else {
                                double idx = parse_number(rhs);
                                
                                if (!is_int(idx)) {
                                    delete[] v;
                                    type_error(number_t, int_t);
                                    //  double != int
                                }
                                
                                if (idx < 0 || idx >= p)
                                    range_error("index " + encode(p) + ", count " + std::to_string(p));
                                
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
                                    double number = parse_number(v[(size_t)idx]);
                                    
                                    rhs = operands.pop();
                                    
                                    if (rhs.empty() || is_string(rhs)) {
                                        delete[] v;
                                        type_error(string_t, number_t);
                                        //  string != double
                                    }
                                    
                                    number += parse_number(rhs);
                                    
                                    v[(size_t)idx] = encode(number);
                                }
                            }
                            
                            rhs = stringify(p, v);
                            delete[] v;
                        }
                    } else if (j == direct_assignment_pos) {
                        rhs = operands.pop();
                        
                        size_t k = i + 1;
                        while (k < n && data[k] == "(")
                            ++k;
                        
                        if (data[k] == uov[indexer_pos]->opcode()) {
                            ++k;
                            while (k < n && data[k] == "(")
                                ++k;
                            
                            string* valuev = new string[lhs.length() + 1];
                            size_t valuec = parse(valuev, lhs);
                            
                            if (valuec == 1) {
                                delete[] valuev;
                                
                                if (!is_key(lhs))
                                    operation_error();
                                
                                int k = io_array(lhs);
                                if (k == -1) {
                                    if (is_defined(lhs))
                                        operation_error();
                                    
                                    undefined_error(lhs);
                                } else {
                                    if (ss::is_array(rhs))
                                        type_error(array_t, int_t);
                                        //  array != int
                                    
                                    if (rhs.empty()) {
                                        if (is_dictionary(std::get<1>(* arrayv[k])))
                                            null_error();
                                        
                                        type_error(string_t, int_t);
                                        //  string != int
                                    }
                                    
                                    if (is_string(rhs)) {
                                        if (!is_dictionary(std::get<1>(* arrayv[k])))
                                            type_error(array_t, dictionary_t);
                                            //  array != dictionary
                                        
                                        rhs = decode(rhs);
                                        
                                        if (rhs.empty())
                                            undefined_error(encode(rhs));
                                        
                                        string ctr = encode(rhs);
                                        
                                        rhs = element(operands.pop());
                                        
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
                                    } else if (is_key(rhs)) {
                                        if (io_array(rhs) != -1)
                                            type_error(array_t, int_t);
                                            //  array != int
                                        
                                        int l = io_string(rhs);
                                        if (l == -1) {
                                            double ctr = get_number(rhs);
                                            
                                            if (!is_int(ctr))
                                                type_error(number_t, int_t);
                                                //  double != int
                                            
                                            if (ctr < 0 || ctr > std::get<1>(* arrayv[k]).size())
                                                range_error("index " + encode(ctr) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                            
                                            rhs = element(operands.pop());
                                            
                                            if (std::get<2>(* arrayv[k]).first)
                                                write_error(lhs);
                                            
                                            set_array(lhs, (int)ctr, rhs);
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
                                        double ctr = parse_number(rhs);
                                        
                                        if (!is_int(ctr))
                                            type_error(number_t, int_t);
                                            //  double != int
                                        
                                        if (ctr < 0 || ctr > std::get<1>(* arrayv[k]).size())
                                            range_error("index " + encode(ctr) + ", count " + std::to_string(std::get<1>(* arrayv[k]).size()));
                                        
                                        rhs = element(operands.pop());
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        set_array(lhs, (size_t)ctr, rhs);
                                    }
                                }
                            } else {
                                if (rhs.empty()) {
                                    delete[] valuev;
                                    
                                    if (is_dictionary(valuec, valuev))
                                        null_error();
                                    
                                    type_error(string_t, int_t);
                                    //  string != int
                                }
                                
                                if (ss::is_array(rhs)) {
                                    delete[] valuev;
                                    type_error(array_t, int_t);
                                    //  array != int
                                }
                                
                                if (is_string(rhs)) {
                                    if (!is_dictionary(valuec, valuev)) {
                                        delete[] valuev;
                                        type_error(array_t, dictionary_t);
                                        //  array != dictionary
                                    }
                                    
                                    rhs = decode(rhs);
                                    
                                    if (rhs.empty()) {
                                        delete[] valuev;
                                        undefined_error(encode(null()));
                                    }
                                    
                                    rhs = encode(rhs);
                                    
                                    string ctr = rhs;
                                    
                                    rhs = element(operands.pop());
                                    
                                    size_t k = 0;
                                    while (k < (size_t)floor(valuec / 2) && valuev[k * 2] != ctr)
                                        ++k;
                                    
                                    if (k == (size_t)floor(valuec / 2)) {
                                        string* tmp = new string[valuec + 2];
                                        for (size_t l = 0; l < valuec; ++l)
                                            tmp[l] = valuev[l];
                                        
                                        delete[] valuev;
                                        
                                        valuev = tmp;
                                        
                                        valuev[valuec++] = ctr;
                                        valuev[valuec++] = rhs;
                                    } else
                                        valuev[k * 2 + 1] = rhs;
                                } else if (is_key(rhs)) {
                                    if (io_array(rhs) != -1) {
                                        delete[] valuev;
                                        type_error(array_t, int_t);
                                        //  array != int
                                    }
                                    
                                    int k = io_string(rhs);
                                    if (k == -1) {
                                        double ctr = get_number(rhs);
                                        
                                        if (!is_int(ctr)) {
                                            delete[] valuev;
                                            type_error(number_t, int_t);
                                            //  double != int
                                        }
                                        
                                        if (ctr < 0 || ctr > valuec) {
                                            delete[] valuev;
                                            
                                            range_error("index " + encode(ctr) + ", count " + std::to_string(valuec));
                                        }
                                        
                                        rhs = element(operands.top());
                                        
                                        if (ctr == valuec) {
                                            string* tmp = new string[valuec + 1];
                                            for (size_t k = 0; k < valuec; ++k)
                                                tmp[k] = valuev[k];
                                            
                                            delete[] valuev;
                                            
                                            valuev = tmp;
                                            
                                            valuev[valuec++] = rhs;
                                        } else
                                            valuev[(size_t)ctr] = rhs;
                                    } else {
                                        if (!is_dictionary(valuec, valuev)) {
                                            delete[] valuev;
                                            type_error(array_t, dictionary_t);
                                            //  array != dictionary
                                        }
                                        
                                        rhs = std::get<1>(* stringv[k]);
                                        
                                        if (rhs.empty()) {
                                            delete[] valuev;
                                            null_error();
                                        }
                                        
                                        if (rhs.length() == 2) {
                                            delete[] valuev;
                                            undefined_error(encode(null()));
                                        }
                                        
                                        string ctr = rhs;
                                        
                                        rhs = element(operands.pop());
                                        
                                        size_t l = 0;
                                        while (l < (size_t)floor(valuec / 2) && valuev[l * 2] != ctr)
                                            ++l;
                                            //  search for key
                                        
                                        if (l == (size_t)floor(valuec / 2)) {
                                            string* tmp = new string[valuec + 2];
                                            
                                            for (size_t m = 0; m < valuec; ++m)
                                                tmp[m] = valuev[m];
                                            
                                            delete[] valuev;
                                            
                                            valuev = tmp;
                                            
                                            valuev[valuec++] = ctr;
                                            valuev[valuec++] = rhs;
                                        } else
                                            valuev[l * 2 + 1] = rhs;
                                    }
                                } else {
                                    double ctr = parse_number(rhs);
                                    
                                    if (!is_int(ctr)) {
                                        delete[] valuev;
                                        type_error(number_t, int_t);
                                        //  double != int
                                    }
                                    
                                    if (ctr < 0 || ctr > valuec) {
                                        delete[] valuev;
                                        range_error("index " + encode(ctr) + ", count " + std::to_string(valuec));
                                    }
                                    
                                    rhs = element(operands.pop());
                                    
                                    if (ctr == valuec) {
                                        string* tmp = new string[valuec + 1];
                                        for (size_t k = 0; k < valuec; ++k)
                                            tmp[k] = valuev[k];
                                        
                                        delete[] valuev;
                                        
                                        valuev = tmp;
                                        
                                        valuev[valuec++] = rhs;
                                    } else
                                        valuev[(size_t)ctr] = rhs;
                                }
                                
                                rhs = stringify(valuec, valuev);
                                
                                delete[] valuev;
                            }
                        } else {
                            if (!is_key(lhs))
                                operation_error();
                        
                            if (data[k] == uov[const_pos]->opcode() || data[k] == uov[var_pos]->opcode()) {
                                ++k;
                                
                                if (data[k] == uov[const_pos]->opcode() || data[k] == uov[var_pos]->opcode())
                                    throw error("Unexpected token: " + data[k]);
                            }
                            
                            if (data[k] == to_string(array_t)) {
                                if (is_defined(lhs)) {
                                    if (_get_state(lhs) == (int)_statec)
                                        defined_error(lhs);
                                    
                                    remove_key(lhs);
                                }
                                    
                                string valuev[rhs.length() + 1];
                                size_t valuec = parse(valuev, rhs);
                                
                                if (valuec == 1) {
                                    if (rhs.empty())
                                        set_array(lhs, 0, null());
                                    
                                    else if (is_string(rhs)) {
                                        rhs = encode(decode(rhs));
                                        
                                        set_array(lhs, 0, rhs);
                                        
                                    } else if (is_key(rhs)) {
                                        int l = io_array(rhs);
                                        if (l == -1) {
                                            l = io_string(rhs);
                                            if (l == -1) {
                                                rhs = encode(get_number(rhs));
                                                
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
                                        rhs = encode(parse_number(rhs));
                                        
                                        set_array(lhs, 0, rhs);
                                    }
                                } else {
                                    for (size_t m = 0; m < valuec; ++m)
                                        set_array(lhs, m, valuev[m]);
                                    
                                    std::get<1>(* arrayv[io_array(lhs)]).shrink_to_fit();
                                }
                                
                                if (data[i + 1] == uov[const_pos]->opcode())
                                    std::get<2>(* arrayv[io_array(lhs)]).first = true;
                            } else {
                                bool handle = true;
                                
                                if (data[i + 1] == uov[const_pos]->opcode() ||
                                    data[i + 1] == uov[var_pos]->opcode()) {
                                    if (is_defined(lhs)) {
                                        if (_get_state(lhs) == (int)_statec)
                                            defined_error(lhs);
                                        
                                        remove_key(lhs);
                                        
                                        handle = false;
                                    }
                                }
                                
                                if (handle) {
                                    handle = false;
                                    
                                    int k = io_array(lhs);
                                    if (k == -1) {
                                        k = io_string(lhs);
                                        if (k == -1) {
                                            if (is_defined(lhs)) {
//                                                if (data[i + 1] == uov[const_pos]->opcode())
//                                                    defined_error(lhs);
                                                
                                                if (ss::is_array(rhs))
                                                    type_error(array_t, number_t);
                                                    //  array != double
                                                
                                                if (rhs.empty() || is_string(rhs))
                                                    type_error(string_t, number_t);
                                                    //  string != double
                                                
                                                double ctr;
                                                
                                                if (is_key(rhs)) {
                                                    if (io_array(rhs) != -1)
                                                        type_error(array_t, number_t);
                                                        //  array != double
                                                        
                                                    if (io_string(rhs) != -1)
                                                        type_error(string_t, number_t);
                                                        //  string != double
                                                    
                                                    ctr = get_number(rhs);
                                                } else
                                                    ctr = parse_number(rhs);
                                                
                                                set_number(lhs, ctr);
                                                
                                                rhs = encode(ctr);
                                            } else
                                                handle = true;
                                        } else {
//                                            if (data[i + 1] == uov[const_pos]->opcode())
//                                                defined_error(lhs);
                                            
                                            if (ss::is_array(rhs))
                                                type_error(array_t, string_t);
                                                //  array != string
                                            
                                            if (is_string(rhs))
                                                rhs = encode(decode(rhs));
                                                
                                            else if (is_key(rhs)) {
                                                if (io_array(rhs) != -1)
                                                    type_error(array_t, string_t);
                                                    //  array != string
                                                
                                                int k = io_string(rhs);
                                                if (k == -1) {
                                                    if (is_defined(rhs))
                                                        type_error(number_t, string_t);
                                                        //  double != string
                                                    
                                                    undefined_error(rhs);
                                                }
                                                
                                                rhs = std::get<1>(* stringv[k]);
                                                
                                                std::get<2>(* stringv[k]).second = true;
                                            } else
                                                type_error(number_t, string_t);
                                                //  double != string
                                            
                                            set_string(lhs, rhs);
                                        }
                                    } else {
//                                        if (data[i + 1] == uov[const_pos]->opcode())
//                                            defined_error(lhs);
                                        
                                        if (std::get<2>(* arrayv[k]).first)
                                            write_error(lhs);
                                        
                                        std::get<1>(* arrayv[k]).clear();
                                        
                                        if (rhs.empty())
                                            std::get<1>(* arrayv[k]).push(rhs);
                                        else {
                                            string valuev[rhs.length() + 1];
                                            size_t valuec = parse(valuev, rhs);
                                            
                                            if (valuec == 1) {
                                                if (is_string(rhs)) {
                                                    rhs = decode(rhs);
                                                    rhs = encode(rhs);
                                                    
                                                    std::get<1>(* arrayv[k]).push(rhs);
                                                    
                                                } else if (is_key(rhs)) {
                                                    int l = io_array(rhs);
                                                    if (l == -1) {
                                                        l = io_string(rhs);
                                                        if (l == -1)
                                                            rhs = encode(get_number(rhs));
                                                        else {
                                                            rhs = std::get<1>(* stringv[l]);
                                                            
                                                            std::get<2>(* stringv[l]).second = true;
                                                        }
                                                        
                                                        std::get<1>(* arrayv[k]).push(rhs);
                                                    } else {
                                                        valuec = std::get<1>(* arrayv[l]).size();
                                                        
                                                        for (size_t m = 0; m < valuec; ++m)
                                                            std::get<1>(* arrayv[k]).push(std::get<1>(* arrayv[l])[m]);
                                                        
                                                        std::get<2>(* arrayv[l]).second = true;
                                                        
                                                        rhs = stringify(std::get<1>(* arrayv[k]));
                                                    }
                                                } else {
                                                    rhs = encode(parse_number(rhs));
                                                    
                                                    std::get<1>(* arrayv[k]).push(rhs);
                                                }
                                            } else
                                                for (size_t m = 0; m < valuec; ++m)
                                                    std::get<1>(* arrayv[k]).push(valuev[m]);
                                        }
                                    }
                                } else
                                    handle = true;
                                
                                if (handle) {
                                    if (rhs.empty()) {
                                        set_string(lhs, rhs);
                                        
                                        if (data[i + 1] == uov[const_pos]->opcode())
                                            std::get<2>(* stringv[io_string(lhs)]).first = true;
                                    } else {
                                        string valuev[rhs.length() + 1];
                                        size_t valuec = parse(valuev, rhs);
                                        
                                        if (valuec == 1) {
                                            if (is_string(rhs)) {
                                                rhs = encode(decode(rhs));
                                                
                                                set_string(lhs, rhs);
                                                
                                                if (data[i + 1] == uov[const_pos]->opcode())
                                                    std::get<2>(* stringv[io_string(lhs)]).first = true;
                                                
                                            } else if (is_key(rhs))  {
                                                k = io_array(rhs);
                                                if (k == -1) {
                                                    k = io_string(rhs);
                                                    if (k == -1) {
                                                        double ctr = get_number(rhs);
                                                        
                                                        set_number(lhs, ctr);
                                                        
                                                        if (data[i + 1] == uov[const_pos]->opcode())
                                                            set_read_only(lhs, true);
                                                        
                                                        rhs = encode(ctr);
                                                    } else {
                                                        rhs = std::get<1>(* stringv[k]);
                                                        
                                                        std::get<2>(* stringv[k]).second = true;
                                                        
                                                        set_string(lhs, rhs);
                                                        
                                                        if (data[i + 1] == uov[const_pos]->opcode())
                                                            std::get<2>(* stringv[io_string(lhs)]).first = true;
                                                    }
                                                } else {
                                                    set_array(lhs, 0, std::get<1>(* arrayv[k])[0]);
                                                    
                                                    k = io_array(rhs);
                                                    
                                                    for (size_t l = 1; l < std::get<1>(* arrayv[k]).size(); ++l)
                                                        set_array(lhs, l, std::get<1>(* arrayv[k])[l]);
                                                    
                                                    if (data[i + 1] == uov[const_pos]->opcode()) {
                                                        k = io_array(lhs);
                                                        std::get<1>(* arrayv[k]).shrink_to_fit();
                                                        std::get<2>(* arrayv[k]).first = true;
                                                    }
                                                        
                                                }
                                            } else {
                                                double ctr = parse_number(rhs);
                                                
                                                set_number(lhs, ctr);
                                                
                                                if (data[i + 1] == uov[const_pos]->opcode())
                                                    set_read_only(lhs, true);
                                                
                                                rhs = encode(ctr);
                                            }
                                        } else {
                                            _set_array(lhs, valuec, valuev);
                                        
                                            if (data[i + 1] == uov[const_pos]->opcode())
                                                std::get<2>(* arrayv[io_array(lhs)]).first = true;
                                        }
                                    }
                                }
                            }
                        }
                    } else if (j == conditional_pos) {
                        double flag;
                        
                        string valuev[lhs.length() + 1];
                        size_t valuec = parse(valuev, lhs);
                        
                        if (valuec == 1) {
                            if (lhs.empty())
                                flag = 0;
                            
                            else if (is_string(lhs))
                                flag = !decode_raw(lhs).empty();
                            
                            else if (is_key(lhs)) {
                                int k = io_array(lhs);
                                if (k == -1) {
                                    k = io_string(lhs);
                                    if (k == -1)
                                        flag = get_number(lhs);
                                    else {
                                        string str = std::get<1>(* stringv[k]);
                                        
                                        if (str.empty())
                                            flag = 0;
                                        else {
                                            str = decode_raw(str);
                                            flag = !str.empty();
                                        }
                                        
                                        std::get<2>(* stringv[k]).second = true;
                                    }
                                } else {
                                    std::get<2>(* arrayv[k]).second = true;
                                    
                                    if (std::get<1>(* arrayv[k]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[k])[0];
                                        
                                        if (lhs.empty())
                                            flag = 0;
                                        
                                        else if (is_string(lhs)) {
                                            string str = decode_raw(lhs);
                                            
                                            flag = !str.empty();
                                        } else
                                            flag = stod(lhs);
                                    } else
                                        flag = 1;
                                }
                            } else
                                flag = parse_number(lhs);
                        } else
                            flag = 1;
                        
                        if (flag) {
                            rhs = evaluate(operands.pop());
                            
                            operands.pop();
                        } else {
                            operands.pop();
                            
                            rhs = evaluate(operands.pop());
                        }
                    } else if (j == nullish_coalescing_pos) {
                        rhs = evaluate(lhs);
                        
                        if (rhs.empty() || rhs == null())
                            rhs = evaluate(operands.top());
                        
                        operands.pop();
                    } else {
                        rhs = operands.pop();
                        rhs = ((binary_universal_operator *)uov[j])->apply(lhs, rhs);
                    }
                    
                    operands.push(rhs);
                }
            }
        }
        
        if (!operands.size())
            return null();
        
        while (operands.size() > 1) {
            if (operands.top().empty() ||
                ss::is_array(operands.top()) ||
                is_string(operands.top())) {
                    operands.pop();
                    continue;
            }
            
            if (is_key(operands.top())) {
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
        
        if (is_string(operands.top()))
            return encode(decode(operands.pop()));
        
        if (is_key(operands.top())) {
            int i = io_array(operands.top());
            
            if (i == -1) {
                i = io_string(operands.top());
                
                if (i == -1)
                    return encode(get_number(operands.pop()));
                
                std::get<2>(* stringv[i]).second = true;
                
                return std::get<1>(* stringv[i]);
            }
            
            std::get<2>(* arrayv[i]).second = true;
            
            return stringify(std::get<1>(* arrayv[i]));;
        }
        
#if DEBUG_LEVEL == 3
        logger_write(std::to_string(duration<double>(steady_clock::now() - beg).count()) + "\n");
#endif
        return encode(parse_number(operands.pop()));
    }

    size_t command_processor::get_state() {
        if (is_pow(statec, 2)) {
            //  resize numbers
            size_t* _state_numberv = new size_t[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_numberv[i] = state_numberv[i];
            
            delete[] state_numberv;
            state_numberv = _state_numberv;
            
            //  resize functions
            pair<size_t, function_t**>** _state_functionv = new pair<size_t, function_t**>*[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_functionv[i] = state_functionv[i];
            
            delete[] state_functionv;
            state_functionv = _state_functionv;
            
            //  resize arrays
            //  array value is a pointer, as its internal pointer will be deallocated otherwise when control exits the array's declaring scope
            pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>** _state_arrayv = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>*[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_arrayv[i] = state_arrayv[i];
            
            delete[] state_arrayv;
            state_arrayv = _state_arrayv;
            
            //  resize strings
            pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>** _state_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>*[statec * 2];
            
            for (size_t i = 0; i < statec;  ++i)
                _state_stringv[i] = state_stringv[i];
            
            delete[] state_stringv;
            state_stringv = _state_stringv;
        }
        
        size_t _state = logic::get_state();
        
        state_numberv[statec] = _state;
        
        //  get_state arrays
        tuple<string, ss::array<string>*, pair<bool, bool>, size_t>** _arrayv = new tuple<string, ss::array<string>*, pair<bool, bool>, size_t>*[pow2(arrayc)];
        
        for (size_t i = 0; i < arrayc; ++i) {
            string key = std::get<0>(* arrayv[i]);
            ss::array<string>* value = new ss::array<string>(std::get<1>(* arrayv[i]));
            pair<bool, bool> flags = std::get<2>(* arrayv[i]);
            size_t state = std::get<3>(* arrayv[i]);
            
            _arrayv[i] = new tuple<string, ss::array<string>*, pair<bool, bool>, size_t>(key, value, flags, state);
        }
        
        state_arrayv[statec] = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>(arrayc, _arrayv);
        
        function_t** _functionv = new function_t*[pow2(functionc)];

        for (size_t i = 0; i < functionc; ++i)
            _functionv[i] = functionv[i];
    
        state_functionv[statec] = new pair<size_t, function_t**>(functionc, _functionv);
        
        tuple<string, string, pair<bool, bool>, size_t>** _stringv = new tuple<string, string, pair<bool, bool>, size_t>*[pow2(stringc)];
        
        for (size_t i = 0; i < stringc; ++i) {
            string key = std::get<0>(* stringv[i]);
            string value = std::get<1>(* stringv[i]);
            pair<bool, bool> flags = std::get<2>(* stringv[i]);
            size_t state = std::get<3>(* stringv[i]);
            
            _stringv[i] = new tuple<string, string, pair<bool, bool>, size_t>(key, value, flags, state);
        }
        
        state_stringv[statec] = new pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>(stringc, _stringv);
        
        ++statec;
        
        return _state;
    }

    void command_processor::get_state(const size_t state) {
        //  resize states
        if (is_pow(statec, 2)) {
            //  resize numbers
            size_t* _state_numberv = new size_t[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_numberv[i] = state_numberv[i];
            
            delete[] state_numberv;
            state_numberv = _state_numberv;
            
            //  resize functions
            pair<size_t, function_t**>** _state_functionv = new pair<size_t, function_t**>*[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_functionv[i] = state_functionv[i];
            
            delete[] state_functionv;
            state_functionv = _state_functionv;
            
            //  resize arrays
            //  array value is a pointer, as its internal pointer will be deallocated otherwise when control exits the array's declaring scope
            pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>** _state_arrayv = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>*[statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_arrayv[i] = state_arrayv[i];
            
            delete[] state_arrayv;
            state_arrayv = _state_arrayv;
            
            //  resize strings
            pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>** _state_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>*[statec * 2];
            
            for (size_t i = 0; i < statec;  ++i)
                _state_stringv[i] = state_stringv[i];
            
            delete[] state_stringv;
            state_stringv = _state_stringv;
        }
        
        logic::get_state(state);
        
        state_numberv[statec] = state;
        
        tuple<string, ss::array<string>*, pair<bool, bool>, size_t>** _arrayv = new tuple<string, ss::array<string>*, pair<bool, bool>, size_t>*[pow2(arrayc)];
        
        for (size_t i = 0; i < arrayc; ++i) {
            string key = std::get<0>(* arrayv[i]);
            ss::array<string>* value = new ss::array<string>(std::get<1>(* arrayv[i]));
            pair<bool, bool> flags = std::get<2>(* arrayv[i]);
            size_t state = std::get<3>(* arrayv[i]);
            
            _arrayv[i] = new tuple<string, ss::array<string>*, pair<bool, bool>, size_t>(key, value, flags, state);
        }
        
        state_arrayv[statec] = new pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>(arrayc, _arrayv);
        
        function_t** _functionv = new function_t*[pow2(functionc)];
        
        for (size_t i = 0; i < functionc; ++i)
            _functionv[i] = functionv[i];
        
        state_functionv[statec] = new pair<size_t, function_t**>(functionc, _functionv);
        
        //  get_state strings
        tuple<string, string, pair<bool, bool>, size_t>** _stringv = new tuple<string, string, pair<bool, bool>, size_t>*[pow2(stringc)];
        
        for (size_t i = 0; i < stringc; ++i) {
            string key = std::get<0>(* stringv[i]);
            string value = std::get<1>(* stringv[i]);
            pair<bool, bool> flags = std::get<2>(* stringv[i]);
            size_t state = std::get<3>(* stringv[i]);
            
            _stringv[i] = new tuple<string, string, pair<bool, bool>, size_t>(key, value, flags, state);
        }
        
        state_stringv[statec] = new pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>(stringc, _stringv);
        
        ++statec;
    }

    string command_processor::get_string(const string key) {
        int i = io_string(key);
        
        if (i == -1)
            undefined_error(key);
        
        std::get<2>(* stringv[i]).second = true;
        
        return std::get<1>(* stringv[i]);
    }

    void command_processor::initialize() {
        //  BEGIN OPERATORS
        
        uov = new operator_t*[83];

        uov[uoc++] = new unary_universal_operator("count", [this](const string rhs) {
            if (rhs.empty())
                null_error();
            
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty())
                    null_error();
                
                if (is_string(rhs)) {
                    string str = decode(rhs);
                    
                    return std::to_string(str.length());
                }
                
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                
                int i = io_array(rhs);
                if (i == -1) {
                    i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(number_t, string_t);
                        
                        undefined_error(rhs);
                    }
                    
                    if (std::get<1>(* stringv[i]).empty())
                        null_error();
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    return std::to_string(decode_raw(std::get<1>(* stringv[i])).length());
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::to_string(std::get<1>(* arrayv[i]).size());
            }
            
            return std::to_string(valuec);
        });
        //  10
        
        uov[const_pos = uoc++] = new unary_universal_operator("const", [this](string rhs) {
            unsupported_error("const");
            return nullptr;
        });
        
        uov[uoc++] = new unary_universal_operator("first", [this](const string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, array_t);
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        if (io_string(rhs) != -1)
                            type_error(string_t, array_t);
                        
                        if (is_defined(rhs))
                            type_error(number_t, array_t);
                        
                        undefined_error(rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return std::get<1>(* arrayv[i])[0];
                }
                
                type_error(number_t, array_t);
            }
            
            return valuev[0];
        });
        
        uov[uoc++] = new unary_universal_operator("inverse", [this](string rhs) {
           if (rhs.empty())
               type_error(string_t, table_t);
                //  string != table
            
            string* data = new string[rhs.length() + 1];
            size_t n = parse(data, rhs);
            
            if (n == 1) {
                delete[] data;
                
                if (is_string(rhs))
                    type_error(string_t, table_t);
                    //  string != table
                
                if (!is_key(rhs))
                    type_error(number_t, table_t);
                    //  double != table
                
                int i = io_array(rhs);
                if (i == -1) {
                    if (io_string(rhs) != -1)
                        type_error(string_t, table_t);
                        //  string != table
                    
                    if (is_defined(rhs))
                        type_error(number_t, table_t);
                        //  double != table
                    
                    undefined_error(rhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(array_t, table_t);
                    //  array != table
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                size_t r = (std::get<1>(* arrayv[i]).size() - 1) / c;
                
                data = new string[std::get<1>(* arrayv[i]).size()];
                n = 0;
                
                data[n++] = std::to_string(r);
                
                for (size_t j = 0; j < c; ++j)
                    for (size_t k = 0; k < r; ++k)
                        data[n++] = std::get<1>(* arrayv[i])[j + (k * c) + 1];
                
                rhs = stringify(n, data);
                
                delete[] data;
                
                return rhs;
            }
            
            if (!is_table(n, data))
                type_error(array_t, table_t);
                //  array != table
            
            size_t c = stoi(data[0]);
            size_t r = (n - 1) / c;
            
            string result[n];
            size_t size = 0;
            
            result[size++] = std::to_string(r);
            
            for (size_t i = 0; i < c; ++i)
                for (size_t j = 0; j < r; ++j)
                    result[size++] = data[i + (j * c) + 1];
                    
            return stringify(size, result);
        });
        
        uov[uoc++] = new unary_universal_operator("isalpha", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && isalpha(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        //  5
        
        uov[uoc++] = new unary_universal_operator("isalnum", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && isalnum(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        //  6
        
        uov[uoc++] = new unary_universal_operator("isdigits", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && isdigit(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        //  8
        
        uov[uoc++] = new unary_universal_operator("islower", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && islower(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        //  7
        
        uov[uoc++] = new unary_universal_operator("isspace", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && isspace(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        
        uov[uoc++] = new unary_universal_operator("isupper", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                std::get<2>(* stringv[i]).second = true;
                
                if (rhs.empty())
                    null_error();
            }
            
            rhs = decode(rhs);
            
            size_t i = 0;
            while (i < rhs.length() && isupper(rhs[i]))
                ++i;
            
            return std::to_string(i == rhs.length());
        });
        //  9
        
        uov[uoc++] = new unary_universal_operator("keys", [this](string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, dictionary_t);
                    //  string != dictionary
                
                if (!is_key(rhs))
                    type_error(number_t, dictionary_t);
                
                int i = io_array(rhs);
                if (i == -1) {
                    if (io_string(rhs) != -1)
                        type_error(string_t, dictionary_t);
                        //  string != dictionary
                    
                    if (is_defined(rhs))
                        type_error(number_t, dictionary_t);
                        //  double != dictionary
                    
                    undefined_error(rhs);
                }
                
                if (!is_dictionary(std::get<1>(* arrayv[i])))
                    type_error(array_t, dictionary_t);
                    //  array != dictionary
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                size_t j;
                
                for (j = 0; j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[j * 2] << get_sep();
                
                ss << std::get<1>(* arrayv[i])[j * 2];
                
                return ss.str();
            }
            
            if (!is_dictionary(valuec, valuev))
                type_error(array_t, dictionary_t);
                //  array != dictionary
            
            stringstream ss;
            size_t i;
            for (i = 0; i < (size_t)floor(valuec / 2) - 1; ++i)
                ss << valuev[i * 2] << get_sep();
            
            ss << valuev[i * 2];
            
            return ss.str();
        });
        
        uov[uoc++] = new unary_universal_operator("last", [this](const string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, array_t);
                
                if (!is_key(rhs))
                    type_error(number_t, array_t);
                
                int i = io_array(rhs);
                if (i == -1) {
                    if (io_string(rhs) != -1)
                        type_error(string_t, array_t);
                    
                    if (is_defined(rhs))
                        type_error(number_t, array_t);
                    
                    undefined_error(rhs);
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::get<1>(* arrayv[i])[std::get<1>(* arrayv[i]).size() - 1];
            }
            
            return valuev[valuec - 1];
        });
        
        uov[uoc++] = new unary_universal_operator("parse", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (!is_string(rhs)) {
                if (rhs.empty())
                    null_error();
                
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
               
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
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
                return encode(parse_number(rhs));
                
            } catch (invalid_argument& ia) {
                return string("nan");
            }
        });
        
        uov[shrink_pos = uoc++] = new unary_universal_operator("shrink", [this](string rhs) {
            if (!is_key(rhs))
                operation_error();
            
            int i = io_array(rhs);
            if (i == -1) {
                if (io_string(rhs) != -1)
                    type_error(string_t, array_t);
                
                if (is_defined(rhs))
                    type_error(number_t, array_t);
                
                undefined_error(rhs);
            }
            
            if (std::get<2>(* arrayv[i]).first)
                write_error(rhs);
            
            rhs = std::to_string(std::get<1>(* arrayv[i]).capacity());
            
            std::get<1>(* arrayv[i]).shrink_to_fit();
               
            return rhs;
        });
        
        uov[uoc++] = new unary_universal_operator("sizeof", [this](const string rhs) {
            if (!is_key(rhs))
                operation_error();
            
            int i = io_array(rhs);
            if (i == -1) {
                if (io_string(rhs) != -1)
                    type_error(string_t, array_t);
                
                if (is_defined(rhs))
                    type_error(number_t, array_t);
                
                undefined_error(rhs);
            }
            
            std::get<2>(* arrayv[i]).second = true;
            
            return std::to_string(std::get<1>(* arrayv[i]).capacity());
        });
        
        uov[uoc++] = new unary_universal_operator("subtypeof", [this](string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec != 1)
                return encode(is_table(valuec, valuev) ?
                  to_string(table_t) :
                  is_dictionary(valuec, valuev) ?
                  to_string(dictionary_t) :
                  to_string(array_t));
                //  array
            
            if (rhs.empty())
               return encode(to_string(string_t));
                //  string
            
            if (is_string(rhs))
                return encode(decode(rhs).length() == 1 ? to_string(char_t) : to_string(string_t));
                //  char | string
            
            if (is_key(rhs)) {
                int i = io_array(rhs);
                if (i == -1) {
                    i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs)) {
                            if (is_int(get_number(rhs)))
                                return encode(to_string(int_t));
                                //  int
                                
                            return encode(to_string(number_t));
                            //  double
                        }
                        
                        return null();
                    }
                    
                    rhs = std::get<1>(* stringv[i]);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (rhs.empty())
                        return encode(to_string(string_t));
                        //  string
                    
                    return encode(rhs.length() == 3 ? to_string(char_t) : to_string(string_t));
                    //  char : string
                }
                
                std::get<2>(* arrayv[i]).second = true;
                
                return encode(is_table(std::get<1>(* arrayv[i])) ?
                              to_string(table_t) :
                              is_dictionary(std::get<1>(* arrayv[i])) ?
                              to_string(dictionary_t) :
                              to_string(array_t));
                //  array
            }
            
            double num = parse_number(rhs);
            
            if (is_int(num))
                return encode(to_string(int_t));
                //  int
                
            return encode(to_string(number_t));
            //  double
        });
        
        uov[uoc++] = new unary_universal_operator("tochar", [this](string rhs) {
            if (rhs.empty())
                type_error(string_t, int_t);
                //  string != int
            
            if (ss::is_array(rhs))
                type_error(array_t, int_t);
                //  array != int
            
            double d;
            
            if (is_string(rhs))
                type_error(string_t, int_t);
                //  string != int
            
            if (is_key(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(array_t, int_t);
                    //  array != int
                
                if (io_string(rhs) != -1)
                    type_error(string_t, int_t);
                    //  string != int
                
                d = get_number(rhs);
            } else
                d = parse_number(rhs);
            
            if (!is_int(d))
                type_error(number_t, int_t);
                //  double != int
            
            if (d < 0 || d >= 128)
                range_error(std::to_string((size_t)d));
            
            rhs = string((char[]){ (char)((size_t)d), '\0' });
            
            return encode(rhs);
        });
        
        uov[uoc++] = new unary_universal_operator("tochararray", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
            
            if (rhs.empty())
                null_error();
            
            if (is_string(rhs)) {
                rhs = decode(rhs);
                
                stringstream ss;
                
                if (rhs.length()) {
                    for (size_t i = 0; i < rhs.length() - 1; ++i)
                        ss << encode(string((char[]){ rhs[i], '\0' })) << get_sep();
                    
                    ss << encode(string((char[]){ rhs[rhs.length() - 1], '\0' }));
                }
                
                return ss.str();
            }
            
            if (!is_key(rhs))
                type_error(number_t, string_t);
                
            if (io_array(rhs) != -1)
                type_error(array_t, string_t);
                
            int i = io_string(rhs);
            if (i == -1) {
                if (is_defined(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                undefined_error(rhs);
            }
            
            rhs = std::get<1>(* stringv[i]);
            
            if (rhs.empty())
                null_error();
            
            rhs = decode_raw(rhs);
            
            stringstream ss;
            
            if (rhs.length()) {
                for (size_t j = 0; j < rhs.length() - 1; ++j)
                    ss << encode(string((char[]){ rhs[j], '\0' })) << get_sep();
                
                ss << encode(string((char[]){ rhs[rhs.length() - 1], '\0' }));
            }
            
            return ss.str();
        });
        
        uov[uoc++] = new unary_universal_operator("tocharcode", [this](string rhs) {
            if (ss::is_array(rhs))
                type_error(array_t, char_t);
            
            if (rhs.empty())
                null_error();
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, char_t);
                    //  double != char
                
                if (io_array(rhs) != -1)
                    type_error(array_t, char_t);
                    //  array != char
                    
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, char_t);
                        //  double != char
                    
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = decode(rhs);
            
            if (rhs.length() != 1)
                type_error(string_t, char_t);
                //  string != char
            
            return std::to_string((size_t)rhs[0]);
        });
        
        uov[uoc++] = new unary_universal_operator("tolower", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = tolower(decode(rhs));
            
            return encode(rhs);
        });
        //  14
        
        uov[uoc++] = new unary_universal_operator("toupper", [this](string rhs) {
            if (rhs.empty())
                null_error();
            
            if (ss::is_array(rhs))
                type_error(array_t, string_t);
                //  array != string
            
            if (!is_string(rhs)) {
                if (!is_key(rhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                if (io_array(rhs) != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                int i = io_string(rhs);
                if (i == -1) {
                    if (is_defined(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                        
                    undefined_error(rhs);
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty())
                    null_error();
                
                std::get<2>(* stringv[i]).second = true;
            }
            
            rhs = tolower(decode(rhs));
            
            return encode(rhs);
        });
        //  15
        
        uov[uoc++] = new unary_universal_operator("typeof", [this](const string rhs) {
            if (ss::is_array(rhs))
                return encode(to_string(array_t));
            
            if (rhs.empty() || is_string(rhs))
                return encode(to_string(string_t));
            
            if (is_key(rhs)) {
                int pos = io_array(rhs);
                
                if (pos == -1) {
                    pos = io_string(rhs);
                    
                    if (pos == -1) {
                        if (is_defined(rhs)) {
                            get_number(rhs);
                            
                            return encode(to_string(number_t));
                        }
                        
                        return null();
                    }
                    
                    std::get<2>(* stringv[pos]).second = true;
                    
                    return encode(to_string(string_t));
                }
                
                std::get<2>(* arrayv[pos]).second = true;
                
                return encode(to_string(array_t));
            }
            
            return encode(to_string(number_t));
        });
        
        uov[uoc++] = new unary_universal_operator("values", [this](string rhs) {
            string valuev[rhs.length() + 1];
            size_t valuec = parse(valuev, rhs);
            
            if (valuec == 1) {
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, dictionary_t);
                    //  string != dictionary
                
                if (!is_key(rhs))
                    type_error(number_t, dictionary_t);
                
                int i = io_array(rhs);
                if (i == -1) {
                    if (io_string(rhs) != -1)
                        type_error(string_t, dictionary_t);
                        //  string != dictionary
                    
                    if (is_defined(rhs))
                        type_error(number_t, dictionary_t);
                        //  double != dictionary
                    
                    undefined_error(rhs);
                }
                
                if (!is_dictionary(std::get<1>(* arrayv[i])))
                    type_error(array_t, dictionary_t);
                    //  array != dictionary
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                size_t j;
                for (j = 0; j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[j * 2 + 1] << get_sep();
                
                ss << std::get<1>(* arrayv[i])[j * 2 + 1];
                
                return ss.str();
            }
            
            if (!is_dictionary(valuec, valuev))
                type_error(array_t, dictionary_t);
                //  array != dictionary
            
            stringstream ss;
            size_t i;
            
            for (i = 0; i < (size_t)floor(valuec / 2) - 1; ++i)
                ss << valuev[i * 2 + 1] << get_sep();
            
            ss << valuev[i * 2 + 1];
            
            return ss.str();
        });
        
        uov[var_pos = uoc++] = new unary_universal_operator("var", [this](string rhs) {
            unsupported_error("var");
            
            return nullptr;
        });

        uov[unary_count = (indexer_pos = uoc)] = new binary_universal_operator(".", [this](string lhs, string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    lhs = decode(lhs);
                    
                    if (rhs.empty())
                        type_error(string_t, int_t);
                        //  string != int
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, int_t);
                        //  array != int
                    
                    if (is_string(rhs))
                        type_error(string_t, int_t);
                        //  string != int
                    
                    double num;
                    
                    if (is_key(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(array_t, int_t);
                            //  array != int
                        
                        if (io_string(rhs) != -1)
                            type_error(string_t, int_t);
                            //  string != int
                        
                        num = get_number(rhs);
                    } else
                        num = parse_number(rhs);
                    
                    if (!is_int(num))
                        type_error(number_t, int_t);
                        //  double != int
                    
                    if (num < 0 || num >= lhs.length())
                        range_error("index " + encode(num) + ", count " + std::to_string(lhs.length()));
                    
                    rhs = string((char[]){ lhs[(size_t)num], '\0' });
                    
                    return encode(rhs);
                }
                
                if (!is_key(lhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, string_t);
                            //  double != string
                        
                        undefined_error(lhs);
                    }
                    
                    lhs = std::get<1>(* stringv[i]);
                    
                    if (lhs.empty())
                        null_error();
                    
                    lhs = decode_raw(lhs);
                    
                    if (rhs.empty())
                        type_error(string_t, int_t);
                        //  string != int
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, int_t);
                        //  array != int
                    
                    if (is_string(rhs))
                        type_error(string_t, int_t);
                        //  string != int
                    
                    double num;
                    
                    if (is_key(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(array_t, int_t);
                            //  array != int
                        
                        if (io_string(rhs) != -1)
                            type_error(string_t, int_t);
                            //  string != int
                        
                        num = get_number(rhs);
                    } else
                        num = parse_number(rhs);
                    
                    if (!is_int(num))
                        type_error(number_t, int_t);
                        //  double != int
                    
                    if (num < 0 || num >= lhs.length())
                        range_error("index " + encode(num) + ", count " + std::to_string(lhs.length()));
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    rhs = string((char[]){ lhs[(size_t)num], '\0' });
                    
                    return encode(rhs);
                }
                
                if (ss::is_array(rhs))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (rhs.empty()) {
                    if (is_dictionary(std::get<1>(* arrayv[i])))
                        null_error();
                    
                    type_error(array_t, dictionary_t);
                    //  string != int
                }
                
                if (is_string(rhs))  {
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(array_t, dictionary_t);
                        //  array != dictionary
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = encode(decode(rhs));
                    
                    size_t j = 0;
                    while (j < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) && std::get<1>(* arrayv[i])[j * 2] != rhs)
                        ++j;
                    
                    if (j == (size_t)floor(std::get<1>(* arrayv[i]).size() / 2))
                        return null();
                    
                    return std::get<1>(* arrayv[i])[j * 2 + 1];
                }
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, int_t);
                        //  array != int
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double num = get_number(rhs);
                        
                        if (!is_int(num))
                            type_error(number_t, int_t);
                            //  double != int
                        
                        if (num < 0 || num >= std::get<1>(* arrayv[i]).size())
                            range_error("index " + encode(num) + ", count " + std::to_string(std::get<1>(* arrayv[i]).size()));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        return std::get<1>(* arrayv[i])[(size_t)num];
                    }
                    
                    rhs = std::get<1>(* stringv[j]);
                    
                    if (rhs.empty()) {
                        if (is_dictionary(std::get<1>(* arrayv[i])))
                            null_error();
                        
                        type_error(array_t, dictionary_t);
                    }
                    
                    if (!is_dictionary(std::get<1>(* arrayv[i])))
                        type_error(array_t, dictionary_t);
                        //  array != dictionary
                    
                    std::get<2>(* arrayv[i]).second = true;
                    std::get<2>(* stringv[j]).second = true;
                    
                    size_t k = 0;
                    while (k < (size_t)floor(std::get<1>(* arrayv[i]).size() / 2) && std::get<1>(* arrayv[i])[k * 2] != rhs)
                        ++k;
                    
                    if (k == (size_t)floor(std::get<1>(* arrayv[i]).size() / 2))
                        return null();
                    
                    return std::get<1>(* arrayv[i])[k * 2 + 1];
                }
                
                double num = parse_number(rhs);
                
                if (!is_int(num))
                    type_error(number_t, int_t);
                    //  double != int
                
                if (num < 0 || num >= std::get<1>(* arrayv[i]).size())
                    range_error("index " + encode(num) + ", count " + std::to_string(std::get<1>(* arrayv[i]).size()));
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::get<1>(* arrayv[i])[(size_t)num];
            }
            
            if (rhs.empty()) {
                if (is_dictionary(valuec, valuev))
                    null_error();
                
                type_error(array_t, dictionary_t);
                //  array != dictionary
            }
            
            if (ss::is_array(rhs))
                type_error(array_t, int_t);
                //  array != int
            
            if (is_string(rhs)) {
                if (!is_dictionary(valuec, valuev))
                    type_error(array_t, dictionary_t);
                    //  array != dictionary
                
                rhs = encode(decode(rhs));
                
                size_t i = 0;
                while (i < (size_t)floor(valuec / 2) && valuev[i * 2] != rhs)
                    ++i;
                
                if (i == (size_t)floor(valuec / 2))
                    return null();
                
                return valuev[i * 2 + 1];
            }
            
            if (is_key(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(array_t, int_t);
                    //  array != int
                
                int i = io_string(rhs);
                if (i == -1) {
                    double num = get_number(rhs);
                    
                    if (!is_int(num))
                        type_error(number_t, int_t);
                        //  double != int
                    
                    if (num < 0 || num >= valuec)
                        range_error("index " + encode(num) + ", count " + std::to_string(valuec));
                    
                    return valuev[(size_t)num];
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty()) {
                    if (is_dictionary(valuec, valuev))
                        null_error();
                    
                    type_error(array_t, dictionary_t);
                }
                
                if (!is_dictionary(valuec, valuev))
                    type_error(array_t, dictionary_t);
                    //  array != dictionary
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < (size_t)floor(valuec / 2) && valuev[j * 2] != rhs)
                    ++j;
                
                if (j == (size_t)floor(valuec / 2))
                    return null();
                
                return valuev[j * 2 + 1];
            }
            
            double num = parse_number(rhs);
                
            if (!is_int(num))
                type_error(number_t, int_t);
                //  double != int
            
            if (num < 0 || num >= valuec)
                range_error("index " + encode(num) + ", count " + std::to_string(valuec));
            
            return valuev[(size_t)num];
        });
        
        ++uoc;

        uov[arithmetic_pos = uoc++] = new binary_universal_operator("^^", [this](const string lhs, const string rhs) {
            unsupported_error("^^");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("*", [this](const string lhs, const string rhs) {
            unsupported_error("*");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("/", [this](const string lhs, const string rhs) {
            unsupported_error("/");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("%", [this](const string lhs, const string rhs) {
            unsupported_error("%");
            return nullptr;
        });

        uov[additive_pos = uoc++] = new binary_universal_operator("+", [this](string lhs, string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
                        
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    if (rhs.empty())
                        null_error();
                    
                    lhs = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, string_t);
                        //  array != string
                    
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_key(rhs)) {
                        if (io_array(rhs) != -1)
                            type_error(array_t, string_t);
                            //  array != string
                        
                        int i = io_string(rhs);
                        
                        if (i == -1)
                            rhs = encode(get_number(rhs));
                        else {
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            rhs = decode_raw(rhs);
                        }
                    } else
                        rhs = encode(parse_number(rhs));
                    
                    return encode(lhs + rhs);
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            double num = get_number(lhs);
                            
                            if (rhs.empty() || is_string(rhs))
                                type_error(string_t, number_t);
                                //  string != double
                            
                            if (ss::is_array(rhs))
                                type_error(array_t, number_t);
                                //  array != double
                            
                            if (is_key(rhs)) {
                                if (io_array(rhs) != -1)
                                    type_error(array_t, number_t);
                                    //  array != double
                                
                                if (io_string(rhs) != -1)
                                    type_error(string_t, number_t);
                                    //  string != double
                                
                                num += get_number(rhs);
                            } else
                                num += parse_number(rhs);
                            
                            return encode(num);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        lhs = decode_raw(lhs);
                        
                        if (ss::is_array(rhs))
                            type_error(array_t, string_t);
                            //  array != string
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs))
                            rhs = decode(rhs);
                        
                        else if (is_key(rhs)) {
                            int j = io_string(rhs);
                            
                            if (j == -1)
                                rhs = encode(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode_raw(rhs);
                            }
                        } else
                            rhs = encode(parse_number(rhs));
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return encode(lhs + rhs);
                    }
                    
                    if (!rhs.empty()) {
                        if (!ss::is_array(rhs)) {
                            if (is_string(rhs))
                                rhs = encode(decode(rhs));
                                
                            else if (is_key(rhs)) {
                                int j = io_array(rhs);
                                
                                if (j == -1) {
                                    j = io_string(rhs);
                                    
                                    if (j == -1)
                                        rhs = encode(get_number(rhs));
                                    else {
                                        std::get<2>(* stringv[j]).second = true;
                                        
                                        rhs = std::get<1>(* stringv[j]);
                                    }
                                } else {
                                    std::get<2>(* arrayv[j]).second = true;
                                    
                                    rhs = stringify(std::get<1>(* arrayv[j]));
                                }
                            } else
                                rhs = encode(parse_number(rhs));
                        }
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return stringify(std::get<1>(* arrayv[i])) + get_sep() + rhs;
                }
                
                double num = parse_number(lhs);
                
                if (ss::is_array(rhs))
                    type_error(array_t, number_t);
                    //  array != double
                
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, number_t);
                    //  string != double
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, number_t);
                        //  array != double
                    
                    if (io_string(rhs) != -1)
                        type_error(string_t, number_t);
                        //  string != double
                    
                    num += get_number(rhs);
                } else
                    num += parse_number(rhs);
                
                return encode(num);
            }
            
            if (!rhs.empty()) {
                if (!ss::is_array(rhs)) {
                    if (is_string(rhs))
                        rhs = encode(decode(rhs));
                        
                    else if (is_key(rhs)) {
                        int i = io_array(rhs);
                        
                        if (i == -1) {
                            i = io_string(rhs);
                            
                            if (i == -1)
                                rhs = encode(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                std::get<2>(* stringv[i]).second = true;
                            }
                        } else {
                            rhs = stringify(std::get<1>(* arrayv[i]));
                            
                            std::get<2>(* arrayv[i]).second = true;
                        }
                    } else
                        rhs = encode(parse_number(rhs));
                }
            }
            
            return lhs + get_sep() + rhs;
        });

        uov[uoc++] = new binary_universal_operator("-", [this](const string lhs, const string rhs) {
            unsupported_error("-");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("<<", [this](const string lhs, const string rhs) {
            unsupported_error("<<");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator(">>", [this](const string lhs, const string rhs) {
            unsupported_error(">>");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("max", [this](const string lhs, const string rhs) {
            unsupported_error("max");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("min", [this](const string lhs, const string rhs) {
            unsupported_error("min");
            return nullptr;
        });
        
        uov[aggregate_pos = uoc++] = new ternary_universal_operator("aggregate", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                    
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, array_t);
                        //  string != array
                    
                    if (is_defined(lhs))
                        type_error(number_t, array_t);
                        //  double != array
                    
                    undefined_error(lhs);
                }
                
                if (!is_key(ctr))
                    expect_error("key");
                
                size_t state = get_state();
                
                if (is_defined(ctr))
                    remove_key(ctr);
                
                valuec = std::get<1>(* arrayv[i]).size();
                valuev = new string[valuec];
                
                for (size_t j = 0; j < valuec; ++j)
                    valuev[j] = std::get<1>(* arrayv[i])[j];
                
                set_array(ctr, 0, valuev[0]);
                
                for (size_t j = 1; j < valuec; ++j) {
                    set_array(ctr, 1, valuev[j]);
                    
                    string result = evaluate(rhs);
                    
                    if (ss::is_array(result))
                        type_error(array_t, item_t);
                    
                    set_array(ctr, 0, result);
                }
                
                delete[] valuev;
                
                string result;
                
                result = std::get<1>(* arrayv[io_array(ctr)])[0];
                
                remove_key(ctr);
                
                set_state(state);
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                return result;
            }
            
            if (!is_key(ctr))
                expect_error("key");;
            
            size_t state = get_state();
            
            if (is_defined(ctr))
                remove_key(ctr);
            
            set_array(ctr, 0, valuev[0]);
            
            for (size_t i = 1; i < valuec; ++i) {
                set_array(ctr, 1, valuev[i]);
                
                string result = evaluate(rhs);
                
                if (ss::is_array(result))
                    type_error(array_t, item_t);
                
                set_array(ctr, 0, result);
            }
            
            string result;
            
            result = std::get<1>(* arrayv[io_array(ctr)])[0];
            
            remove_key(ctr);
            
            set_state(state);
            
            return result;
        });
        
        uov[cell_pos = uoc++] = new ternary_universal_operator("cell", [this](const string lhs, const string ctr, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, table_t);
                
                if (!is_key(lhs))
                    type_error(number_t, table_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, table_t);
                        
                    if (is_defined(lhs))
                        type_error(number_t, table_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(array_t, table_t);
                    //  array != table
                
                if (ss::is_array(ctr))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (ctr.empty()|| is_string(ctr))
                    type_error(string_t, int_t);
                    //  string != int
                
                double y = stod(ctr);
                
                if (!is_int(y))
                    type_error(number_t, int_t);
                    //  double != int
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (y < 0 || y >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                    range_error("index " + encode(y) + ", rows " + std::to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                
                if (ss::is_array(rhs))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, int_t);
                    //  string != int
                
                double x = stod(rhs);
                
                if (!is_int(x))
                    type_error(number_t, int_t);
                    //  double != int
                
                if (x < 0 || x >= c)
                    range_error("index " + encode(x) + ", cols " + std::to_string(c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::get<1>(* arrayv[i])[(size_t)(y * c + x + 1)];
            }
            
            if (!is_table(valuec, valuev))
                type_error(array_t, table_t);
                //  array != table
            
            if (ss::is_array(ctr))
                type_error(array_t, int_t);
                //  array != int
            
            if (rhs.empty() || is_string(rhs))
                type_error(string_t, int_t);
                //  string != int
            
            double y = stod(ctr);
            
            if (!is_int(y))
                type_error(number_t, int_t);
            
            size_t c = stoi(valuev[0]);
            
            if (y < 0 || y >= (valuec - 1) / c)
                range_error("index " + encode(y) + ", rows " + std::to_string((valuec - 1) / c));
            
            if (ss::is_array(rhs))
                type_error(array_t, int_t);
                //  array != int
            
            if (rhs.empty() || is_string(rhs))
                type_error(string_t, int_t);
                //  string != int
            
            double x = stod(rhs);
            
            if (!is_int(x))
                type_error(number_t, int_t);
            
            if (x < 0 || x >= c)
                range_error("index " + encode(x) + ", cols " + std::to_string(c));
            
            string result = valuev[(size_t)(y * c + x + 1)];
            
            return result;
        });
        
        uov[col_pos = uoc++] = new binary_universal_operator("col", [this](string lhs, string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, table_t);
                
                if  (!is_key(lhs))
                    type_error(number_t, table_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, table_t);
                    
                    if (is_defined(lhs))
                        type_error(number_t, table_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(array_t, table_t);
                
                if (ss::is_array(rhs))
                    type_error(array_t, item_t);
                
                if (rhs.empty())
                    null_error();
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (is_string(rhs)) {
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = encode(decode(rhs));
                    
                    size_t j = 0;
                    while (j < c && std::get<1>(* arrayv[i])[j + 1] != rhs)
                        ++j;
                    
                    if (j == c)
                        return null();
                        //  null
                    
                    stringstream ss;
                    
                    if ((std::get<1>(* arrayv[i]).size() - 1) / c != 1) {
                        size_t k;
                        for (k = 1; k < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[k * c + j + 1] << get_sep();
                        
                        ss << std::get<1>(* arrayv[i])[k * c + j + 1];
                    }
                    
                    return ss.str();
                }
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, item_t);
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double idx = get_number(rhs);
                        
                        if (!is_int(idx))
                            type_error(number_t, int_t);
                        
                        if (idx < 0 || idx >= c)
                            range_error("index " + encode(idx) + ", cols " + std::to_string(c));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        stringstream ss;
                        
                        size_t k;
                        for (k = 0; k < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[k * c + idx + 1] << get_sep();
                        
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
                        return null();
                        //  null
                    
                    stringstream ss;
                    
                    if ((std::get<1>(* arrayv[i]).size() - 1) / c != 1) {
                        size_t l;
                        for (l = 1; l < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++l)
                            ss << std::get<1>(* arrayv[i])[l * c + k + 1] << get_sep();
                        
                        ss << std::get<1>(* arrayv[i])[l * c + k + 1];
                    }
                    
                    return ss.str();
                }
                
                double idx = parse_number(rhs);
                
                if (!is_int(idx))
                    type_error(number_t, int_t);
                
                if (idx < 0 || idx >= c)
                    range_error("index " + encode(idx) + ", cols " + std::to_string(c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                
                size_t j;
                for (j = 0; j < (std::get<1>(* arrayv[i]).size() - 1) / c - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[j * c + idx + 1] << get_sep();
                
                ss << std::get<1>(* arrayv[i])[j * c + idx + 1];
                
                return ss.str();
            }
            
            if (!is_table(valuec, valuev)) {
                delete[] valuev;
                type_error(array_t, table_t);
            }
            
            if (ss::is_array(rhs)) {
                delete[] valuev;
                type_error(array_t, item_t);
            }
            
            if (rhs.empty()) {
                delete[] valuev;
                null_error();
            }
            
            size_t c = stoi(valuev[0]);
            
            if (is_string(rhs)) {
                rhs = encode(decode(rhs));
                
                size_t i = 0;
                while (i < c && valuev[i + 1] != rhs)
                    ++i;
                
                if (i == c) {
                    delete[] valuev;
                    return null();
                    //  null
                }
                
                stringstream ss;
                
                if ((valuec - 1) / c != 1) {
                    size_t j;
                    for (j = 1; j < (valuec - 1) / c - 1; ++j)
                        ss << valuev[j * c + i + 1] << get_sep();
                    
                    ss << valuev[j * c + i + 1];
                }
                
                return ss.str();
            }
            
            if (is_key(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] valuev;
                    type_error(array_t, item_t);
                }
                
                int i = io_string(rhs);

                if (i == -1) {
                    double idx = get_number(rhs);
                    
                    if (!is_int(idx)) {
                        delete[] valuev;
                        type_error(number_t, int_t);
                    }
                    
                    if (idx < 0 || idx >= c) {
                        delete[] valuev;
                        range_error("index " + encode(idx) + ", cols " + std::to_string(c));
                    }
                    
                    stringstream ss;
                    
                    size_t j;
                    for (j = 0; j < (valuec - 1) / c - 1; ++j)
                        ss << valuev[j * c + (size_t)idx + 1] << get_sep();
                    
                    ss << valuev[j * c + (size_t)idx + 1];
                    
                    return ss.str();
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty()) {
                    delete[] valuev;
                    null_error();
                }
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < c && valuev[j + 1] != rhs)
                    ++j;
                
                if (j == c) {
                    delete[] valuev;
                    return null();
                }
                
                stringstream ss;
                
                if ((valuec - 1) / c != 1) {
                    size_t k;
                    for (k = 1; k < (valuec - 1) / c - 1; ++k)
                        ss << valuev[k * c + j + 1] << get_sep();
                    
                    ss << valuev[k * c + j + 1];
                }
                
                return ss.str();
            }
            
            double idx = parse_number(rhs);
            
            if (!is_int(idx)) {
                delete[] valuev;
                type_error(number_t, int_t);
            }
            
            if (idx < 0 || idx >= c) {
                delete[] valuev;
                range_error("index " + encode(idx) + ", cols " + std::to_string(c));
            }
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < (valuec - 1) / c - 1; ++i)
                ss << valuev[i * c + (size_t)idx + 1] << get_sep();
            
            ss << valuev[i * c + (size_t)idx + 1];
            
            delete[] valuev;
            
            return ss.str();
        });
        
        uov[contains_pos = uoc++] = new binary_universal_operator("contains", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    string text = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, string_t);
                    
                    if (rhs.empty())
                        null_error();
                        
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (text.length() < pattern.length())
                            return std::to_string(0);
                        
                        size_t i;
                        for (i = 0; i <= text.length() - pattern.length(); ++i) {
                            size_t j = 0;
                            while (j < pattern.length() && text[i + j] == pattern[j])
                                ++j;
                            
                            if (j == pattern.length())
                                break;
                        }
                        
                        return std::to_string(i != text.length() - pattern.length() + 1);
                    }
                    
                    if (!is_key(rhs))
                        type_error(number_t, string_t);
                    
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(number_t, string_t);
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[i]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode_raw(pattern);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (text.length() < pattern.length())
                        return std::to_string(0);
                    
                    size_t j;
                    for (j = 0; j <= text.length() - pattern.length(); ++j) {
                        size_t k = 0;
                        while (k < pattern.length() && text[j + k] == pattern[k])
                            ++k;
                        
                        if (k == pattern.length())
                            break;
                    }
                    
                    return std::to_string(j != text.length() - pattern.length() + 1);
                }
                
                if (!is_key(lhs))
                    type_error(number_t, string_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (is_defined(lhs))
                                type_error(number_t, string_t);
                            
                            undefined_error(lhs);
                        }
                        
                        string text = std::get<1>(* stringv[i]);
                        
                        if (text.empty())
                            null_error();
                        
                        text = decode_raw(text);
                        
                        if (ss::is_array(rhs))
                            type_error(array_t, string_t);
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            string pattern = decode(rhs);
                            
                            if (text.length() < pattern.length())
                                return std::to_string(0);
                            
                            size_t j;
                            for (j = 0; j <= text.length() - pattern.length(); ++j) {
                                size_t k = 0;
                                while (k < pattern.length() && text[j + k] == pattern[k])
                                    ++k;
                                
                                if (k == pattern.length())
                                    break;
                            }
                            
                            return std::to_string(j != text.length() - pattern.length() + 1);
                        }
                        
                        if (!is_key(rhs))
                            type_error(number_t, string_t);
                        
                        if (io_array(rhs) != -1)
                            type_error(array_t, string_t);
                        
                        int j = io_string(rhs);
                        if (j == -1) {
                            if (is_defined(rhs))
                                type_error(number_t, string_t);
                            
                            undefined_error(rhs);
                        }
                        
                        string pattern = std::get<1>(* stringv[j]);
                        
                        if (pattern.empty())
                            null_error();
                        
                        pattern = decode_raw(pattern);
                        
                        std::get<2>(* stringv[i]).second = true;
                        std::get<2>(* stringv[j]).second = true;
                        
                        if (text.length() < pattern.length())
                            return std::to_string(0);
                        
                        size_t k;
                        for (k = 0; k <= text.length() - pattern.length(); ++k) {
                            size_t l = 0;
                            while (l < pattern.length() && text[k + l] == pattern[l])
                                ++l;
                            
                            if (l == pattern.length())
                                break;
                        }
                        
                        return std::to_string(k != text.length() - pattern.length() + 1);
                    }
                }
                
                string pattern = element(rhs);
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::to_string(std::get<1>(* arrayv[i]).index_of(pattern));
            }
            string pattern = element(rhs);
            
            size_t i = 0;
            while (i < valuec && valuev[i] != pattern)
                ++i;
            
            return std::to_string(i != valuec);
        });
        
        uov[fill_pos = uoc++] = new binary_universal_operator("fill", [this](string lhs, string rhs) {
            if (lhs.empty())
                type_error(string_t, array_t);
                //  string != array
            
            string* v = new string[lhs.length() + 1];
            size_t n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    type_error(string_t, array_t);
                    //  string != array
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                    //  double != array
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, array_t);
                            //  double != array
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(string_t, array_t);
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
                    type_error(array_t, string_t);
                    //  array != string
                
                if (is_string(rhs))
                    rhs = encode(decode(rhs));
                    
                else if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                        //  array != string
                    
                    int j = io_string(rhs);
                    if (j == -1)
                        rhs = encode(get_number(rhs));
                    else {
                        rhs = std::get<1>(* stringv[j]);
                        
                        std::get<2>(* stringv[j]).second = true;
                    }
                } else
                    rhs = encode(parse_number(rhs));
            }
            
            for (size_t j = 0; j < n; ++j)
                v[j] = rhs;
            
            rhs = stringify(n, v);
            
            delete[] v;
            
            return rhs;
        });
        
        uov[filter_pos = uoc++] = new ternary_universal_operator("filter", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, array_t);
                    
                    if (is_defined(lhs))
                        type_error(number_t, array_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_key(ctr))
                    expect_error("key");
                
                size_t state = get_state();
                
                if (is_defined(ctr))
                    remove_key(ctr);
                
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
                    
                    remove_key(ctr);
                    
                    if (ss::evaluate(result))
                        ++j;
                    else {
                        for (size_t k = j; k < valuec - 1; ++k)
                            swap(valuev[k], valuev[k + 1]);
                        
                        --valuec;
                    }
                }
                
                set_state(state);
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                string result = stringify(valuec, valuev);
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_key(ctr))
                expect_error("key");
            
            size_t state = get_state();
            
            if (is_defined(ctr))
                remove_key(ctr);
            
            size_t i = 0;
            while (i < valuec) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                remove_key(ctr);
                
                if (ss::evaluate(result))
                    ++i;
                else {
                    for (size_t j = i; j < valuec - 1; ++j)
                        swap(valuev[j], valuev[j + 1]);
                    
                    --valuec;
                }
            }
            
            set_state(state);
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        uov[find_pos = uoc++] = new ternary_universal_operator("find", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, array_t);
                    
                    if (is_defined(lhs))
                        type_error(number_t, array_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_key(ctr))
                    expect_error("key");
                
                size_t state = get_state();
                
                if (is_defined(ctr))
                    remove_key(ctr);
                
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
                    
                    remove_key(ctr);
                    
                    if (ss::evaluate(result))
                        break;
                }
                
                set_state(state);
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                if (j == valuec) {
                    delete[] valuev;
                    return null();
                }
                
                result = valuev[j];
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_key(ctr)) {
                delete[] valuev;
                expect_error("key");
            }
            
            size_t state = get_state();
            
            if (is_defined(ctr))
                remove_key(ctr);
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                remove_key(ctr);
                
                if (ss::evaluate(result))
                    break;
            }
            
            if (i == valuec) {
                delete[] valuev;
                return null();
            }
            
            set_state(state);
            
            string result = valuev[i];
            
            delete[] valuev;
            
            return result;
        });
        
        uov[find_index_pos = uoc++] = new ternary_universal_operator("findindex", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, array_t);
                    
                    if (is_defined(lhs))
                        type_error(number_t, array_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_key(ctr))
                    operation_error();
                
                size_t state = get_state();
                
                if (is_defined(ctr))
                    remove_key(ctr);
                
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
                    
                    remove_key(ctr);
                    
                    if (ss::evaluate(result))
                        break;
                }
                
                set_state(state);
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                if (j == valuec) {
                    delete[] valuev;
                    return null();
                }
                
                result = valuev[j];
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_key(ctr)) {
                delete[] valuev;
                operation_error();
            }
            
            size_t state = get_state();
            
            if (is_defined(ctr))
                remove_key(ctr);
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                remove_key(ctr);
                
                if (ss::evaluate(result))
                    break;
            }
            
            set_state(state);
            
            if (i == valuec) {
                delete[] valuev;
                return std::to_string(-1);
            }
            
            delete[] valuev;
            
            return std::to_string(i);
        });
        
        uov[format_pos = uoc++] = new binary_universal_operator("format", [this](const string lhs, const string rhs) {
            if (ss::is_array(lhs))
                type_error(array_t, string_t);
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
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                size_t i = 0;
                                while (i < text.length() - 1 && (text[i] != '{' || text[i + 1] != '}'))
                                    ++i;
                                
                                if (i == text.length() - 1)
                                    return encode(text);
                                
                                string pattern = encode(get_number(rhs));
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
                            
                            pattern = pattern.empty() ? "null" : decode_raw(pattern);
                            
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
                                    pattern = decode_raw(pattern);
                                
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
                    
                    string pattern = encode(parse_number(rhs));
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
            
            if (!is_key(lhs))
                type_error(number_t, string_t);
                //  double != string
            
            if (io_array(lhs) != -1)
                type_error(array_t, string_t);
                //  array != string
            
            int i = io_string(lhs);
            if (i == -1) {
                if (is_defined(lhs))
                    type_error(number_t, string_t);
                    //  double != string
                
                undefined_error(lhs);
            }
            
            std::get<2>(* stringv[i]).second = true;
            
            string text = std::get<1>(* stringv[i]);
            
            if (text.empty())
                null_error();
            
            text = decode_raw(text);
            
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
                
                if (is_key(rhs)) {
                    int j = io_array(rhs);
                    if (j == -1) {
                        j = io_string(rhs);
                        if (j == -1) {
                            size_t k = 0;
                            while (k < text.length() - 1 && (text[k] != '{' || text[k + 1] != '}'))
                                ++k;
                            
                            if (k == text.length() - 1)
                                return encode(text);
                            
                            string pattern = encode(get_number(rhs));
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
                        
                        pattern = pattern.empty() ? "null" : decode_raw(pattern);
                        
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
                                pattern = decode_raw(pattern);
                            
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
                
                string pattern = encode(parse_number(rhs));
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
        });
        
        uov[index_of_pos = uoc++] = new binary_universal_operator("indexof", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    string text = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, string_t);
                    
                    if (rhs.empty())
                        null_error();
                        
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (text.length() < pattern.length())
                            return std::to_string(-1);
                        
                        int i;
                        for (i = 0; i <= text.length() - pattern.length(); ++i) {
                            size_t j = 0;
                            while (j < pattern.length() && text[i + j] == pattern[j])
                                ++j;
                            
                            if (j == pattern.length())
                                break;
                        }
                        
                        return std::to_string(i == text.length() - pattern.length() + 1 ? -1 : i);
                    }
                    
                    if (!is_key(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                    
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                        //  array != string
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(number_t, string_t);
                            //  double != string
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[i]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode_raw(pattern);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (text.length() < pattern.length())
                        return std::to_string(-1);
                    
                    int j;
                    for (j = 0; j <= text.length() - pattern.length(); ++j) {
                        size_t k = 0;
                        while (k < pattern.length() && text[j + k] == pattern[k])
                            ++k;
                        
                        if (k == pattern.length())
                            break;
                    }
                    
                    return std::to_string(j == text.length() - pattern.length() + 1 ? -1 : j);
                }
                
                if (!is_key(lhs))
                    type_error(number_t, string_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (is_defined(lhs))
                                type_error(number_t, string_t);
                            
                            undefined_error(lhs);
                        }
                        
                        string text = std::get<1>(* stringv[i]);
                        
                        if (text.empty())
                            null_error();
                        
                        text = decode_raw(text);
                        
                        if (ss::is_array(rhs))
                            type_error(array_t, string_t);
                            //  array != string
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            string pattern = decode(rhs);
                            
                            if (text.length() < pattern.length())
                                return std::to_string(-1);
                            
                            int j;
                            for (j = 0; j <= text.length() - pattern.length(); ++j) {
                                size_t k = 0;
                                while (k < pattern.length() && text[j + k] == pattern[k])
                                    ++k;
                                
                                if (k == pattern.length())
                                    break;
                            }
                            
                            return std::to_string(j == text.length() - pattern.length() + 1 ? -1 : j);
                        }
                        
                        if (!is_key(rhs))
                            type_error(number_t, string_t);
                            //  double != string
                        
                        if (io_array(rhs) != -1)
                            type_error(array_t, string_t);
                            //  array != string
                        
                        int j = io_string(rhs);
                        if (j == -1) {
                            if (is_defined(rhs))
                                type_error(number_t, string_t);
                                //  double != string
                            
                            undefined_error(rhs);
                        }
                        
                        string pattern = std::get<1>(* stringv[j]);
                        
                        if (pattern.empty())
                            null_error();
                        
                        pattern = decode_raw(pattern);
                        
                        std::get<2>(* stringv[i]).second = true;
                        std::get<2>(* stringv[j]).second = true;
                        
                        if (text.length() < pattern.length())
                            return std::to_string(-1);
                        
                        int k;
                        for (k = 0; k <= text.length() - pattern.length(); ++k) {
                            size_t l = 0;
                            while (l < pattern.length() && text[k + l] == pattern[l])
                                ++l;
                            
                            if (l == pattern.length())
                                break;
                        }
                        
                        return std::to_string(k == text.length() - pattern.length() + 1 ? -1 : k);
                    }
                }
                
                string pattern = element(rhs);
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::to_string(std::get<1>(* arrayv[i]).index_of(pattern));
            }
            
            string pattern = element(rhs);
            
            int i = 0;
            while (i < valuec && valuev[i] != pattern)
                ++i;
            
            return std::to_string(i == valuec ? -1 : i);
        });
        
        uov[insert_pos = uoc++] = new ternary_universal_operator("insert", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, array_t);
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(string_t, array_t);
                }
                
                if (ss::is_array(ctr))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (ctr.empty() || is_string(ctr))
                    type_error(string_t, int_t);
                    //  string != int
                
                double idx = stod(ctr);
                
                if (!is_int(idx))
                    type_error(number_t, int_t);
                    //  double != int
                
                if (idx < 0 || idx > std::get<1>(* arrayv[i]).size())
                    range_error("index " + encode(idx) + ", count " + std::to_string(std::get<1>(* arrayv[i]).size()));
                
                if (std::get<2>(* arrayv[i]).first)
                    write_error(lhs);
                
                valuev = new string[rhs.length() + 1];
                valuec = parse(valuev, rhs);
                
                std::get<1>(* arrayv[i]).ensure_capacity(std::get<1>(* arrayv[i]).size() + valuec);
                
                for (size_t l = 0; l < valuec; ++l)
                    std::get<1>(* arrayv[i]).insert((size_t)idx + l, valuev[l]);
                
                delete[] valuev;
                
                return std::to_string(std::get<1>(* arrayv[i]).size());
            }
            
            if (ss::is_array(ctr))
                type_error(array_t, int_t);
                //  array != int
            
            if (ctr.empty() || is_string(ctr))
                type_error(string_t, int_t);
                //  string != int
            
            double idx = stod(ctr);
                
            if (!is_int(idx))
                type_error(number_t, int_t);
                //  double != int
            
            if (idx < 0 || idx > valuec)
                range_error(encode(idx));
            
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
        
        uov[join_pos = uoc++] = new binary_universal_operator("join", [this](string lhs, string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                    //  string != array
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                    //  double != array
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, array_t);
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(string_t, array_t);
                }
                
                if (ss::is_array(rhs))
                    type_error(array_t, string_t);
                    //  array != string
                
                if (rhs.empty())
                    null_error();
                
                if (is_string(rhs))
                    rhs = decode(rhs);
                
                else if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                        //  array != string
                    
                    int j = io_string(rhs);
                    if (j == -1)
                        rhs = encode(get_number(rhs));
                    else {
                        rhs = std::get<1>(* stringv[j]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        rhs = decode_raw(rhs);
                        
                        std::get<2>(* stringv[j]).second = true;
                    }
                } else
                    rhs = encode(parse_number(rhs));
                
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
                type_error(array_t, string_t);
                //  array != string
            
            if (rhs.empty())
                null_error();
            
            if (is_string(rhs))
                rhs = decode(rhs);
            
            else if (is_key(rhs)) {
                int i = io_array(rhs);
                if (i != -1)
                    type_error(array_t, string_t);
                    //  array != string
                
                i = io_string(rhs);
                if (i == -1)
                    rhs = encode(get_number(rhs));
                else {
                    rhs = std::get<1>(* stringv[i]);
                    
                    if (rhs.empty())
                        null_error();
                    
                    rhs = decode_raw(rhs);
                    
                    std::get<2>(* stringv[i]).second = true;
                }
            } else
                rhs = encode(parse_number(rhs));
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < valuec - 1; ++i) {
                if (valuev[i].empty())
                    null_error();
                
                ss << decode_raw(valuev[i]) << rhs;
            }
            
            if (valuev[i].empty())
                null_error();
            
            ss << decode_raw(valuev[i]);
            
            return encode(ss.str());
        });
        
        uov[last_index_of_pos = uoc++] = new binary_universal_operator("lastindexof", [this](const string lhs, const string rhs) {
            string valuev[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                if (lhs.empty())
                    null_error();
                
                if (is_string(lhs)) {
                    string text = decode(lhs);
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, string_t);
                        //  array != string_t
                    
                    if (rhs.empty())
                        null_error();
                    
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (text.length() < pattern.length())
                            return std::to_string(-1);
                        
                        int i;
                        for (i = (int)text.length() - (int)pattern.length(); i >= 0; --i) {
                            size_t j = 0;
                            while (j < pattern.length() && text[i + j] == pattern[j])
                                ++j;
                            
                            if (j == pattern.length())
                                break;
                        }
                        
                        return std::to_string(i);
                    }
                    
                    if (!is_key(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                    
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                        //  array != string
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            type_error(number_t, string_t);
                            //  double != string
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[i]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode_raw(pattern);
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    if (text.length() < pattern.length())
                        return std::to_string(-1);
                    
                    int j;
                    for (j = (int)text.length() - (int)pattern.length(); j >= 0; --j) {
                        size_t k = 0;
                        while (k < pattern.length() && text[j + k] == pattern[k])
                            ++k;
                        
                        if (k == pattern.length())
                            break;
                    }
                    
                    return std::to_string(j);
                }
                
                if (!is_key(lhs))
                    type_error(number_t, string_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, string_t);
                        
                        undefined_error(lhs);
                    }
                    
                    string text = std::get<1>(* stringv[i]);
                    
                    if (text.empty())
                        null_error();
                    
                    std::get<2>(* stringv[i]).second = true;
                    
                    text = decode_raw(text);
                    
                    if (ss::is_array(rhs))
                        type_error(array_t, string_t);
                        //  array != string
                    
                    if (rhs.empty())
                        null_error();
                    
                    if (is_string(rhs)) {
                        string pattern = decode(rhs);
                        
                        if (text.length() < pattern.length())
                            return std::to_string(-1);
                        
                        int j;
                        for (j = (int)text.length() - (int)pattern.length(); j >= 0; --j) {
                            size_t k = 0;
                            while (k < pattern.length() && text[j + k] == pattern[k])
                                ++k;
                            
                            if (k == pattern.length())
                                break;
                        }
                        
                        return std::to_string(j);
                    }
                    
                    if (!is_key(rhs))
                        type_error(number_t, string_t);
                        //  double != string
                    
                    if (io_array(rhs) != -1)
                        type_error(array_t, string_t);
                        //  array != string
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        if (is_defined(rhs))
                            type_error(number_t, string_t);
                            //  double != string
                        
                        undefined_error(rhs);
                    }
                    
                    string pattern = std::get<1>(* stringv[j]);
                    
                    if (pattern.empty())
                        null_error();
                    
                    pattern = decode_raw(pattern);
                    
                    std::get<2>(* stringv[j]).second = true;
                    
                    if (text.length() < pattern.length())
                        return std::to_string(-1);
                    
                    int k;
                    for (k = (int)text.length() - (int)pattern.length(); k >= 0; --k) {
                        size_t l = 0;
                        while (l < pattern.length() && text[k + l] == pattern[l])
                            ++l;
                        
                        if (l == pattern.length())
                            break;
                    }
                    
                    return std::to_string(k);
                }
                
                string pattern = element(rhs);
                
                std::get<2>(* arrayv[i]).second = true;
                
                return std::to_string(std::get<1>(* arrayv[i]).last_index_of(pattern));
            }
            
            string pattern = element(rhs);
            
            int i = (int)valuec - 1;
            while (i >= 0 && valuev[i] != pattern)
                --i;
            
            return std::to_string(i);
        });
        
        uov[map_pos = uoc++] = new ternary_universal_operator("map", [this](const string lhs, const string ctr, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, array_t);
                
                if (!is_key(lhs))
                    type_error(number_t, array_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, array_t);
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(string_t, array_t);
                }
                
                if (!is_key(ctr))
                    expect_error("key");
                
                size_t state = get_state();
                
                if (is_defined(ctr))
                    remove_key(ctr);
                
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
                        type_error(array_t, item_t);
                    
                    remove_key(ctr);
                    
                    valuev[j] = result;
                }
                
                set_state(state);
                
                std::get<2>(* arrayv[io_array(lhs)]).second = true;
                
                string result = stringify(valuec, valuev);
                
                delete[] valuev;
                
                return result;
            }
            
            if (!is_key(ctr))
                expect_error("key");
            
            size_t state = get_state();
            
            if (is_defined(ctr))
                remove_key(ctr);
            
            size_t i;
            for (i = 0; i < valuec; ++i) {
                if (valuev[i].empty() || is_string(valuev[i]))
                    set_string(ctr, valuev[i]);
                else
                    set_number(ctr, stod(valuev[i]));
                
                string result = evaluate(rhs);
                
                if (ss::is_array(result)) {
                    delete[] valuev;
                    type_error(array_t, item_t);
                }
                
                remove_key(ctr);
                
                valuev[i] = result;
            }
            
            set_state(state);
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        uov[reserve_pos = uoc++] = new binary_universal_operator("reserve", [this](const string lhs, const string rhs) {
            if (!is_key(lhs))
                operation_error();
            
            int i = io_array(lhs);
            if (i == -1) {
                i = io_string(lhs);
                if (i == -1) {
                    if (is_defined(lhs))
                        type_error(number_t, array_t);
                    
                    undefined_error(lhs);
                }
                
                type_error(string_t, array_t);
            }
            
            if (ss::is_array(rhs))
                type_error(array_t, int_t);
                //  array != int
            
            if (is_string(rhs))
                type_error(string_t, int_t);
                //  string != int
            
            double num;
            
            if (is_key(rhs)) {
                if (io_array(rhs) != -1)
                    type_error(array_t, int_t);
                    //  array != int
                
                if (io_string(rhs) != -1)
                    type_error(string_t, int_t);
                    //  string != int
                
                num = get_number(rhs);
            } else
                num = parse_number(rhs);
            
            if (!is_int(num))
                type_error(number_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            if (std::get<2>(* arrayv[i]).first)
                write_error(lhs);
            
            string result = std::to_string(std::get<1>(* arrayv[i]).capacity());
            
            std::get<1>(* arrayv[i]).ensure_capacity((size_t)num);
            
            return result;
        });
        //  19
        
        uov[row_pos = uoc++] = new binary_universal_operator("row", [this](string lhs, string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (lhs.empty() || is_string(lhs))
                    type_error(string_t, table_t);
                
                if  (!is_key(lhs))
                    type_error(number_t, table_t);
                
                int i = io_array(lhs);
                if (i == -1) {
                    if (io_string(lhs) != -1)
                        type_error(string_t, table_t);
                    
                    if (is_defined(lhs))
                        type_error(number_t, table_t);
                    
                    undefined_error(lhs);
                }
                
                if (!is_table(std::get<1>(* arrayv[i])))
                    type_error(array_t, table_t);
                
                if (ss::is_array(rhs))
                    type_error(array_t, item_t);
                
                if (rhs.empty())
                    null_error();
                
                size_t c = stoi(std::get<1>(* arrayv[i])[0]);
                
                if (is_string(rhs)) {
                    std::get<2>(* arrayv[i]).second = true;
                    
                    rhs = encode(decode(rhs));
                    
                    size_t j = 0;
                    while (j < (std::get<1>(* arrayv[i]).size() - 1) / c && std::get<1>(* arrayv[i])[j * c + 1] != rhs)
                        ++j;
                    
                    if (j == (std::get<1>(* arrayv[i]).size() - 1) / c)
                        return null();
                    
                    stringstream ss;
                    
                    if (c != 1) {
                        size_t k;
                        for (k = 1; k < c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[j * c + k + 1] << get_sep();
                        
                        ss << std::get<1>(* arrayv[i])[j * c + k + 1];
                    }
                    
                    return ss.str();
                }
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, item_t);
                    
                    int j = io_string(rhs);
                    if (j == -1) {
                        double idx = get_number(rhs);
                        
                        if (!is_int(idx))
                            type_error(number_t, int_t);
                        
                        if (idx < 0 || idx >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                            range_error("index " + encode(idx) + ", rows " + std::to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        stringstream ss;
                        
                        size_t k;
                        for (k = 0; k < c - 1; ++k)
                            ss << std::get<1>(* arrayv[i])[idx * c + k + 1] << get_sep();
                        
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
                        return null();
                    
                    stringstream ss;
                    
                    if (c != 1) {
                        size_t l;
                        for (l = 1; l < c - 1; ++l)
                            ss << std::get<1>(* arrayv[i])[k * c + l + 1] << get_sep();
                        
                        ss << std::get<1>(* arrayv[i])[k * c + l + 1];
                    }
                    
                    return ss.str();
                }
                
                double idx = parse_number(rhs);
                
                if (!is_int(idx))
                    type_error(number_t, int_t);
                
                if (idx < 0 || idx >= (std::get<1>(* arrayv[i]).size() - 1) / c)
                    range_error("index " + encode(idx) + ", rows " + std::to_string((std::get<1>(* arrayv[i]).size() - 1) / c));
                
                std::get<2>(* arrayv[i]).second = true;
                
                stringstream ss;
                
                size_t j;
                for (j = 0; j < c - 1; ++j)
                    ss << std::get<1>(* arrayv[i])[idx * c + j + 1] << get_sep();
                
                ss << std::get<1>(* arrayv[i])[idx * c + j + 1];
                
                return ss.str();
            }
            
            if (!is_table(valuec, valuev)) {
                delete[] valuev;
                type_error(array_t, table_t);
            }
            
            if (ss::is_array(rhs)) {
                delete[] valuev;
                type_error(array_t, item_t);
            }
            
            if (rhs.empty()) {
                delete[] valuev;
                null_error();
            }
            
            size_t c = stoi(valuev[0]);
            
            if (is_string(rhs)) {
                rhs = encode(decode(rhs));
                
                size_t i = 0;
                while (i < (valuec - 1) / c && valuev[i * c + 1] != rhs)
                    ++i;
                
                if (i == (valuec - 1) / c) {
                    delete[] valuev;
                    return null();
                }
                
                stringstream ss;
                
                if (c != 1) {
                    size_t j;
                    for (j = 1; j < c - 1; ++j)
                        ss << valuev[i * c + j + 1] << get_sep();
                    
                    ss << valuev[i * c + j + 1];
                }
                
                return ss.str();
            }
            
            if (is_key(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] valuev;
                    type_error(array_t, item_t);
                }
                
                int i = io_string(rhs);
                if (i == -1) {
                    double idx = get_number(rhs);
                    
                    if (!is_int(idx)) {
                        delete[] valuev;
                        type_error(number_t, int_t);
                    }
                    
                    if (idx < 0 || idx >= (valuec - 1) / c) {
                        delete[] valuev;
                        range_error("index " + encode(idx) + ", rows " + std::to_string((valuec - 1) / c));
                    }
                    
                    stringstream ss;
                    
                    size_t j;
                    for (j = 0; j < c - 1; ++j)
                        ss << valuev[(size_t)idx * c + j + 1] << get_sep();
                    
                    ss << valuev[(size_t)idx * c + j + 1];
                    
                    return ss.str();
                }
                
                rhs = std::get<1>(* stringv[i]);
                
                if (rhs.empty()) {
                    delete[] valuev;
                    null_error();
                }
                
                std::get<2>(* stringv[i]).second = true;
                
                size_t j = 0;
                while (j < (valuec - 1) / c && valuev[j * c + 1] != rhs)
                    ++j;
                
                if (j == (valuec - 1) / c) {
                    delete[] valuev;
                    return null();
                }
                
                stringstream ss;
                
                if (c != 1) {
                    size_t k;
                    for (k = 1; k < c - 1; ++k)
                        ss << valuev[j * c + k + 1] << get_sep();
                    
                    ss << valuev[j * c + k + 1];
                }
                
                return ss.str();
            }
            
            double idx = parse_number(rhs);
            
            if (!is_int(idx)) {
                delete[] valuev;
                type_error(number_t, int_t);
            }
            
            if (idx < 0 || idx >= (valuec - 1) / c) {
                delete[] valuev;
                range_error("index " + encode(idx) + ", rows " + std::to_string((valuec - 1) / c));
            }
            
            stringstream ss;
            
            size_t i;
            for (i = 0; i < c - 1; ++i)
                ss << valuev[(size_t)idx * c + i + 1] << get_sep();
            
            ss << valuev[(size_t)idx * c + i + 1];
            
            delete[] valuev;
            
            return ss.str();
        });
        
        uov[resize_pos = uoc++] = new binary_universal_operator("setcount", [this](const string lhs, const string rhs) {
            string* valuev = new string[lhs.length() + 1];
            size_t valuec = parse(valuev, lhs);
            
            if (valuec == 1) {
                delete[] valuev;
                
                if (!is_key(lhs))
                    operation_error();
                
                int i = io_array(lhs);
                if (i == -1) {
                    i = io_string(lhs);
                    if (i == -1) {
                        if (is_defined(lhs))
                            type_error(number_t, array_t);
                        
                        undefined_error(lhs);
                    }
                    
                    type_error(string_t, array_t);
                }
                
                if (is_array(rhs))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (rhs.empty() || is_string(rhs))
                    type_error(string_t, int_t);
                    //  string != int
                
                double num;
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        type_error(array_t, int_t);
                        //  array != int
                    
                    if (io_string(rhs) != -1)
                        type_error(string_t, int_t);
                        //  string != int
                    
                    num = get_number(rhs);
                } else
                    num = parse_number(rhs);
                
                if (is_int(num))
                    type_error(number_t, int_t);
                    //  double != int
                
                if (num < 0)
                    range_error(std::to_string((int)num));
                
                string result = std::to_string(std::get<1>(* arrayv[i]).size());
                
                if (std::get<2>(*arrayv[i]).first)
                    write_error(lhs);
                
                std::get<1>(* arrayv[i]).resize((int)num);
                
                return result;
            }
            
            if (is_array(rhs)) {
                delete[] valuev;
                type_error(array_t, int_t);
                //  array != int
            }
            
            if (rhs.empty() || is_string(rhs)) {
                delete[] valuev;
                type_error(string_t, int_t);
                //  string != int
            }
            
            double num;
            if (is_key(rhs)) {
                if (io_array(rhs) != -1) {
                    delete[] valuev;
                    type_error(array_t, int_t);
                    //  array != int
                }
                
                if (io_string(rhs) != -1) {
                    delete[] valuev;
                    type_error(string_t, int_t);
                    //  string != int
                }
                
                num = get_number(rhs);
            } else
                num = parse_number(rhs);
            
            if (!is_int(num)) {
                delete[] valuev;
                type_error(number_t, int_t);
                //  double != int
            }
            
            if (num < 0) {
                delete[] valuev;
                range_error(std::to_string((int)num));
            }
            
            while (valuec > num)
                --valuec;
            
            while (valuec < num) {
                if (valuec == lhs.length() + 1) {
                    string* tmp = new string[(size_t)num];
                    
                    for (size_t i = 0; i < valuec; ++i)
                        tmp[i] = valuev[i];
                    
                    delete[] valuev;
                    
                    valuev = tmp;
                }
                
                valuev[valuec++] = null();
            }
            
            string result = stringify(valuec, valuev);
            
            delete[] valuev;
            
            return result;
        });
        
        
        uov[slice_pos = uoc++] = new ternary_universal_operator("slice", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("slice");
            return nullptr;
        });
        
        uov[splice_pos = uoc++] = new ternary_universal_operator("splice", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("splice");
            return nullptr;
        });
        
        uov[substr_pos = uoc++] = new ternary_universal_operator("substr", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("substr");
            return nullptr;
        });
        
        uov[tospliced_pos = uoc++] = new ternary_universal_operator("tospliced", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("tospliced");
            return nullptr;
        });
        
        uov[relational_pos = uoc++] = new binary_universal_operator("<=", [this](string lhs, string rhs) {
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
                        return std::to_string(1);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return std::to_string(lhs <= rhs);
                    }
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = encode(get_number(rhs));
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs <= rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs <= rhs);
                        }
                        
                        return std::to_string(1);
                    }
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return std::to_string(1);
                            
                            if (is_string(rhs)) {
                                lhs = encode(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a <= b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = encode(get_number(lhs));
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs <= rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = encode(get_number(lhs));
                                        rhs = decode_raw(rhs);
                                        
                                        return std::to_string(lhs <= rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = parse_number(rhs);
                                    
                                    return std::to_string(a <= b);
                                }
                                
                                return std::to_string(1);
                            }
                            
                            double a = get_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a <= b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = decode_raw(lhs);
                        
                        if (ss::is_array(rhs))
                            return std::to_string(1);
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs <= rhs);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    rhs = encode(get_number(rhs));
                                    
                                    return std::to_string(lhs <= rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            return std::to_string(1);
                        }
                        
                        rhs = encode(parse_number(rhs));
                        
                        return std::to_string(lhs <= rhs);
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
                                
                                lhs = decode_raw(lhs);
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            return std::to_string(0);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        double a = parse_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a <= b);
                                    }
                                    
                                    return std::to_string(0);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs <= rhs);
                                }
                                
                                return std::to_string(0);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return std::to_string(0);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return std::to_string(1);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs > rhs)
                                        return std::to_string(0);
                                    
                                    if (lhs < rhs)
                                        return std::to_string(1);
                                } else {
                                    double a = parse_number(std::get<1>(* arrayv[i])[k]);
                                    double b = parse_number(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a > b)
                                        return std::to_string(0);
                                    
                                    if (a < b)
                                        return std::to_string(1);
                                }
                            }
                            
                            return std::to_string(1);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs <= rhs);
                            }
                            
                            double a = parse_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a <= b);
                        }
                        
                        return std::to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return std::to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return std::to_string(1);
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
                                
                                return std::to_string(0);
                            }
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        } else {
                            double a = parse_number(std::get<1>(* arrayv[i])[k]);
                            double b = parse_number(valuev[k]);
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return std::to_string(0);
                            }
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return std::to_string(1);
                }
                
                if (ss::is_array(rhs))
                    return std::to_string(1);
                
                if (is_string(rhs)) {
                    lhs = encode(parse_number(lhs));
                    rhs = decode(rhs);
                    
                    return std::to_string(lhs <= rhs);
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = parse_number(lhs);
                            double b = get_number(rhs);
                            
                            return std::to_string(a <= b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = encode(parse_number(lhs));
                        rhs = decode_raw(rhs);
                        
                        return std::to_string(lhs <= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = encode(parse_number(lhs));
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs <= rhs);
                        }
                        
                        double a = parse_number(lhs);
                        double b = parse_number(rhs);
                        
                        return std::to_string(a <= b);
                    }
                    
                    return std::to_string(1);
                }
                
                double a = parse_number(lhs);
                double b = parse_number(rhs);
                
                return std::to_string(a <= b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return std::to_string(0);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return std::to_string(1);
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
                        
                        return std::to_string(0);
                    }
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                } else {
                    double a = parse_number(valuev[i]);
                    double b = parse_number(_valuev[i]);
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return std::to_string(0);
                    }
                        
                    if (a < b) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return std::to_string(1);
        });
        //  23
        
        uov[uoc++] = new binary_universal_operator(">=", [this](string lhs, string rhs) {
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
                        return std::to_string(0);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return std::to_string(lhs >= rhs);
                    }
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = encode(get_number(rhs));
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs >= rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs >= rhs);
                        }
                        
                        return std::to_string(0);
                    }
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return std::to_string(0);
                            
                            if (is_string(rhs)) {
                                lhs = encode(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a >= b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = encode(get_number(lhs));
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs >= rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = encode(get_number(lhs));
                                        rhs = decode_raw(rhs);
                                        
                                        return std::to_string(lhs >= rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = parse_number(rhs);
                                    
                                    return std::to_string(a >= b);
                                }
                                
                                return std::to_string(0);
                            }
                            
                            double a = get_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a >= b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = decode_raw(lhs);
                        
                        if (ss::is_array(rhs))
                            return std::to_string(0);
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs >= rhs);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    rhs = encode(get_number(rhs));
                                    
                                    return std::to_string(lhs >= rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            return std::to_string(0);
                        }
                        
                        rhs = encode(parse_number(rhs));
                        
                        return std::to_string(lhs >= rhs);
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
                                
                                lhs = decode_raw(lhs);
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            return std::to_string(1);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        //  BUG
                                        double a = parse_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a >= b);
                                    }
                                    
                                    return std::to_string(1);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs >= rhs);
                                }
                                
                                return std::to_string(1);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return std::to_string(1);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return std::to_string(0);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs < rhs)
                                        return std::to_string(0);
                                    
                                    if (lhs > rhs)
                                        return std::to_string(1);
                                } else {
                                    double a = parse_number(std::get<1>(* arrayv[i])[k]);
                                    double b = parse_number(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a < b)
                                        return std::to_string(0);
                                    
                                    if (a > b)
                                        return std::to_string(1);
                                }
                            }
                            
                            return std::to_string(1);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs >= rhs);
                            }
                            
                            double a = parse_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a >= b);
                        }
                        
                        return std::to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return std::to_string(0);
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
                                
                                return std::to_string(0);
                            }
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        } else {
                            double a = parse_number(std::get<1>(* arrayv[i])[k]);
                            double b = parse_number(valuev[k]);
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return std::to_string(0);
                            }
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return std::to_string(1);
                }
                
                if (ss::is_array(rhs))
                    return std::to_string(0);
                
                if (is_string(rhs)) {
                    lhs = encode(parse_number(lhs));
                    rhs = decode(rhs);
                    
                    return std::to_string(lhs >= rhs);
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = parse_number(lhs);
                            double b = get_number(rhs);
                            
                            return std::to_string(a >= b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = encode(parse_number(lhs));
                        rhs = decode_raw(rhs);
                        
                        return std::to_string(lhs >= rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = encode(parse_number(lhs));
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs >= rhs);
                        }
                        
                        double a = parse_number(lhs);
                        double b = parse_number(rhs);
                        
                        return std::to_string(a >= b);
                    }
                    
                    return std::to_string(0);
                }
                
                double a = parse_number(lhs);
                double b = parse_number(rhs);
                
                return std::to_string(a >= b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return std::to_string(1);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return std::to_string(0);
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
                        
                        return std::to_string(0);
                    }
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                } else {
                    double a = parse_number(valuev[i]);
                    double b = parse_number(_valuev[i]);
                    
                    if (a < b) {
                        delete[] valuev;
                        
                        return std::to_string(0);
                    }
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return std::to_string(1);
        });
        //  24
        
        uov[uoc++] = new binary_universal_operator("<", [this](string lhs, string rhs) {
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
                        return std::to_string(1);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return std::to_string(lhs < rhs);
                    }
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = encode(get_number(rhs));
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs < rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs < rhs);
                        }
                        
                        return std::to_string(1);
                    }
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return std::to_string(1);
                            
                            if (is_string(rhs)) {
                                lhs = encode(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a < b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = encode(get_number(lhs));
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs < rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = encode(get_number(lhs));
                                        rhs = decode_raw(rhs);
                                        
                                        return std::to_string(lhs < rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = parse_number(rhs);
                                    
                                    return std::to_string(a < b);
                                }
                                
                                return std::to_string(1);
                            }
                            
                            double a = get_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a < b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = decode_raw(lhs);
                        
                        if (ss::is_array(rhs))
                            return std::to_string(1);
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs < rhs);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    rhs = encode(get_number(rhs));
                                    
                                    return std::to_string(lhs < rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            return std::to_string(1);
                        }
                        
                        rhs = encode(parse_number(rhs));
                        
                        return std::to_string(lhs < rhs);
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
                                
                                lhs = decode_raw(lhs);
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            return std::to_string(0);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        //  BUG
                                        double a = parse_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a < b);
                                    }
                                    
                                    return std::to_string(0);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs < rhs);
                                }
                                
                                return std::to_string(0);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return std::to_string(0);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return std::to_string(1);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs > rhs)
                                        return std::to_string(0);
                                    
                                    if (lhs < rhs)
                                        return std::to_string(1);
                                } else {
                                    double a = parse_number(std::get<1>(* arrayv[i])[k]);
                                    double b = parse_number(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a > b)
                                        return std::to_string(0);
                                    
                                    if (a < b)
                                        return std::to_string(1);
                                }
                            }
                            
                            return std::to_string(0);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs < rhs);
                            }
                            
                            double a = parse_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a < b);
                        }
                        
                        return std::to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        
                        return std::to_string(0);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        
                        return std::to_string(1);
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
                                
                                return std::to_string(0);
                            }
                            
                            if (lhs < rhs) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        } else {
                            double a = parse_number(std::get<1>(* arrayv[i])[k]);
                            double b = parse_number(valuev[k]);
                            
                            if (a > b) {
                                delete[] valuev;
                                
                                return std::to_string(0);
                            }
                            
                            if (a < b) {
                                delete[] valuev;
                                
                                return std::to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return std::to_string(0);
                }
                
                if (ss::is_array(rhs))
                    return std::to_string(1);
                
                if (is_string(rhs)) {
                    lhs = encode(parse_number(lhs));
                    rhs = decode(rhs);
                    
                    return std::to_string(lhs < rhs);
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = parse_number(lhs);
                            double b = get_number(rhs);
                            
                            return std::to_string(a < b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = encode(parse_number(lhs));
                        rhs = decode_raw(rhs);
                        
                        return std::to_string(lhs < rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = encode(parse_number(lhs));
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs < rhs);
                        }
                        
                        double a = parse_number(lhs);
                        double b = parse_number(rhs);
                        
                        return std::to_string(a < b);
                    }
                    
                    return std::to_string(1);
                }
                
                double a = parse_number(lhs);
                double b = parse_number(rhs);
                
                return std::to_string(a < b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return std::to_string(0);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return std::to_string(1);
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
                        
                        return std::to_string(0);
                    }
                    
                    if (lhs < rhs) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                } else {
                    double a = parse_number(valuev[i]);
                    double b = parse_number(_valuev[i]);
                    
                    if (a > b) {
                        delete[] valuev;
                        
                        return std::to_string(0);
                    }
                        
                    if (a < b) {
                        delete[] valuev;
                        
                        return std::to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            
            return std::to_string(0);
        });
        
        uov[uoc++] = new binary_universal_operator(">", [this](string lhs, string rhs) {
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
                        return std::to_string(0);
                    
                    if (is_string(rhs)) {
                        lhs = decode(lhs);
                        rhs = decode(rhs);
                        
                        return std::to_string(lhs > rhs);
                    }
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                lhs = decode(lhs);
                                rhs = encode(get_number(rhs));
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            if (rhs.empty())
                                null_error();
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs > rhs);
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                null_error();
                            
                            lhs = decode(lhs);
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs > rhs);
                        }
                        
                        return std::to_string(0);
                    }
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            if (ss::is_array(rhs))
                                return std::to_string(0);
                            
                            if (is_string(rhs)) {
                                lhs = encode(get_number(lhs));
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        double a = get_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a > b);
                                    }
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    std::get<2>(* stringv[j]).second = true;
                                    
                                    lhs = encode(get_number(lhs));
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs > rhs);
                                }
                                
                                std::get<2>(* arrayv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[j]).size() == 1) {
                                    rhs = std::get<1>(* arrayv[j])[0];
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    if (is_string(rhs)) {
                                        lhs = encode(get_number(lhs));
                                        rhs = decode_raw(rhs);
                                        
                                        return std::to_string(lhs > rhs);
                                    }
                                    
                                    double a = get_number(lhs);
                                    double b = parse_number(rhs);
                                    
                                    return std::to_string(a > b);
                                }
                                
                                return std::to_string(0);
                            }
                            
                            double a = get_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a > b);
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        if (lhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = decode_raw(lhs);
                        
                        if (ss::is_array(rhs))
                            return std::to_string(0);
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs > rhs);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    rhs = encode(get_number(rhs));
                                    
                                    return std::to_string(lhs > rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[j]);
                                
                                if (rhs.empty())
                                    null_error();
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[j]).size() == 1) {
                                rhs = std::get<1>(* arrayv[j])[0];
                                
                                if (rhs.empty())
                                    null_error();
                                
                                rhs = decode_raw(rhs);
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            return std::to_string(0);
                        }
                        
                        rhs = encode(parse_number(rhs));
                        
                        return std::to_string(lhs > rhs);
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
                                
                                lhs = decode_raw(lhs);
                                rhs = decode(rhs);
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            return std::to_string(1);
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        
                                        if (lhs.empty())
                                            null_error();
                                        
                                        //  BUG
                                        double a = parse_number(lhs);
                                        double b = get_number(rhs);
                                        
                                        return std::to_string(a > b);
                                    }
                                    
                                    return std::to_string(1);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                if (std::get<1>(* arrayv[i]).size() == 1) {
                                    lhs = std::get<1>(* arrayv[i])[0];
                                    
                                    if (lhs.empty())
                                        null_error();
                                    
                                    rhs = std::get<1>(* stringv[j]);
                                    
                                    if (rhs.empty())
                                        null_error();
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode_raw(rhs);
                                    
                                    return std::to_string(lhs > rhs);
                                }
                                
                                return std::to_string(1);
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() > std::get<1>(* arrayv[j]).size())
                                return std::to_string(1);
                            
                            if (std::get<1>(* arrayv[i]).size() < std::get<1>(* arrayv[j]).size())
                                return std::to_string(0);
                            
                            for (size_t k = 0; k < std::get<1>(* arrayv[i]).size(); ++k) {
                                if (std::get<1>(* arrayv[i])[k].empty())
                                    null_error();
                                
                                if (std::get<1>(* arrayv[j])[k].empty())
                                    null_error();
                                
                                if (is_string(std::get<1>(* arrayv[i])[k]) || is_string(std::get<1>(* arrayv[i])[k])) {
                                    lhs = decode(std::get<1>(* arrayv[i])[k]);
                                    rhs = decode(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (lhs < rhs)
                                        return std::to_string(0);
                                    
                                    if (lhs > rhs)
                                        return std::to_string(1);
                                } else {
                                    double a = parse_number(std::get<1>(* arrayv[i])[k]);
                                    double b = parse_number(std::get<1>(* arrayv[j])[k]);
                                    
                                    if (a < b)
                                        return std::to_string(0);
                                    
                                    if (a > b)
                                        return std::to_string(1);
                                }
                            }
                            
                            return std::to_string(0);
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() == 1) {
                            lhs = std::get<1>(* arrayv[i])[0];
                            
                            if (lhs.empty())
                                null_error();
                            
                            if (is_string(lhs)) {
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs > rhs);
                            }
                            
                            double a = parse_number(lhs);
                            double b = parse_number(rhs);
                            
                            return std::to_string(a > b);
                        }
                        
                        return std::to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() > valuec) {
                        delete[] valuev;
                        return std::to_string(1);
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() < valuec) {
                        delete[] valuev;
                        return std::to_string(0);
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
                                return std::to_string(0);
                            }
                            
                            if (lhs > rhs) {
                                delete[] valuev;
                                return std::to_string(1);
                            }
                        } else {
                            double a = parse_number(std::get<1>(* arrayv[i])[k]);
                            double b = parse_number(valuev[k]);
                            
                            if (a < b) {
                                delete[] valuev;
                                return std::to_string(0);
                            }
                            
                            if (a > b) {
                                delete[] valuev;
                                return std::to_string(1);
                            }
                        }
                    }
                    
                    delete[] valuev;
                    
                    return std::to_string(0);
                }
                
                if (ss::is_array(rhs))
                    return std::to_string(0);
                
                if (is_string(rhs)) {
                    lhs = encode(parse_number(lhs));
                    rhs = decode(rhs);
                    
                    return std::to_string(lhs > rhs);
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            double a = parse_number(lhs);
                            double b = get_number(rhs);
                            
                            return std::to_string(a > b);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        if (rhs.empty())
                            null_error();
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        lhs = encode(parse_number(lhs));
                        rhs = decode_raw(rhs);
                        
                        return std::to_string(lhs > rhs);
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (std::get<1>(* arrayv[i]).size() == 1) {
                        rhs = std::get<1>(* arrayv[i])[0];
                        
                        if (rhs.empty())
                            null_error();
                        
                        if (is_string(rhs)) {
                            lhs = encode(parse_number(lhs));
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs > rhs);
                        }
                        
                        double a = parse_number(lhs);
                        double b = parse_number(rhs);
                        
                        return std::to_string(a > b);
                    }
                    
                    return std::to_string(0);
                }
                
                double a = parse_number(lhs);
                double b = parse_number(rhs);
                
                return std::to_string(a > b);
            }
            
            string _valuev[rhs.length() + 1];
            size_t _valuec = parse(_valuev, rhs);
            
            if (valuec > _valuec) {
                delete[] valuev;
                
                return std::to_string(1);
            }
            
            if (valuec < _valuec) {
                delete[] valuev;
                
                return std::to_string(0);
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
                        return std::to_string(0);
                    }
                    
                    if (lhs > rhs) {
                        delete[] valuev;
                        return std::to_string(1);
                    }
                } else {
                    double a = parse_number(valuev[i]);
                    double b = parse_number(_valuev[i]);
                    
                    if (a < b) {
                        delete[] valuev;
                        return std::to_string(0);
                    }
                    
                    if (a > b) {
                        delete[] valuev;
                        return std::to_string(1);
                    }
                }
            }
            
            delete[] valuev;
            return std::to_string(0);
        });
        //  26
        
        uov[equality_pos = uoc++] = new binary_universal_operator("===", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return std::to_string(1);
                    //  null === null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return std::to_string(0);
                    //  null === (array)
                    //  null === (string)
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs)) {
                                logic::consume(rhs);
                                
                                return std::to_string(0);
                                //  null === double
                            }
                            
                            undefined_error(rhs);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        return std::to_string(rhs.empty());
                        //  null === string
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    return std::to_string(0);
                    // null === array
                }
                
                return std::to_string(0);
                //  null === (double)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs)) {
                    if (rhs.empty())
                        return std::to_string(0);
                        //  (string) === null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    delete[] v;
                    
                    if (n != 1)
                        return std::to_string(0);
                        //  (string) === (array)
                    
                    lhs = decode(lhs);
                    
                    if (is_string(rhs)) {
                        rhs = decode(rhs);
                        return std::to_string(lhs == rhs ? 1 : 0);
                        //  (string) === (string)
                    }
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs)) {
                                    logic::consume(rhs);
                                    return std::to_string(0);
                                    //  (string) === double
                                }
                                    
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            std::get<2>(* stringv[i]).second = true;
                            
                            if (rhs.empty())
                                return std::to_string(0);
                                //  (string) === null
                            
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs == rhs ? 1 : 0);
                            //  (string) === string
                        }
                        
                        std::get<2>(* arrayv[i]).second = true;
                        return std::to_string(0);
                        //  (string) === array
                    }
                    
                    return std::to_string(0);
                    //  (str) === double
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            double d = get_number(lhs);
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || rhs.empty() || is_string(rhs))
                                return std::to_string(0);
                                //  double === null
                                //  double === (array)
                                //  double === (string)
                            
                            if (is_key(rhs)) {
                                i = io_array(rhs);
                                if (i != -1) {
                                    std::get<2>(* arrayv[i]).second = true;
                                    return std::to_string(0);
                                    //  double === array
                                }
                                
                                i = io_string(rhs);
                                if (i != -1) {
                                    std::get<2>(* stringv[i]).second = true;
                                    return std::to_string(0);
                                    //  double === string
                                }
                                
                                double e = get_number(rhs);
                                if (isnan(d) && isnan(e))
                                    return std::to_string(1);
                                
                                return std::to_string(d == e ? 1 : 0);
                                //  double === double
                            }
                            
                            double e = parse_number(rhs);
                            
                            if (isnan(d) && isnan(e))
                                return std::to_string(1);
                            
                            return std::to_string(d == e ? 1 : 0);
                            //  double === (double)
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (lhs.empty()) {
                            if (rhs.empty())
                                return std::to_string(1);
                                //  null === null
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || is_string(rhs))
                                return std::to_string(0);
                                //  null === (array)
                                //  null === (string)
                            
                            if (is_key(rhs)) {
                                if (io_array(rhs) != -1) {
                                    return std::to_string(0);
                                    //  null === array
                                }
                                
                                int i = io_string(rhs);
                                if (i == -1) {
                                    if (is_defined(rhs))
                                        return std::to_string(0);
                                        //  null === double
                                    
                                    undefined_error(rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[i]);
                                
                                std::get<2>(* stringv[i]).second = true;
                                
                                return std::to_string(rhs.empty() ? 1 : 0);
                                //  null === string
                            }
                            
                            return std::to_string(0);
                            //  null === (double)
                        }
                        
                        lhs = decode_raw(lhs);
                        
                        if (rhs.empty())
                            return std::to_string(0);
                            //  string === null
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            return std::to_string(lhs == rhs ? 1 : 0);
                            //  string === (string)
                        }
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j != -1) {
                                std::get<2>(* arrayv[j]).second = true;
                                return std::to_string(0);
                                //  str === array
                            }
                            
                            j = io_string(rhs);
                            if (j == -1) {
                                if (is_defined(rhs))
                                    return std::to_string(0);
                                    //  str === double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[j]);
                            
                            std::get<2>(* stringv[j]).second = true;
                            
                            if (rhs.empty())
                                return std::to_string(0);
                                //  string === null
                            
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs == rhs ? 1 : 0);
                            //  string === string
                        }
                        
                        return std::to_string(0);
                        //  str === (double)
                    }
                    
                    std::get<2>(* arrayv[i]).second = true;
                    
                    if (rhs.empty())
                        return std::to_string(0);
                        //  array === null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    
                    if (n == 1) {
                        delete[] v;
                        
                        if (is_string(rhs))
                            return std::to_string(0);
                            //  array === (str)
                        
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (is_defined(rhs))
                                        return std::to_string(0);
                                        //  array === double
                                    
                                    undefined_error(rhs);
                                }
                                
                                std::get<2>(* stringv[j]).second = true;
                                
                                return std::to_string(0);
                                //  array === string
                            }
                            
                            std::get<2>(* arrayv[j]).second = true;
                            
                            if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                return std::to_string(0);
                                //  array === array
                            
                            size_t k = 0;
                            while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                ++k;
                            
                            return std::to_string(k == std::get<1>(* arrayv[i]).size() ? 1 : 0);
                            //  array === array
                        }
                        
                        return std::to_string(0);
                        //  array === (double)
                    }
                        
                    if (std::get<1>(* arrayv[i]).size() != n) {
                        delete[] v;
                        return std::to_string(0);
                        //  array === (array)
                    }
                    
                    size_t j = 0;
                    while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 1 : 0);
                    //  array === (array)
                }
                
                if (rhs.empty())
                    return std::to_string(0);
                    //  (double) === null
                
                double d = parse_number(lhs);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return std::to_string(0);
                    //  double === (array)
                    //  double === (string)
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1 || io_string(rhs) != -1)
                        return std::to_string(0);
                        //  double === (array)
                        //  double === (string)
                    
                    double e = get_number(rhs);
                    
                    if (isnan(d) && isnan(e))
                        return std::to_string(1);
                    
                    return std::to_string(d == e ? 1 : 0);
                    //  double === double
                }
                
                double e = parse_number(rhs);
                if (isnan(d) && isnan(e))
                    return std::to_string(1);
                
                return std::to_string(d == e ? 1 : 0);
                //  double === (double)
            }
            
            if (rhs.empty()) {
                delete[] v;
                return std::to_string(0);
                //  (array) === null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    return std::to_string(0);
                    //  (array) === (str)
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return std::to_string(0);
                                //  (array) === double
                            
                            undefined_error(rhs);
                        }
                        
                        return std::to_string(0);
                        //  (array) === string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        return std::to_string(0);
                        //  (array) === array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 1 : 0);
                    //  (array) === array
                }
                
                return std::to_string(0);
                //  (array) === (double)
            }
            
            if (n != p) {
                delete[] v;
                
                return std::to_string(0);
                //  (array) === (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return std::to_string(i == n ? 1 : 0);
            //  (array) === (array)
        });
        //  27
        
        uov[uoc++] = new binary_universal_operator("!==", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return std::to_string(0);
                    //  null !== null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return std::to_string(1);
                    //  null !== (array)
                    //  null !== (string)
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1)
                        return std::to_string(1);
                        //  null !== array
                    
                    int i = io_string(rhs);
                    if (i == -1) {
                        if (is_defined(rhs))
                            return std::to_string(1);
                            //  null !== double
                        
                        undefined_error(rhs);
                    }
                    
                    rhs = std::get<1>(* stringv[i]);
                    
                    return std::to_string(rhs.empty() ? 0 : 1);
                    //  null !== str
                }
                
                return std::to_string(1);
                //  null !== (double)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs)) {
                    if (rhs.empty())
                        return std::to_string(1);
                        //  (string) !== null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    delete[] v;
                    
                    if (n != 1)
                        return std::to_string(1);
                        //  (string) !== (array)
                    
                    lhs = decode(lhs);
                    
                    if (is_string(rhs)) {
                        rhs = decode(rhs);
                        return std::to_string(lhs != rhs ? 1 : 0);
                        //  (string) !== (string)
                    }
                    
                    if (is_key(rhs)) {
                        if (io_array(rhs) != -1)
                            return std::to_string(1);
                            //  (string) !== array
                        
                        int i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return std::to_string(1);
                                //  (string) !== double
                            
                            undefined_error(rhs);
                        }
                        
                        rhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (rhs.empty())
                            return std::to_string(1);
                            //  (string) !== null
                        
                        rhs = decode_raw(rhs);
                        
                        return std::to_string(lhs != rhs ? 1 : 0);
                        //  (string) !== string
                    }
                    
                    return std::to_string(1);
                    //  (string) !== (d)
                }
                
                if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1) {
                            double d = get_number(lhs);
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || rhs.empty() || is_string(rhs))
                                return std::to_string(1);
                                //  double !== null
                                //  double !== (array)
                                //  double !== (string)
                            
                            if (is_key(rhs)) {
                                if (io_array(rhs) != -1 || io_string(rhs) != -1)
                                    return std::to_string(1);
                                    //  double !== array
                                    //  double !== string
                                
                                double e = get_number(rhs);
                                
                                if (isnan(d) && isnan(e))
                                    return std::to_string(0);
                                
                                return std::to_string(d != e ? 1 : 0);
                                //  double !== double
                            }
                            
                            double e = parse_number(rhs);
                            
                            if (isnan(d) && isnan(e))
                                return std::to_string(0);
                            
                            return std::to_string(d != e ? 1 : 0);
                            //  double !== (double)
                        }
                        
                        lhs = std::get<1>(* stringv[i]);
                        
                        std::get<2>(* stringv[i]).second = true;
                        
                        if (lhs.empty()) {
                            if (rhs.empty())
                                return std::to_string(0);
                                //  null !== null
                            
                            v = new string[rhs.length() + 1];
                            n = parse(v, rhs);
                            delete[] v;
                            
                            if (n != 1 || is_string(rhs))
                                return std::to_string(1);
                                //  null !== (array)
                            
                            if (is_key(rhs)) {
                                if (io_array(rhs) != -1)
                                    return std::to_string(1);
                                    //  null !== array
                                
                                int i = io_string(rhs);
                                if (i == -1) {
                                    if (is_defined(rhs))
                                        return std::to_string(1);
                                        //  null !== double
                                    
                                    undefined_error(rhs);
                                }
                                
                                rhs = std::get<1>(* stringv[i]);
                                
                                return std::to_string(rhs.empty() ? 0 : 1);
                                //  null !== string
                            }
                            
                            return std::to_string(1);
                            //  null !== (double)
                        }
                        
                        lhs = decode_raw(lhs);
                        
                        if (rhs.empty())
                            return std::to_string(1);
                            //  string !== null
                        
                        if (is_string(rhs)) {
                            rhs = decode(rhs);
                            
                            return std::to_string(lhs != rhs ? 1 : 0);
                            //  string !== (string)
                        }
                        
                        if (is_key(rhs)) {
                            if (io_array(rhs) != -1)
                                return std::to_string(1);
                                //  str !== array
                            
                            int j = io_string(rhs);
                            if (j == -1) {
                                if (is_defined(rhs))
                                    return std::to_string(1);
                                    //  str !== double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[j]);
                            
                            std::get<2>(* stringv[j]).second = true;
                            
                            if (rhs.empty())
                                return std::to_string(1);
                                //  string !== null
                            
                            rhs = decode_raw(rhs);
                            
                            return std::to_string(lhs != rhs ? 1 : 0);
                            //  string !== string
                        }
                        
                        return std::to_string(1);
                        //  string !== (double)
                    }
                    
                    if (rhs.empty())
                        return std::to_string(1);
                        //  array !== null
                    
                    v = new string[rhs.length() + 1];
                    n = parse(v, rhs);
                    
                    if (n == 1) {
                        delete[] v;
                        
                        if (is_string(rhs))
                            return std::to_string(1);
                            //  array !== (string)
                            
                        if (is_key(rhs)) {
                            int j = io_array(rhs);
                            if (j == -1) {
                                j = io_string(rhs);
                                if (j == -1) {
                                    if (is_defined(rhs))
                                        return std::to_string(1);
                                        //  array !== double
                                    
                                    undefined_error(rhs);
                                }
                                
                                return std::to_string(1);
                                //  array !== string
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                return std::to_string(1);
                                //  array !== array
                            
                            size_t k = 0;
                            while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                ++k;
                            
                            return std::to_string(k == std::get<1>(* arrayv[i]).size() ? 0 : 1);
                            //  array !== array
                        }
                        
                        return std::to_string(1);
                        //  array !== (double)
                    }
                    
                    if (std::get<1>(* arrayv[i]).size() != n) {
                        delete[] v;
                        return std::to_string(1);
                        //  array !== (array)
                    }
                    
                    size_t j = 0;
                    while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 0 : 1);
                    //  array !== (array)
                }
                
                if (rhs.empty())
                    return std::to_string(1);
                    //  double !== null
                
                double d = parse_number(lhs);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n != 1 || is_string(rhs))
                    return std::to_string(1);
                    //  double !== (array)
                    //  double !== (string)
                
                if (is_key(rhs)) {
                    if (io_array(rhs) != -1 || io_string(rhs) != -1)
                        return std::to_string(1);
                        // double !== array
                        // double !== string
                    
                    double e = get_number(rhs);
                    
                    if (isnan(d) && isnan(e))
                        return std::to_string(0);
                    
                    return std::to_string(d != e ? 1 : 0);
                    //  double !== (double)
                }
                
                double e = parse_number(rhs);
                
                if (isnan(d) && isnan(e))
                    return std::to_string(0);
                
                return std::to_string(d != e ? 1 : 0);
                //  double !== (double)
            }
            
            if (rhs.empty())
                return std::to_string(1);
                //  (array) !== null
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    return std::to_string(1);
                    //  (array) !== (string)
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return std::to_string(1);
                                //  (array) !== double
                            
                            undefined_error(rhs);
                        }
                        
                        return std::to_string(1);
                        //  (array) !== string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        return std::to_string(1);
                        //  (array) !== array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 0 : 1);
                    //  (array) !== array
                }
                
                return std::to_string(1);
                //  (array) !== (double)
            }
            
            if (n != p) {
                delete[] v;
                return std::to_string(1);
                //  (array) !== (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return std::to_string(i == n ? 0 : 1);
            //  (array) !== (array)
        });
        //  28
        
        uov[uoc++] = new binary_universal_operator("==", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return std::to_string(1);
                    //  null == null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        return std::to_string(0);
                        //  null == (string)
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs))
                                    return std::to_string(0);
                                    //  null == double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            return std::to_string(rhs.empty() ? 1 : 0);
                            //  null == string
                        }
                        
                        return std::to_string(std::get<1>(* arrayv[i]).size() == 1
                                         && std::get<1>(* arrayv[i])[0].empty()
                                         ? 1 : 0);
                        //  null == array
                    }
                    
                    return std::to_string(0);
                    //  null == (double)
                }
                
                return std::to_string(0);
                //  null == (array)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    lhs = decode(lhs);
                
                else if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1)
                            lhs = encode(get_number(lhs));
                        else {
                            lhs = std::get<1>(* stringv[i]);
                            
                            if (lhs.empty()) {
                                if (rhs.empty())
                                    return std::to_string(1);
                                    //  null == null
                                
                                v = new string[rhs.length() + 1];
                                n = parse(v, rhs);
                                delete[] v;
                                
                                if (n == 1) {
                                    if (is_string(rhs))
                                        return std::to_string(0);
                                        //  null == (string)
                                    
                                    if (is_key(rhs)) {
                                        int j = io_array(rhs);
                                        if (j == -1) {
                                            j = io_string(rhs);
                                            if (j == -1) {
                                                if (is_defined(rhs))
                                                    return std::to_string(0);
                                                    //  null == double
                                                
                                                undefined_error(rhs);
                                            }
                                            
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            return std::to_string(rhs.empty() ? 1 : 0);
                                            //  null == string
                                        }
                                        
                                        return std::to_string(std::get<1>(* arrayv[j]).size() == 1
                                                         && std::get<1>(* arrayv[j])[0].empty()
                                                         ? 1 : 0);
                                        //  null == array
                                    }
                                    
                                    return std::to_string(0);
                                    //  null == (double)
                                }
                                
                                return std::to_string(0);
                                //  null == (array)
                            }
                            
                            lhs = decode_raw(lhs);
                        }
                    } else {
                        if (rhs.empty())
                            return std::to_string(std::get<1>(* arrayv[i]).size() == 1
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
                                        return std::to_string(0);
                                        //  null == (string)
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode(rhs);
                                    
                                    return std::to_string(lhs == rhs ? 1 : 0);
                                    //  array == (string)
                                }
                                
                                return std::to_string(0);
                                //  array == (string)
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        if (std::get<1>(* arrayv[i]).size() == 1) {
                                            lhs = std::get<1>(* arrayv[i])[0];
                                            
                                            if (lhs.empty())
                                                return std::to_string(0);
                                                //  array == (double)
                                            
                                            lhs = decode_raw(lhs);
                                            rhs = encode(get_number(rhs));
                                        
                                            return std::to_string(lhs == rhs ? 1 : 0);
                                            //  array == double
                                        }
                                        
                                        if (is_defined(rhs))
                                            return std::to_string(0);
                                            //  array == double
                                        
                                        undefined_error(rhs);
                                    }
                                    
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        rhs = std::get<1>(* stringv[j]);
                                        
                                        return std::to_string(lhs == rhs ? 1 : 0);
                                        //  array == string
                                    }
                                    
                                    return std::to_string(0);
                                    //  array == string
                                }
                                
                                if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                    return std::to_string(0);
                                    //  array == array
                                
                                size_t k = 0;
                                while (k < std::get<1>(* arrayv[i]).size() && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                    ++k;
                            
                                return std::to_string(k == std::get<1>(* arrayv[i]).size() ? 1 : 0);
                                //  array == array
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    return std::to_string(0);
                                    //  null == (double)
                                
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs == rhs ? 1 : 0);
                                //  array == (double)
                            }
                            
                            return std::to_string(0);
                            //  array == (double)
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() != n) {
                            delete[] v;
                            return std::to_string(0);
                            //  array == (array)
                        }
                        
                        size_t j = 0;
                        while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                            ++j;
                        
                        delete[] v;
                        
                        return std::to_string(j == n ? 1 : 0);
                        //  array == (array)
                    }
                } else
                    lhs = encode(parse_number(lhs));
                
                if (rhs.empty())
                    return std::to_string(0);
                    //  (str) == null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1)
                                rhs = encode(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                if (rhs.empty())
                                    return std::to_string(0);
                                    //  (string) == null
                                
                                rhs = decode(rhs);
                            }
                        } else {
                            if (std::get<1>(* arrayv[i]).size() != 1)
                                return std::to_string(0);
                                //  (string) == array
                            
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                return std::to_string(0);
                                //  (string) == null
                            
                            rhs = decode_raw(rhs);
                        }
                    } else
                        rhs = encode(parse_number(rhs));
                    
                    return std::to_string(lhs == rhs ? 1 : 0);
                    //  (string) == (string)
                }
                
                return std::to_string(0);
                //  (string) == (array)
            }
            
            if (rhs.empty()) {
                delete[] v;
                return std::to_string(0);
                //  (array) == null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    return std::to_string(0);
                    //  (array) == (string)
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return std::to_string(0);
                                //  (array) == d
                            
                            undefined_error(rhs);
                        }
                        
                        return std::to_string(0);
                        //  (array) == str
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        
                        return std::to_string(0);
                        //  (array) == array
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 1 : 0);
                    //  (array) == array
                }
                
                delete[] v;
                
                return std::to_string(0);
                //  (array) == (double)
            }
            
            if (n != p) {
                delete[] v;
                return std::to_string(0);
                //  (array) == (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return std::to_string(i == n ? 1 : 0);
            //  (array) == (array)
        });
        //  29
        
        uov[uoc++] = new binary_universal_operator("!=", [this](string lhs, string rhs) {
            string* v = NULL;
            size_t n;
            
            if (lhs.empty()) {
                if (rhs.empty())
                    return std::to_string(0);
                    //  null != null
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        return std::to_string(1);
                        //  null != (str)
                    
                    if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1) {
                                if (is_defined(rhs))
                                    return std::to_string(1);
                                    //  null != double
                                
                                undefined_error(rhs);
                            }
                            
                            rhs = std::get<1>(* stringv[i]);
                            
                            return std::to_string(rhs.empty() ? 0 : 1);
                            //  null != string
                        }
                        
                        return std::to_string(std::get<1>(* arrayv[i]).size() == 1
                                         && std::get<1>(* arrayv[i])[0].empty()
                                         ? 0 : 1);
                        //  null != array
                    }
                    
                    return std::to_string(1);
                    //  null != (double)
                }
                
                return std::to_string(1);
                //  null != (array)
            }
            
            v = new string[lhs.length() + 1];
            n = parse(v, lhs);
            
            if (n == 1) {
                delete[] v;
                
                if (is_string(lhs))
                    lhs = decode(lhs);
                
                else if (is_key(lhs)) {
                    int i = io_array(lhs);
                    if (i == -1) {
                        i = io_string(lhs);
                        if (i == -1)
                            lhs = encode(get_number(lhs));
                        else {
                            lhs = std::get<1>(* stringv[i]);
                            
                            if (lhs.empty()) {
                                if (rhs.empty())
                                    return std::to_string(0);
                                    //  null != null
                                
                                v = new string[rhs.length() + 1];
                                n = parse(v, rhs);
                                delete[] v;
                                
                                if (n == 1) {
                                    if (is_string(rhs))
                                        return std::to_string(1);
                                        //  null != (string)
                                    
                                    if (is_key(rhs)) {
                                        int j = io_array(rhs);
                                        if (j == -1) {
                                            j = io_string(rhs);
                                            if (j == -1) {
                                                if (is_defined(rhs))
                                                    return std::to_string(1);
                                                    //  null != double
                                                
                                                undefined_error(rhs);
                                            }
                                            
                                            rhs = std::get<1>(* stringv[j]);
                                            
                                            return std::to_string(rhs.empty() ? 0 : 1);
                                            //  null != string
                                        }
                                        
                                        return std::to_string(std::get<1>(* arrayv[j]).size() == 1
                                                         && std::get<1>(* arrayv[j])[0].empty()
                                                         ? 0 : 1);
                                        //  null != array
                                    }
                                    
                                    return std::to_string(1);
                                    //  null == (double)
                                }
                                
                                return std::to_string(1);
                                //  null != (array)
                            }
                            
                            lhs = decode_raw(lhs);
                        }
                    } else {
                        if (rhs.empty())
                            return std::to_string(std::get<1>(* arrayv[i]).size() == 1
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
                                        return std::to_string(1);
                                        //  null != (string)
                                    
                                    lhs = decode_raw(lhs);
                                    rhs = decode(rhs);
                                    
                                    return std::to_string(lhs != rhs ? 1 : 0);
                                    // array != (string)
                                }
                                
                                return std::to_string(1);
                                //  array != (string)
                            }
                            
                            if (is_key(rhs)) {
                                int j = io_array(rhs);
                                if (j == -1) {
                                    j = io_string(rhs);
                                    if (j == -1) {
                                        if (std::get<1>(* arrayv[i]).size() == 1) {
                                            lhs = std::get<1>(* arrayv[i])[0];
                                            
                                            if (lhs.empty())
                                                return std::to_string(1);
                                                //  null != double
                                            
                                            lhs = decode_raw(lhs);
                                            rhs = encode(get_number(rhs));
                                            
                                            return std::to_string(lhs != rhs ? 1 : 0);
                                            //  array != double
                                        }
                                        
                                        if (is_defined(rhs))
                                            return std::to_string(1);
                                            //  array != double
                                        
                                        undefined_error(rhs);
                                    }
                                    
                                    if (std::get<1>(* arrayv[i]).size() == 1) {
                                        lhs = std::get<1>(* arrayv[i])[0];
                                        rhs = std::get<1>(* stringv[j]);
                                        
                                        return std::to_string(lhs != rhs ? 1 : 0);
                                        //  array != string
                                    }
                                    
                                    return std::to_string(1);
                                    //  array != string
                                }
                                
                                if (std::get<1>(* arrayv[i]).size() != std::get<1>(* arrayv[j]).size())
                                    return std::to_string(1);
                                    //  array != array
                                
                                size_t k = 0;
                                while (k < std::get<1>(* arrayv[i]).size()
                                       && std::get<1>(* arrayv[i])[k] == std::get<1>(* arrayv[j])[k])
                                    ++k;
                                
                                return std::to_string(k == std::get<1>(* arrayv[i]).size() ? 0 : 1);
                                //  array != array
                            }
                            
                            if (std::get<1>(* arrayv[i]).size() == 1) {
                                lhs = std::get<1>(* arrayv[i])[0];
                                
                                if (lhs.empty())
                                    return std::to_string(1);
                                    //  null != (double)
                                
                                lhs = decode_raw(lhs);
                                rhs = encode(parse_number(rhs));
                                
                                return std::to_string(lhs != rhs ? 1 : 0);
                                //  array != (double)
                            }
                            
                            return std::to_string(1);
                            //  array != (double)
                        }
                        
                        if (std::get<1>(* arrayv[i]).size() != n) {
                            delete[] v;
                            return std::to_string(1);
                            //  array != (array)
                        }
                        
                        size_t j = 0;
                        while (j < n && std::get<1>(* arrayv[i])[j] == v[j])
                            ++j;
                        
                        return std::to_string(j == n ? 0 : 1);
                        //  array != (array)
                    }
                } else
                    lhs = encode(parse_number(lhs));
                
                if (rhs.empty())
                    return std::to_string(1);
                
                v = new string[rhs.length() + 1];
                n = parse(v, rhs);
                
                delete[] v;
                
                if (n == 1) {
                    if (is_string(rhs))
                        rhs = decode(rhs);
                    
                    else if (is_key(rhs)) {
                        int i = io_array(rhs);
                        if (i == -1) {
                            i = io_string(rhs);
                            if (i == -1)
                                rhs = encode(get_number(rhs));
                            else {
                                rhs = std::get<1>(* stringv[i]);
                                
                                if (rhs.empty())
                                    return std::to_string(1);
                                    //  (string) != null
                                
                                rhs = decode_raw(rhs);
                            }
                        } else {
                            if (std::get<1>(* arrayv[i]).size() != 1)
                                return std::to_string(1);
                                //  (string) != array
                            
                            rhs = std::get<1>(* arrayv[i])[0];
                            
                            if (rhs.empty())
                                return std::to_string(1);
                                //  (string) != null
                            
                            rhs = decode_raw(rhs);
                        }
                    } else
                        rhs = encode(parse_number(rhs));
                    
                    return std::to_string(lhs != rhs ? 1 : 0);
                    //  (string) != (string)
                }
                
                return std::to_string(1);
                //  (str) != (array)
            }
            
            if (rhs.empty()) {
                delete[] v;
                return std::to_string(1);
                //  (array) != null
            }
            
            string w[rhs.length() + 1];
            size_t p = parse(w, rhs);
            
            if (p == 1) {
                if (is_string(rhs)) {
                    delete[] v;
                    return std::to_string(1);
                    //  (array) != (string)
                }
                
                if (is_key(rhs)) {
                    int i = io_array(rhs);
                    if (i == -1) {
                        delete[] v;
                        
                        i = io_string(rhs);
                        if (i == -1) {
                            if (is_defined(rhs))
                                return std::to_string(1);
                                //  (array) != double
                            
                            undefined_error(rhs);
                        }
                        
                        return std::to_string(1);
                        //  (array) != string
                    }
                    
                    if (n != std::get<1>(* arrayv[i]).size()) {
                        delete[] v;
                        
                        return std::to_string(1);
                    }
                    
                    size_t j = 0;
                    while (j < n && v[j] == std::get<1>(* arrayv[i])[j])
                        ++j;
                    
                    delete[] v;
                    
                    return std::to_string(j == n ? 0 : 1);
                    //  (array) != array
                }
                
                delete[] v;
                
                return std::to_string(1);
                //  (array) != (double)
            }
            
            if (n != p) {
                delete[] v;
                return std::to_string(1);
                //  (array) != (array)
            }
            
            size_t i = 0;
            while (i < n && v[i] == w[i])
                ++i;
            
            delete[] v;
            
            return std::to_string(i == n ? 0 : 1);
            //  (array) != (array)
        });

        uov[bitwise_pos = uoc++] = new binary_universal_operator("&", [this](const string lhs, const string rhs) {
            unsupported_error("&");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("^", [this](const string lhs, const string rhs) {
            unsupported_error("&");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("|", [this](const string lhs, const string rhs) {
            unsupported_error("&");
            return nullptr;
        });

        uov[logical_pos = uoc++] = new binary_universal_operator("&&", [this](const string lhs, const string rhs) {
            unsupported_error("&&");
            return nullptr;
        });

        uov[uoc++] = new binary_universal_operator("||", [this](const string lhs, const string rhs) {
            unsupported_error("||");
            return nullptr;
        });
        
        uov[assignment_pos = uoc++] = new binary_universal_operator("*=", [this](const string lhs, const string rhs) {
            unsupported_error("*=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("/=", [this](const string lhs, const string rhs) {
            unsupported_error("/=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("%=", [this](const string lhs, const string rhs) {
            unsupported_error("%=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("+=", [this](const string lhs, const string rhs) {
            unsupported_error("+=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("-=", [this](const string lhs, const string rhs) {
            unsupported_error("-=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator(">>=", [this](string lhs, string rhs) {
            unsupported_error(">>=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("<<=", [this](string lhs, string rhs) {
            unsupported_error("<<=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("&=", [this](string lhs, string rhs) {
            unsupported_error("&=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("^=", [this](string lhs, string rhs) {
            unsupported_error("^=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("|=", [this](string lhs, string rhs) {
            unsupported_error("|=");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator("=", [this](const string lhs, string rhs) {
            unsupported_error("=");
            return nullptr;
        });
        
        uov[conditional_pos = uoc++] = new ternary_universal_operator("?", [this](const string lhs, const string ctr, const string rhs) {
            unsupported_error("?");
            return nullptr;
        });
        
        uov[uoc++] = new binary_universal_operator(":", [this](const string lhs, const string rhs) {
            unsupported_error(":");
            return nullptr;
        });

        uov[nullish_coalescing_pos = uoc++]= new binary_universal_operator("??", [this] (const string lhs, const string rhs) {
            unsupported_error("??");
            return nullptr;
        });
        
        uov[sequencer_pos = uoc++] = new binary_universal_operator(",", [this](const string lhs, const string rhs) {
            unsupported_error(",");
            return nullptr;
        });
        
        buocc = 14;
        buocv = new size_t[buocc];
        buov = new binary_universal_operator**[buocc];
        
        //  indexer                                     //  .
        
        //  arithmetic
        buocv[0] = 1;
        buov[0] = new binary_universal_operator*[buocv[0]];
        buov[0][0] = (binary_universal_operator *)uov[arithmetic_pos];         //  ^^
        
        buocv[1] = 3;
        buov[1] = new binary_universal_operator*[buocv[1]];
        buov[1][0] = (binary_universal_operator *)uov[arithmetic_pos + 1];     //  *
        buov[1][1] = (binary_universal_operator *)uov[arithmetic_pos + 2];     //  /
        buov[1][2] = (binary_universal_operator *)uov[arithmetic_pos + 3];     //  %
        
        buocv[2] = 2;
        buov[2] = new binary_universal_operator*[buocv[2]];
        buov[2][0] = (binary_universal_operator *)uov[additive_pos];           //  +
        buov[2][1] = (binary_universal_operator *)uov[additive_pos + 1];       //  -
        
        buocv[3] = 2;
        buov[3] = new binary_universal_operator*[buocv[3]];
        buov[3][0] = (binary_universal_operator *)uov[additive_pos + 2];       //  <<
        buov[3][1] = (binary_universal_operator *)uov[additive_pos + 3];       //  >>
        
        buocv[4] = 2;
        buov[4] = new binary_universal_operator*[buocv[4]];
        buov[4][0] = (binary_universal_operator *)uov[additive_pos + 4];       //  max
        buov[4][1] = (binary_universal_operator *)uov[additive_pos + 5];       //  min
        
        //  functional
        buocv[5] = 21;
        buov[5] = new binary_universal_operator*[buocv[5]];
        buov[5][0] = (binary_universal_operator *)uov[aggregate_pos];          //  aggregate
        buov[5][1] = (binary_universal_operator *)uov[cell_pos];               //  cell
        buov[5][2] = (binary_universal_operator *)uov[col_pos];                //  column
        buov[5][3] = (binary_universal_operator *)uov[contains_pos];           //  contains
        buov[5][4] = (binary_universal_operator *)uov[reserve_pos];            //  ensure
        buov[5][5] = (binary_universal_operator *)uov[fill_pos];               //  fill
        buov[5][6] = (binary_universal_operator *)uov[find_pos];               //  find
        buov[5][7] = (binary_universal_operator *)uov[find_index_pos];         //  findIndex
        buov[5][8] = (binary_universal_operator *)uov[filter_pos];             //  filter
        buov[5][9] = (binary_universal_operator *)uov[format_pos];             //  format
        buov[5][10] = (binary_universal_operator *)uov[index_of_pos];          //  index
        buov[5][11] = (binary_universal_operator *)uov[insert_pos];            //  insert
        buov[5][12] = (binary_universal_operator *)uov[join_pos];              //  join
        buov[5][13] = (binary_universal_operator *)uov[last_index_of_pos];     //  lastIndexOf
        buov[5][14] = (binary_universal_operator *)uov[map_pos];               //  map
        buov[5][15] = (binary_universal_operator *)uov[splice_pos];            //  remove
        buov[5][16] = (binary_universal_operator *)uov[resize_pos];            //  resize
        buov[5][17] = (binary_universal_operator *)uov[row_pos];               //  row
        buov[5][18] = (binary_universal_operator *)uov[slice_pos];             //  slice
        buov[5][19] = (binary_universal_operator *)uov[substr_pos];            //  substring
        buov[5][20] = (binary_universal_operator *)uov[tospliced_pos];         //  toSpliced
        
        //  relational
        buocv[6] = 4;
        buov[6] = new binary_universal_operator*[buocv[6]];
        buov[6][0] = (binary_universal_operator *)uov[relational_pos];         //  <=
        buov[6][1] = (binary_universal_operator *)uov[relational_pos + 1];     //  >=
        buov[6][2] = (binary_universal_operator *)uov[relational_pos + 2];     //  <
        buov[6][3] = (binary_universal_operator *)uov[relational_pos + 3];     //  >
        
        //  equality
        buocv[7] = 4;
        buov[7] = new binary_universal_operator*[buocv[7]];
        buov[7][0] = (binary_universal_operator *)uov[equality_pos];           //  ===
        buov[7][1] = (binary_universal_operator *)uov[equality_pos + 1];       //  !==
        buov[7][2] = (binary_universal_operator *)uov[equality_pos + 2];       //  ==
        buov[7][3] = (binary_universal_operator *)uov[equality_pos + 3];       //  !=
        
        //  bitwise
        buocv[8] = 1;
        buov[8] = new binary_universal_operator*[buocv[8]];
        buov[8][0] = (binary_universal_operator *)uov[bitwise_pos];            //  &
        
        buocv[9] = 1;
        buov[9] = new binary_universal_operator*[buocv[9]];
        buov[9][0] = (binary_universal_operator *)uov[bitwise_pos + 1];        //  ^
        
        buocv[10] = 1;
        buov[10] = new binary_universal_operator*[buocv[10]];
        buov[10][0] = (binary_universal_operator *)uov[bitwise_pos + 2];        //  |

        //  logical
        buocv[11] = 1;
        buov[11] = new binary_universal_operator*[buocv[11]];
        buov[11][0] = (binary_universal_operator *)uov[logical_pos];            //  &&
        
        buocv[12] = 1;
        buov[12] = new binary_universal_operator*[buocv[12]];
        buov[12][0] = (binary_universal_operator *)uov[logical_pos + 1];        //  ||
        
        //  assignment
        buocv[13] = 13;
        buov[13] = new binary_universal_operator*[buocv[13]];
        buov[13][0] = (binary_universal_operator *)uov[assignment_pos];         //  *=
        buov[13][1] = (binary_universal_operator *)uov[assignment_pos + 1];     //  /=
        buov[13][2] = (binary_universal_operator *)uov[assignment_pos + 2];     //  %=
        buov[13][3] = (binary_universal_operator *)uov[additive_assignment_pos = assignment_pos + 3];     //  +=
        buov[13][4] = (binary_universal_operator *)uov[assignment_pos + 4];     //  -=
        buov[13][5] = (binary_universal_operator *)uov[assignment_pos + 5];     //  >>=
        buov[13][6] = (binary_universal_operator *)uov[assignment_pos + 6];     //  <<=
        buov[13][7] = (binary_universal_operator *)uov[assignment_pos + 7];     //  &=
        buov[13][8] = (binary_universal_operator *)uov[assignment_pos + 8];     //  ^=
        buov[13][9] = (binary_universal_operator *)uov[assignment_pos + 9];     //  |=
        buov[13][10] = (binary_universal_operator *)uov[direct_assignment_pos = assignment_pos + 10];   //  =
        buov[13][11] = (binary_universal_operator *)uov[conditional_pos];       //  ?
        buov[13][12] = (binary_universal_operator *)uov[nullish_coalescing_pos];        //  ??
        
        //  sequencer
                                                          //  ,
        //  END OPERATORS
        
        arrayv = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>*[1];
        functionv = new function_t*[1];
        stringv = new tuple<string, string, pair<bool, bool>, size_t>*[1];
        
        state_arrayv = new pair<size_t, tuple<string, ss::array<string>*,  pair<bool, bool>, size_t>**>*[1];
        state_functionv = new pair<size_t, function_t**>*[1];
        state_numberv = new size_t[1];
        state_stringv = new pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>*[1];
    }

    int command_processor::io_array(const string key) const {
        return io_array(key, 0, arrayc);
    }

    int command_processor::io_array(const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (std::get<0>(* arrayv[start + len]) == key)
            return (int)(start + len);
        
        if (std::get<0>(* arrayv[start + len]) > key)
            return io_array(key, start, start + len);
        
        return io_array(key, start + len + 1, end);
    }

    int command_processor::io_function(const string key) const {
        return io_function(key, 0, functionc);
    }

    int command_processor::io_function(const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (functionv[start + len]->name() == key)
            return (int)(start + len);
        
        if (functionv[start + len]->name() > key)
            return io_function(key, start, start + len);
        
        return io_function(key, start + len + 1, end);
    }

    int command_processor::io_string(const string key) const {
        return io_string(key, 0, stringc);
    }

    int command_processor::io_string(const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (std::get<0>(* stringv[start + len]) == key)
            return (int)(start + len);
        
        if (std::get<0>(* stringv[start + len]) > key)
            return io_string(key, start, start + len);
        
        return io_string(key, start + len + 1, end);
    }

    int command_processor::io_state_array(const size_t state, const string key) const {
        return io_state_array(state, key, 0, state_arrayv[state]->first);
    }

    int command_processor::io_state_array(const size_t state, const string key, size_t start, size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (std::get<0>(* state_arrayv[state]->second[start + len]) == key)
            return (int)(start + len);
        
        if (std::get<0>(* state_arrayv[state]->second[start + len]) > key)
            return io_state_array(state, key, start, start + len);
        
        return io_state_array(state, key, start + len + 1, end);
    }

    int command_processor::io_state_function(const size_t state, const string key) const {
        return io_state_function(state, key, 0, state_functionv[state]->first);
    }

    int command_processor::io_state_function(const size_t state, const string key, size_t start, size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (state_functionv[state]->second[start + len]->name() == key)
            return (int)(start + len);
        
        if (state_functionv[state]->second[start + len]->name() > key)
            return io_state_function(state, key, start, start + len);
        
        return io_state_function(state, key, start + len + 1, end);
    }

    int command_processor::io_state_string(const size_t state, const string key) const {
        return io_state_string(state, key, 0, state_stringv[state]->first);
    }

    int command_processor::io_state_string(const size_t state, const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (std::get<0>(* state_stringv[state]->second[start + len]) == key)
            return (int)(start + len);
        
        if (std::get<0>(* state_stringv[state]->second[start + len]) > key)
            return io_state_string(state, key, start, start + len);
        
        return io_state_string(state, key, start + len + 1, end);
    }

    bool command_processor::is_mutating(const string expression) const {
#if DEBUG_LEVEL
        assert(!expression.empty());
#endif
        string data[expression.length() + 1];
        size_t n = prefix(data, expression);
        
        size_t i = 0;
        while (i < n && data[i] == "(")
            ++i;
        
        if (is_key(data[i]) && i != n - 1 && data[i + 1] == "(")
            return true;
        
        string term = tolower(data[i]);
        
        if (term == uov[insert_pos]->opcode() ||
            term == uov[reserve_pos]->opcode() ||
            term == uov[resize_pos]->opcode() ||
            term == uov[shrink_pos]->opcode() ||
            term == uov[splice_pos]->opcode())
            return true;
        
        size_t j = assignment_pos;
        while (j < conditional_pos && term != uov[j]->opcode())
            ++j;
        
        if (j == conditional_pos)
            return false;
        
        while (i < n && data[i] == ")")
            ++i;
        
        return i != n;
    }

    void command_processor::kill() {
        this->mutex.lock();
        
        while (this->stack.size() > 1)
            this->stack.pop();
        
        if (this->stack.size())
            this->stack.top()->kill();
        
        this->mutex.unlock();
    }

    int command_processor::merge(int n, string* data) const {
#if DEBUG_LEVEL == 2
        assert(n >= 0);
        assert(data != NULL);
#endif
        /*
        for (size_t i = 0; i < n; ++i)
            cout << data[i] << "\t\t";
        
        cout << endl;
         // */
        
        //  MERGE COALESCING OPERANDS
        
        int i;
        for (i = 1; i < n - 1; ++i) {
            if (data[i] == uov[nullish_coalescing_pos]->opcode()) {
                int l = i - 1;   int p = -1;
                do {
                    if (data[l] == "(") {
                        ++p;
                        
                        if (!p)
                            break;
                        
                    } else if (data[l] == ")")
                        --p;
                    
                    else if (p == -1) {
                        size_t j = conditional_pos;
                        while (j < uoc && data[l] != uov[j]->opcode())
                            ++j;
                        
                        if (j != uoc)
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
                   
                size_t r = i + 1;   p = 1;
                do {
                    if (data[r] == "(")
                        ++p;
                    
                    else if (data[r] == ")") {
                        --p;
                        
                        if (!p)
                            break;
                        
                    } else if (p == 1) {
                        size_t j = conditional_pos;
                        while (j < uoc && data[r] != uov[j]->opcode())
                            ++j;
                        
                        if (j != uoc)
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
                
                ++i;
            }
        }
        
        //  MERGE RH CONDITIONAL OPERANDS
        
        i = 0;
        while (i < n - 1) {
            string term = tolower(data[i]);
            
            if (term == uov[conditional_pos]->opcode()) {
                size_t j = ++i;       int p = 0;
                do {
                    if (data[j] == "(")
                        ++p;
                    
                    else if (data[j] == ")")
                        --p;
                    
                    if (!p) {
                        if (data[j + 1] == ")")
                            break;
                        
                        size_t k = conditional_pos;
                        while (k < uoc && data[j + 1] != uov[k]->opcode())
                            ++k;
                        
                        if (k != uoc)
                            break;
                    }
                    
                    ++j;
                    
                } while (j < n - 1);
                    
                if (j == n - 1 || data[j + 1] != uov[conditional_pos + 1]->opcode())
                    expect_error("':'");
                
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
                    
                    else if (data[j] == ")") {
                        --p;
                        
                        if (p == -1)
                            break;
                        
                    } else if (!p) {
                        size_t k = conditional_pos;
                        while (k < uoc && data[j + 1] != uov[k]->opcode())
                            ++k;
                        
                        if (k != uoc)
                            break;
                    }
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
        
        //  MERGE BINARY LOGICAL OPERANDS
        
        for (i = 1; i < n - 1; ++i) {
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
                        if (data[l] == uov[sequencer_pos]->opcode())
                            break;
                        
                        size_t k = 1;
                        while (k < loc - 1 && data[l] != lov[k]->opcode())
                            ++k;
                        
                        if (k != loc - 1)
                            break;
                        
                        k = assignment_pos;
                        while (k < conditional_pos && data[l] != uov[k]->opcode())
                            ++k;
                        
                        if (k != conditional_pos)
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
                   
                size_t r = i + 1;   p = 1;
                do {
                    if (data[r] == "(")
                        ++p;
                    
                    else if (data[r] == ")") {
                        --p;
                        
                        if (!p)
                            break;
                        
                    } else if (p == 1) {
                        if (data[r] == uov[conditional_pos]->opcode() ||
                            data[r] == uov[sequencer_pos]->opcode())
                            break;
                        
                        size_t k = 1;
                        while (k < loc - 1 && data[r] != lov[k]->opcode())
                            ++k;
                        
                        if (k != loc - 1)
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
                
                ++i;
            }
        }
        
        //  MERGE INEQUALITY OPERATORS
        
        for (i = 0; i < n - 1; ++i) {
            if (data[i] == "!" && (data[i + 1] == "=" || data[i + 1] == "==")) {
                data[i] += data[i + 1];
                
                for (size_t j = i + 1; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
            }
        }
        
        //  MERGE HYBRID OPERANDS
        
        i = 0;
        while (i < n - 2) {
            string term = tolower(data[i]);
            
            if (term == uov[splice_pos]->opcode() ||
                term == uov[slice_pos]->opcode() ||
                term == uov[substr_pos]->opcode() ||
                term == uov[tospliced_pos]->opcode()) {
                size_t j = ++i;       int p = 0;
                do {
                    if (data[j] == "(")
                        ++p;
                    
                    else if (data[j] == ")")
                        --p;
                    
                    if (!p && (data[j + 1] == ")" || data[j + 1] == uov[sequencer_pos]->opcode()))
                        break;
                    
                    ++j;
                    
                } while (j < n - 1);
                
                for (; j > i; --j) {
                    data[i] += " " + data[i + 1];
                    
                    for (size_t k = i + 1; k < n - 1; ++k)
                        swap(data[k], data[k + 1]);
                    
                    --n;
                }
                
                if (i != n - 2 && data[i + 1] == uov[sequencer_pos]->opcode()) {
                    ++i;
                    
                    p = 0;
                    for (j = ++i; j < n; ++j) {
                        if (data[j] == "(")
                            ++p;
                        
                        else if (data[j] == ")") {
                            --p;
                            
                            if (p == -1)
                                break;
                            
                        } else if (!p && (data[j] == uov[conditional_pos]->opcode() ||
                                   data[j] == uov[sequencer_pos]->opcode()))
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
        
        //  MERGE TERNARY OPERANDS
        
        i = 0;
        while (i < n - 1) {
            string term = tolower(data[i]);
            
            if (term == uov[aggregate_pos]->opcode() ||
                term == uov[cell_pos]->opcode() ||
                term == uov[filter_pos]->opcode() ||
                term == uov[find_pos]->opcode() ||
                term == uov[find_index_pos]->opcode() ||
                term == uov[insert_pos]->opcode() ||
                term == uov[map_pos]->opcode()) {
                
                size_t j = ++i;       int p = 0;
                do {
                    if (data[j] == "(")
                        ++p;
                    
                    else if (data[j] == ")")
                        --p;
                    
                    if (!p && (data[j + 1] == ")" || data[j + 1] == uov[sequencer_pos]->opcode()))
                        break;
                    
                    ++j;
                    
                } while (j < n - 1);
                
                if (j == n - 1 || data[j + 1] != uov[sequencer_pos]->opcode())
                    expect_error("','");
                
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
                    
                    else if (data[j] == ")") {
                        --p;
                        
                        if (p == -1)
                            break;
                        
                    } else if (!p && (data[j] == uov[conditional_pos]->opcode() ||
                               data[j] == uov[sequencer_pos]->opcode()))
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
        
        //  MERGE EXPONENTIATION OPERATOR

        i = 0;
        while (i < n - 1) {
            if (data[i] == "^" && data[i + 1] == "^") {
                data[i] += data[i + 1];

                for (size_t j = i + 1; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);

                --n;
            } else
                ++i;
        }
        
        //  MERGE FUNCTION ARGUMENTS
        
        for (i = 0; i < n - 2; ++i) {
            size_t j = 0;
            while (j < functionc && data[i] != functionv[j]->name())
                ++j;
            
            if (j != functionc) {
                if (functionv[j]->name() == to_string(array_t) && is_key(data[i + 1]))
                    continue;
                
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
                    
                    else if (!p && data[e] == uov[sequencer_pos]->opcode()) {
                        for (size_t l = e; l > s + 1; --l) {
                            data[s] += " " + data[s + 1];
                            
                            for (size_t m = s + 1; m < n - 1; ++m)
                                swap(data[m], data[m + 1]);
                            
                            --n;
                            --k;
                            --e;
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
        
        for (i = 0; i < n - 2; ++i) {
            size_t j = 0;
            while (j < 3 && data[i + j] == ".")
                ++j;
            
            if (j == 3) {
                for (size_t k = 0; k < 2; ++k) {
                    data[i] += data[i + 1];
                    
                    for (size_t l = i + 1; l < n - 1; ++l)
                        swap(data[l], data[l + 1]);
                    
                    --n;
                }
            }
        }
        
        n = merge_numbers(n, data);
        
        return n;
    }

    int command_processor::merge_numbers(int n, string* data) const {
#if DEBUG_LEVEL == 2
        assert(n >= 0);
        assert(data != NULL);
#endif
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
                        string term = tolower(data[i - 1]);
                        
                        bool flag = false;
                        
                        //  is operator?
                        
                        size_t j = 0;
                        while (j < arithmetic_pos && term != uov[j]->opcode())
                            ++j;
                        
                        if (j == arithmetic_pos) {
                            if (term == uov[additive_pos]->opcode())
                                j = additive_pos;
                            else {
                                j = aggregate_pos;
                                while (j < bitwise_pos && term != uov[j]->opcode())
                                    ++j;
                                
                                if (j == bitwise_pos) {
                                    if (term == uov[assignment_pos + 3]->opcode())
                                        j = assignment_pos + 3;
                                    else {
                                        j = direct_assignment_pos;
                                        while (j < uoc - 1 && term != uov[j]->opcode())
                                            ++j;
                                    }
                                }
                            }
                        }
                        
                        if (j == uoc - 1) {
                            j = 0;
                            while (j < loc - 1 && term != lov[j]->opcode())
                                ++j;
                            
                            if (j == loc - 1) {
                                j = 0;
                                while (j < logic::additive_pos && term != aov[j]->opcode())
                                    ++j;
                                
                                if (j == logic::additive_pos) {
                                    ++j;
                                    
                                    while (j < logic::relational_pos && term != aov[j]->opcode())
                                        ++j;
                                    
                                    if (j == logic::relational_pos) {
                                        j = logic::bitwise_pos;
                                        while (j < aoc - 2 && term != aov[j]->opcode())
                                            ++j;
                                    }
                                }
                                
                                if (j != aoc - 2)
                                    flag = true;
                            } else
                                flag = true;
                        } else
                            flag = true;
                        
                        if (flag && is_number(data[i + 1])) {
                            data[i] += data[i + 1];
                            
                            for (size_t k = i + 1; k < n - 1; ++k)
                                swap(data[k], data[k + 1]);
                            
                            --n;
                        }
                    }
                }
            }
        }
        
        return n;
    }

    int command_processor::prefix(string* dst, const string src) const {
#if DEBUG_LEVEL == 2
        assert(dst != NULL);
#endif
        int n = merge((int)tokens(dst, src, sizeof(SEPARATORS) / sizeof(SEPARATORS[0]), SEPARATORS), dst);
        
        /*
        for (size_t i = 0; i < n; ++i)
            cout << dst[i] << "\t\t";
        
        cout << endl;
         // */
        
        //  array indexer
        
        for (int i = 1; i < n - 1; ++i) {
            if (dst[i] == uov[indexer_pos]->opcode()) {
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
                    string term = tolower(dst[r + 1]);
                    
                    size_t j = 0;
                    while (j < logic::unary_count && term != aov[j]->opcode())
                        ++j;
                    
                    if (j == logic::unary_count && dst[r + 1] != lov[0]->opcode()) {
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
            
        for (int i = 0; i < buocc; ++i) {
            for (int j = 1; j < n - 1; ++j) {
                string term = tolower(dst[j]);
                
                int k = 0;
                while (k < buocv[i] && buov[i][k]->opcode() != term)
                    ++k;
                
                if (k != buocv[i]) {
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
                            term = tolower(dst[l - 1]);
                            
                            m = 0;
                            while (m < logic::unary_count && term != aov[m]->opcode())
                                ++m;
                            
                            if (m == logic::unary_count) {
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
                            term = tolower(dst[r + 1]);
                            
                            m = 0;
                            while (m < logic::unary_count && term != aov[m]->opcode())
                                ++m;
                            
                            if (m == logic::unary_count) {
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

    void command_processor::remove_key(const string key) {
        int i = io_function(key);
        
        if (i == -1) {
            i = io_array(key);
            
            if (i == -1) {
                i = io_string(key);
                
                if (i == -1) {
                    logic::remove_key(key);
                    return;
                }
                
                for (size_t j = i; j < stringc - 1; ++j)
                    swap(stringv[j], stringv[j + 1]);
                
                delete stringv[--stringc];
                
                logic::remove_key(key);
                
                return;
            }
            
            for (size_t j = i; j < arrayc - 1; ++j)
                swap(arrayv[j], arrayv[j + 1]);
            
            delete arrayv[--arrayc];
            
            logic::remove_key(key);
            
            return;
        }
        
        for (size_t j = i; j < functionc - 1; ++j)
            swap(functionv[j], functionv[j + 1]);
        
        --functionc;
    }

    void command_processor::save_state() {
        if (state != -1) {
            size_t state = get_state();
            
            set_state(this->state, false, false);
            set_state(state, false, false);
        }
        
        this->state = (int)get_state();
    }

    void command_processor::set_state() {
#if DEBUG_LEVEL
        assert(state != -1);
#endif
        set_state(state, false, false);
        
        state = -1;
        
        save_state();
    }

    void command_processor::set_state(const size_t state, bool verbose, bool update) {
        size_t _state = state;
        
        int i = (int)statec - 1;
        while (i >= 0 && state_numberv[i] != _state)
            --i;
        
#if DEBUG_LEVEL
        if (i == -1)
            null_error(std::to_string(state));
#endif
        for (size_t j = 0; j < arrayc; ++j) {
            int k = io_state_array(i, std::get<0>(* arrayv[j]));
            
            if (k == -1 || (std::get<3>(* state_arrayv[i]->second[k]) != std::get<3>(* arrayv[j]))) {
                if (verbose)
                    if (!std::get<2>(* arrayv[j]).second)
                        logger_write("Unused " + string(std::get<2>(* arrayv[j]).first ? "constant" : "variable") + " '" + std::get<0>(* arrayv[j]) + "'\n");
            } else if (update) {
                delete std::get<1>(* state_arrayv[i]->second[k]);
                
                std::get<1>(* state_arrayv[i]->second[k]) = new ss::array<string>(std::get<1>(* arrayv[j]));
                std::get<2>(* state_arrayv[i]->second[k]).second = std::get<2>(* arrayv[j]).second;
            }
        }
        
        for (size_t j = 0; j < arrayc; ++j)
            delete arrayv[j];
        
        delete[] arrayv;
        
        arrayc = state_arrayv[i]->first;
        arrayv = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>*[pow2(arrayc)];
        
        for (size_t j = 0; j < arrayc; ++j) {
            string key = std::get<0>(* state_arrayv[i]->second[j]);
            ss::array<string> value = *std::get<1>(* state_arrayv[i]->second[j]);
            pair<bool, bool> flags = std::get<2>(* state_arrayv[i]->second[j]);
            size_t state = std::get<3>(* state_arrayv[i]->second[j]);
            
            arrayv[j] = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>(key, value, flags, state);
            
            delete std::get<1>(* state_arrayv[i]->second[j]);
            delete state_arrayv[i]->second[j];
        }
        
        delete[] state_arrayv[i]->second;
        delete state_arrayv[i];
        
        for (size_t j = i; j < statec - 1; ++j)
            swap(state_arrayv[j], state_arrayv[j + 1]);
        
        if (verbose) {
            for (size_t j = 0; j < functionc; ++j) {
                int k = io_state_function(i, functionv[j]->name());
                
                if (k == -1 && !functionv[j]->count())
                    logger_write("Unused function '" + functionv[j]->name() + "'\n");
            }
        }
        
        delete[] functionv;
        
        functionc = state_functionv[i]->first;
        functionv = state_functionv[i]->second;
        
        delete state_functionv[i];
        
        for (size_t j = i; j < statec - 1; ++j)
            swap(state_functionv[j], state_functionv[j + 1]);
        
        //  set_state numbers
        logic::set_state(state_numberv[i], verbose, update);
        
        //  remove numbers get_state
        for (size_t j = i; j < statec - 1; ++j)
            swap(state_numberv[j], state_numberv[j + 1]);
        
        //  set_state strings
        for (size_t j = 0; j < stringc; ++j) {
            int k = io_state_string(i, std::get<0>(* stringv[j]));
            
            //  no such string
            if (k == -1 || std::get<3>(* state_stringv[i]->second[k]) != std::get<3>(* stringv[j])) {
                if (verbose)
                    if (!std::get<2>(* stringv[j]).second)
                        logger_write("Unused " + string(std::get<2>(* stringv[j]).first ? "constant" : "variable") + " '" + std::get<0>(* stringv[j]) + "'\n");
                
            } else if (update) {
                std::get<1>(* state_stringv[i]->second[k]) = std::get<1>(* stringv[j]);
                std::get<2>(* state_stringv[i]->second[k]).second = std::get<2>(* stringv[j]).second;
            }
        }
        
        //  deallocate strings
        for (size_t j = 0; j < stringc; ++j)
            delete stringv[j];
        
        delete[] stringv;
        
        //  copy strings
        stringc = state_stringv[i]->first;
        stringv = state_stringv[i]->second;
        
        delete state_stringv[i];
        
        //  remove strings get_state
        for (size_t j = i; j < statec - 1; ++j)
            swap(state_stringv[j], state_stringv[j + 1]);
        
        --statec;
    }

    void command_processor::set_array(const string key, const size_t index, const string value) {
#if DEBUG_LEVEL
        if (!is_key(key))
            expect_error("key: " + key);
        
        if (!value.empty() && !is_string(value) && !is_number(value))
            throw error("Unexpected token: " + value);
#endif
        int i = io_array(key);
        
        if (i == -1) {
#if DEBUG_LEVEL
            if (is_defined(key))
                defined_error(key);
#endif
            if (is_pow(arrayc, 2)) {
                tuple<string, ss::array<string>, pair<bool, bool>, size_t>** _arrayv = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>*[arrayc * 2];
                
                for (size_t j = 0; j < arrayc; ++j)
                    _arrayv[j] = arrayv[j];
                
                delete[] arrayv;
                arrayv = _arrayv;
            }
            
            arrayv[arrayc] = new tuple<string, ss::array<string>, pair<bool, bool>, size_t>(key, ss::array<string>(), pair<bool, bool>(false, false), _statec);
            
            size_t j;
            for (j = arrayc++; j > 0 && std::get<0>(* arrayv[j]) < std::get<0>(* arrayv[j - 1]); --j)
                swap(arrayv[j], arrayv[j - 1]);
            
            std::get<1>(* arrayv[j]).push(value);
            
            add_key(key);
        } else {
            if (std::get<2>(* arrayv[i]).first)
                write_error(key);
            
            if (index > std::get<1>(* arrayv[i]).size())
                range_error(std::to_string(index));
            
            if (index == std::get<1>(* arrayv[i]).size())
                std::get<1>(* arrayv[i]).push(value);
            else
                std::get<1>(* arrayv[i])[index] = value;
        }
    }

    void command_processor::set_array(const string key, const string value) {
#if DEBUG_LEVEL
        if (!is_key(key))
            expect_error("key: " + key);
#endif
        string valuev[value.length() + 1];
        size_t valuec = parse(valuev, value);
        
#if DEBUG_LEVEL
        for (size_t i = 0; i < valuec; ++i) {
            if (valuev[i].empty())
                continue;
            
            if (is_string(valuev[i]))
                valuev[i] = encode(decode(valuev[i]));
            
            else if (is_number(valuev[i]))
                valuev[i] = trim_end(parse_number(valuev[i]));
            else
                throw error("Unexpected token: " + value);
        }
#endif
        _set_array(key, valuec, valuev);
    }

    void command_processor::set_function(function_t* function) {
#if DEBUG_LEVEL
        if (is_defined(function->name()))
            defined_error(function->name());
#endif
        if (is_pow(functionc, 2)) {
            function_t** _functionv = new function_t*[functionc * 2];
            
            for (size_t i = 0; i < functionc; ++i)
                _functionv[i] = functionv[i];
            
            delete[] functionv;
            
            functionv = _functionv;
        }
        
        functionv[functionc] = function;
        
        add_key(functionv[functionc]->name());
        
        for (size_t i = functionc++; i > 0 && functionv[i]->name() < functionv[i - 1]->name(); --i)
            swap(functionv[i], functionv[i - 1]);
    }

    void command_processor::set_paused(const bool value) {
        this->mutex.lock();
        
        ss::stack<function_t*>* _stack = new ss::stack<function_t*>();
        
        while (this->stack.size() > 1)
            _stack->push(this->stack.pop());
        
        this->stack.top()->set_paused(value);
        
        while (_stack->size())
            this->stack.push(_stack->pop());
        
        this->mutex.unlock();
    }

    void command_processor::set_read_only(const string key, const bool value) {
        int i;
        if ((i = io_array(key)) == -1) {
            if ((i = io_string(key)) == -1)
                logic::set_read_only(key, false);
            else
                std::get<2>(* stringv[i]).first = value;
        } else
            std::get<2>(* arrayv[i]).first = value;
    }

    void command_processor::set_string(const string key, const string value) {
#if DEBUG_LEVEL
        if (!is_key(key))
            expect_error("key: " + key);
        
        if (!value.empty() && !is_string(value))
            throw error("Unexpected token: " + value);
#endif
        int i = io_string(key);
        if (i == -1) {
#if DEBUG_LEVEL
            if (is_defined(key))
                defined_error(key);
#endif
            if (is_pow(stringc, 2)) {
                tuple<string, string, pair<bool, bool>, size_t>** tmp = new tuple<string, string, pair<bool, bool>, size_t>*[stringc * 2];
                
                for (size_t j = 0; j < stringc; ++j)
                    tmp[j] = stringv[j];
                
                delete[] stringv;
                
                stringv = tmp;
            }
            
            stringv[stringc] = new tuple<string, string, pair<bool, bool>, size_t>(key, value, pair<bool, bool>(false, false), _statec);
            
            for (size_t j = stringc++; j > 0 && std::get<0>(* stringv[j]) < std::get<0>(* stringv[j - 1]); --j)
                swap(stringv[j], stringv[j - 1]);
            
            add_key(key);
        } else {
            if (std::get<2>(* stringv[i]).first)
                write_error(key);
            
            std::get<1>(* stringv[i]) = value;
        }
    }

    void command_processor::stack_push(function_t* function) {
#if DEBUG_LEVEL
        assert(function != NULL);
#endif
        this->mutex.lock();
        
        this->stack.push(function);
        this->mutex.unlock();
    }

    string command_processor::stack_trace() {
        this->mutex.lock();
        
        ostringstream ss;
        
        ss << "  " << expression << endl;

        string padding = "    ";
        
        while (!this->stack.empty()) {
            ss << padding << this->stack.pop()->name() << "()\n";
            
            padding += "  ";
        }
        
        this->mutex.unlock();
        
        return ss.str();
    }

    //  NON-MEMBER FUNCTIONS

    bool evaluate(const string value) {
        if (ss::is_array(value))
            return true;
        
        if (value.empty())
            return false;
        
        if (is_string(value))
            return !decode_raw(value).empty();
        
        return stod(value);
    }

    size_t get_dictionary(string* destination, const string source) {
        size_t len = parse(destination, source);
        
        if (len == 1) {
            if (destination[0].empty() || is_string(destination[0]))
                type_error(string_t, dictionary_t);
            
            type_error(number_t, dictionary_t);
        }
        
        if (!is_dictionary(len, destination))
            type_error(array_t, dictionary_t);
        
        return len;
    }

    int get_int(const string value) {
        if (ss::is_array(value))
            type_error(array_t, int_t);
        
        if (value.empty() || is_string(value))
            type_error(string_t, int_t);
        
        double num = stod(value);
        
        if (!is_int(num))
            type_error(number_t, int_t);
        
        return (int)num;
    }

    double get_number(const string value) {
        if (ss::is_array(value))
            type_error(array_t, number_t);
        
        if (value.empty() || is_string(value))
            type_error(string_t, number_t);
        
        return stod(value);
    }

    string get_string(const string value) {
        if (ss::is_array(value))
            type_error(array_t, string_t);
        
        if (value.empty())
            null_error();
        
        if (!is_string(value))
            type_error(number_t, string_t);
        
        return value;
    }

    size_t get_table(string* destination, const string source) {
        size_t len = parse(destination, source);
        
        if (len == 1) {
            if (destination[0] == std::to_string(1))
                return 1;
            
            if (destination[0].empty() || is_string(destination[0]))
                type_error(string_t, table_t);
            
            type_error(number_t, table_t);
        }
        
        if (!is_table(len, destination))
            type_error(array_t, table_t);
        
        return len;
    }
}
