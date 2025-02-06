//
//  arithmetic.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#include "arithmetic.h"

namespace ss {
    //  CONSTRUCTORS

    arithmetic::arithmetic() {
        initialize();
       
        set_number("E", exp(1), [](variable<double>* value) {
            value->readonly() = true;
            value->value();
        });
        
        set_number("pi", 2 * acos(0.0), [](variable<double>* value) {
            value->readonly() = true;
            value->value();
        });
    }

    arithmetic::~arithmetic() {
        while (statec)
            set_state(std::get<0>(* statev_key[statec - 1]), false, false);
        
        //  deallocate get_states
        delete[] state_numberv;
        delete[] statev_key;
        
        //  dellocate numbers
        for (size_t i = 0; i < numberc; ++i)
            delete numberv[i];
        
        delete[] numberv;
          
        //  deallocate keys
        delete[] keyv;
        
        //  deallocate arithmetic operators tree
        for (size_t i = 0; i < 9; ++i)
            delete[] baov[i];
        
        delete[] baov;
        
        //  close arithmetic operators
        for (size_t i = 0; i < aoc; ++i)
            aov[i]->close();
        
        delete[] aov;
    }

    //  MEMBER FUNCTIONS

    int arithmetic::_get_state(const string key) const {
        return (int)numberv[find_number(key)]->state();
    }

    void arithmetic::add_key(const string key) {
#if DEBUG_LEVEL
        if (!is_key(key))
            expect_error("key: " + key);
#endif
        this->mutex.lock();
        
        int i = this->find_key(key);
        
        if (i != -1)
            defined_error(key);
        
        if (is_pow(this->keyc, 2)) {
            string* tmp = new string[this->keyc * 2];
            
            for (size_t j = 0; j < this->keyc; ++j)
                tmp[j] = this->keyv[j];
            
            delete[] this->keyv;
            
            this->keyv = tmp;
        }
        
        this->keyv[this->keyc] = key;
        
        size_t j = this->keyc++;
        
        while (j > 0 && key < this->keyv[j - 1]) {
            swap(this->keyv[j], this->keyv[j - 1]);
            
            --j;
        }
        
        this->mutex.unlock();
    }

    void arithmetic::analyze(const size_t n, string* data) const {
#if DEBUG_LEVEL == 2
        assert(data != NULL);
#endif
        size_t start = 0, end = n;
        while (start < end && data[start] == "(" && data[end - 1] == ")") {
            int p = 0;
            for (size_t i = start + 1; i < end - 1; ++i) {
                if (data[i] == "(")
                    ++p;
                else if (data[i] == ")") {
                    --p;
                    if (p < 0)
                        break;
                }
            }
            
            if (p)
                break;
            
            ++start;
            --end;
        }
        //  trim leading and trailing balanced parentheses
        
        while (start < end && data[start] == "(")
            ++start;
            //  (j = 0) < (n = 1) && j % 2 == 0
                
        size_t i = 0;
        while (i < aoc && aov[i]->opcode() != data[start])
            ++i;
            //  test leading term for operator
        
        if (i == aoc) {
            if (!isalpha(data[start][0]) && !is_number(data[start]))
                operation_error();
            
            ++start;
            //  operand
        } else {
            //  operator
            
            if (i >= unary_count)
                throw invalid_argument("Syntax error on token \"" + data[start] + "\", expression expected before this token");
                //  illegal binary operator
            
            size_t j;
            for (j = start + 1; j < end; ++j) {
                while (j < end && data[j] == "(")
                    ++j;
                    //  ignore opening parentheses
                
                while (j < end && data[j] == ")")
                    ++j;
                    //  ignore closing parentheses
                
                if (j == end)
                    break;
                
                size_t k = 0;
                while (k < aoc && aov[k]->opcode() != data[j])
                    ++k;
                    //  test terms subsequent unary operator for operators
                
                if (k == aoc) {
                    if (!isalpha(data[j][0]) && !is_number(data[j]))
                        throw invalid_argument("no conversion");
                    
                    start = j + 1;
                    break;
                }
                //  operand
                
                if (k >= unary_count) {
                    stringstream ss;
                    
                    for (size_t k = start; k < j; ++k)
                        if (data[k] != "(")
                            ss << data[k] << " ";
                    
                    ss << data[j];
                    
                    throw invalid_argument("Syntax error on token \"" + ss.str() + "\", invalid operation");
                    //  illegal binary operator
                }
            }
            
            if (j == end) {
                size_t k;
                for (k = j - 1; k >= 0; --k) {
                    size_t l = 0;
                    while (l < unary_count && aov[l]->opcode() != data[k])
                        ++l;
                    
                    if (l != unary_count)
                        break;
                }
                
                throw invalid_argument("Syntax error on token \"" + data[k] + "\", expression expected after this token");
            }
                //  expression ends in unary operator
        }
        //  find leading operand
            
        while (start < end) {
            while (start < end && data[start] == "(")
                ++start;
                //  ignore opening parentheses
            
            while (start < end && data[start] == ")")
                ++start;
            
            if (start == end)
                break;
            
            i = 0;
            while (i < aoc && data[start] != aov[i]->opcode())
                ++i;
                //  test term for operator
            
            if (i < unary_count)
                throw invalid_argument("Syntax error on token \"" + data[start] + "\", invalid operation");
                //  illegal unary operator
                //  unary operator cannot directly follow operand
            
            size_t j;
            for (j = start + 1; j < end; ++j) {
                while (j < end && data[j] == "(")
                    ++j;
                    //  ignore opening parentheses
                
                while (j < end && data[j] == ")")
                    ++j;
                
                if (j == end)
                    break;
                
                //  ignore balanced enclosing parentheses
                
                size_t k = 0;
                while (k < aoc && aov[k]->opcode() != data[j])
                    ++k;
                    //  test terms subsequent binary operator for operators
                
                if (k == aoc) {
                    if (!isalpha(data[j][0]) && !is_number(data[j]))
                        throw invalid_argument("no conversion");
                    
                    start = j + 1;
                    break;
                }
                //  operand
                
                if (k >= unary_count) {
                    stringstream ss;
                    
                    for (size_t k = start; k < j; ++k)
                        if (data[k] != "(")
                            ss << data[k] << " ";
                    
                    ss << data[j];
                    
                    throw invalid_argument("Syntax error on token \"" + ss.str() + "\", invalid operation");
                    //  illegal binary operator
                }
            }
            
            if (j == end) {
                size_t k;
                for (k = j - 1; k >= 0; --k) {
                    size_t l = unary_count;
                    while (l < aoc && aov[l]->opcode() != data[k])
                        ++l;
                    
                    if (l != aoc)
                        break;
                }
                
                throw invalid_argument("Syntax error on token \"" + data[k] + "\", expression expected after this token");
            }
            //  expression ends in operator
        }
    }

    size_t arithmetic::get_state() {
        this->mutex.lock();
        
        size_t _state = this->_statec++;
        
        //  resize get_states
        if (is_pow(this->statec, 2)) {
            //  resize numbers
            pair<size_t, variable<double>**>** _state_numberv =
                new pair<size_t, variable<double>**>*[this->statec * 2];
            
            for (size_t i = 0; i < statec; ++i)
                _state_numberv[i] = this->state_numberv[i];
            
            delete[] this->state_numberv;

            this->state_numberv = _state_numberv;
            
            //  resize keys
            tuple<size_t, size_t, string*>** _statev_key =
                new tuple<size_t, size_t, string*>*[this->statec * 2];
            
            for (size_t i = 0; i < this->statec; ++i)
                _statev_key[i] = this->statev_key[i];
            
            delete[] this->statev_key;

            this->statev_key = _statev_key;
        }
        
        variable<double>** _numberv =
            new variable<double>*[pow2(this->numberc)];
        
        for (size_t i = 0; i < this->numberc; ++i)
            _numberv[i] = new variable<double>(* this->numberv[i]);

        this->state_numberv[this->statec] = new pair<size_t, variable<double>**>(numberc, _numberv);
        
        string* _keyv = new string[pow2(this->keyc)];
        
        for (size_t i = 0; i < this->keyc; ++i)
            _keyv[i] = this->keyv[i];
        
        this->statev_key[this->statec] = new tuple<size_t, size_t, string*>(_state, this->keyc, _keyv);
        this->statec++;
        this->mutex.unlock();
        
        return _state;
    }

    void arithmetic::get_state(const size_t state) {
#if DEBUG_LEVEL
        int i = io_state(state);
        
        if (i != -1)
            defined_error(std::to_string(state));
#endif
        this->mutex.lock();
        
        if (is_pow(this->statec, 2)) {
            //  resize numbers
            pair<size_t, variable<double>**>** _state_numberv =
                new pair<size_t, variable<double>**>*[this->statec * 2];
            
            for (size_t i = 0; i < this->statec; ++i)
                _state_numberv[i] = this->state_numberv[i];
            
            delete[] this->state_numberv;

            this->state_numberv = _state_numberv;
            
            //  resize keys
            tuple<size_t, size_t, string*>** _statev_key =
                new tuple<size_t, size_t, string*>*[this->statec * 2];
            
            for (size_t i = 0; i < this->statec; ++i)
                _statev_key[i] = this->statev_key[i];
            
            delete[] this->statev_key;

            this->statev_key = _statev_key;
        }
        
        //  get_state keys
        string* _keyv = new string[pow2(this->keyc)];
        
        for (size_t i = 0; i < this->keyc; ++i)
            _keyv[i] = this->keyv[i];
        
        this->statev_key[this->statec] = new tuple<size_t, size_t, string*>(state, this->keyc, _keyv);
        
        variable<double>** _numberv =
        new variable<double>*[pow2(this->numberc)];
        
        for (size_t i = 0; i < this->numberc; ++i)
            _numberv[i] = new variable<double>(* this->numberv[i]);

        this->state_numberv[this->statec] = new pair<size_t, variable<double>**>(this->numberc, _numberv);
        this->statec++;
        this->mutex.unlock();
    }

    void arithmetic::consume(const string key) {
        int pos = find_number(key);
        
        if (pos == -1)
            undefined_error(key);
        
        numberv[pos]->value();
    }

    int arithmetic::find_key(const string key) const {
        return find_key(key, 0, keyc);
    }

    int arithmetic::find_key(const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (keyv[start + len] == key)
            return (int)(start + len);
        
        if (keyv[start + len] > key)
            return find_key(key, start, start + len);
        
        return find_key(key, start + len + 1, end);
    }

    double arithmetic::evaluate(const string expr) {
        if (expr.empty())
            throw invalid_argument("empty");
        
        if (balance(expr) > 0)
            throw invalid_argument("Syntax error, insert \")\" to complete expr body");
        
        if (balance(expr) < 0)
            throw invalid_argument("Syntax error on token \")\", delete this token");
        
        string* data = new string[expr.length() + 1];
        size_t n = prefix(data, expr);
        stack<string> s = stack<string>();
        
        for (int i = (int)n - 1; i >= 0; --i) {
            int j = 0;
            while (j < aoc && aov[j]->opcode() != data[i])
                ++j;
            
            if (j == aoc)
                s.push(data[i]);
            else {
                double d;
                
                if (j < unary_count) {
                    unary_arithmetic_operator* u = (unary_arithmetic_operator *)aov[j];
                    
                    d = u->apply(value(s.top()));
                    
                    s.pop();
                } else {
                    string lhs = s.top();   s.pop();
                    
                    binary_arithmetic_operator_t* b = (binary_arithmetic_operator_t *)aov[j];
                    
                    if (j < assignment_pos) {
                        d = b->apply(value(lhs), value(s.top()));
                        s.pop();
                    } else {
                        d = j == aoc - 1 ? value(s.top()) : b->apply(value(lhs), value(s.top()));
                        s.pop();
                        set_number(lhs, d);
                    }
                }
                
                s.push(std::to_string(d));
            }
        }
        
        delete[] data;
        
        return value(s.top());
    }

    double arithmetic::get_number(const string key) {
        int i = find_number(key);
        
        if (i == -1)
            undefined_error(key);
        
        return numberv[i]->value();
    }

    void arithmetic::initialize() {
        numberv = new variable<double>*[1];
    
        state_numberv = new pair<size_t, variable<double>**>*[1];
        statev_key = new tuple<size_t, size_t, string*>*[1];
        
        keyv = new string[1];
        
        aov = new operator_t*[aoc];
        
        aov[0] = new unary_arithmetic_operator("abs", [](const double rhs) { return abs(rhs); });
        aov[1] = new unary_arithmetic_operator("cbrt", [](const double rhs) { return cbrt(rhs); });
        aov[2] = new unary_arithmetic_operator("ceil", [](const double rhs) { return ceil(rhs); });
        aov[3] = new unary_arithmetic_operator("floor", [](const double rhs) { return floor(rhs); });
        aov[4] = new unary_arithmetic_operator("log", [](const double rhs) { return std::log(rhs); });
        aov[5] = new unary_arithmetic_operator("sqrt", [](const double rhs) { return sqrt(rhs); });
        //  unary
        
        aov[unary_count = 6] = new binary_arithmetic_operator("^^", [](const double lhs, const double rhs) { return pow(lhs ,rhs); });
        aov[7] = new binary_arithmetic_operator("*", [](const double lhs, const double rhs) { return lhs * rhs; });
        aov[8] = new binary_arithmetic_operator("/", [](const double lhs, const double rhs) { return lhs / rhs; });
        aov[9] = new binary_arithmetic_operator("%", [](const double lhs, const double rhs) { return fmod(lhs, rhs); });
        aov[additive_pos = 10] = new binary_arithmetic_operator("+", [](const double lhs, const double rhs) { return lhs + rhs; });
        aov[11] = new binary_arithmetic_operator("-", [](const double lhs, const double rhs) { return lhs - rhs; });
        //  arithmetic
        
        aov[12] = new bitwise_binary_arithmetic_operator(">>", [](const double lhs, const double rhs) {
            return (int)lhs << (int)rhs;
        });
        
        aov[13] = new bitwise_binary_arithmetic_operator("<<", [](const double lhs, const double rhs) {
            return (int)lhs >> (int)rhs;
        });
        //  bitwise
        
        aov[14] = new binary_arithmetic_operator("max", [](const double lhs, const double rhs) { return lhs > rhs ? lhs : rhs; });
        aov[15] = new binary_arithmetic_operator("min", [](const double lhs, const double rhs) { return lhs < rhs ? lhs : rhs; });
        
        aov[relational_pos = 16] = new binary_arithmetic_operator("<=", [](const double lhs, const double rhs) { return lhs <= rhs ? 1 : 0; });
        aov[17] = new binary_arithmetic_operator(">=", [](const double lhs, const double rhs) { return lhs >= rhs ? 1 : 0; });
        aov[18] = new binary_arithmetic_operator("<", [](const double lhs, const double rhs) { return lhs < rhs ? 1 : 0; });
        aov[19] = new binary_arithmetic_operator(">", [](const double lhs, const double rhs) { return lhs > rhs ? 1 : 0; });
        //  simply reordering two-tailed operators before one-tailed operators removes the need to merge later
        
        aov[20] = new binary_arithmetic_operator("==", [](const double lhs, const double rhs) { return lhs == rhs ? 1 : 0; });
        aov[21] = new binary_arithmetic_operator("!=", [](const double lhs, const double rhs) { return lhs != rhs ? 1 : 0; });
        //  relational
        
        aov[bitwise_pos = 22] = new bitwise_binary_arithmetic_operator("&", [](const double lhs, const double rhs) { return (int)lhs & (int)rhs; });
        aov[23] = new bitwise_binary_arithmetic_operator("^", [](const double lhs, const double rhs) { return (int)lhs ^ (int)rhs; });
        aov[24] = new bitwise_binary_arithmetic_operator("|", [](const double lhs, const double rhs) { return (int)lhs | (int)rhs; });
        //  bitwise
        
        aov[assignment_pos = 25] = new binary_arithmetic_operator("*=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[7])->apply(lhs, rhs); });
        aov[26] = new binary_arithmetic_operator("/=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[8])->apply(lhs, rhs);; });
        aov[27] = new binary_arithmetic_operator("%=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[9])->apply(lhs, rhs); });
        aov[28] = new binary_arithmetic_operator("-=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[11])->apply(lhs, rhs); });
        aov[29] = new bitwise_binary_arithmetic_operator(">>=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[12])->apply(lhs, rhs); });
        aov[30] = new bitwise_binary_arithmetic_operator("<<=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[13])->apply(lhs, rhs); });
        aov[31] = new bitwise_binary_arithmetic_operator("&=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[23])->apply(lhs, rhs); });
        aov[32] = new bitwise_binary_arithmetic_operator("^=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[24])->apply(lhs, rhs); });
        aov[33] = new bitwise_binary_arithmetic_operator("|=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[assignment_pos])->apply(lhs, rhs); });
        aov[34] = new binary_arithmetic_operator("+=", [this](const double lhs, const double rhs) { return ((binary_arithmetic_operator *)aov[10])->apply(lhs, rhs); });
        aov[35] = new binary_arithmetic_operator("=", [](const double lhs, const double rhs) { return rhs; });
        //  binary
        
        baov = new binary_arithmetic_operator_t**[9];
        
        //  requires corresponding size matrix
                
        baoc[0] = 1;
        baov[0] = new binary_arithmetic_operator_t*[baoc[0]];
        baov[0][0] = (binary_arithmetic_operator_t *)aov[6];        //  pow
        
        baoc[1] = 3;
        baov[1] = new binary_arithmetic_operator_t*[baoc[1]];
        baov[1][0] = (binary_arithmetic_operator_t *)aov[7];        //  *
        baov[1][1] = (binary_arithmetic_operator_t *)aov[8];        //  /
        baov[1][2] = (binary_arithmetic_operator_t *)aov[9];        //  %
        
        baoc[2] = 2;
        baov[2] = new binary_arithmetic_operator_t*[baoc[2]];
        baov[2][0] = (binary_arithmetic_operator_t *)aov[10];       //  +
        baov[2][1] = (binary_arithmetic_operator_t *)aov[11];       //  -
        
        baoc[3] = 2;
        baov[3] = new binary_arithmetic_operator_t*[baoc[3]];
        baov[3][0] = (binary_arithmetic_operator_t *)aov[12];       //  >>
        baov[3][1] = (binary_arithmetic_operator_t *)aov[13];       //  <<
        
        baoc[4] = 2;
        baov[4] = new binary_arithmetic_operator_t*[baoc[4]];
        baov[4][0] = (binary_arithmetic_operator_t *)aov[14];       //  max
        baov[4][1] = (binary_arithmetic_operator_t *)aov[15];       //  min
        
        baoc[5] = 4;
        baov[5] = new binary_arithmetic_operator_t*[baoc[5]];
        baov[5][0] = (binary_arithmetic_operator_t *)aov[16];       //  <=
        baov[5][1] = (binary_arithmetic_operator_t *)aov[17];       //  >=
        baov[5][2] = (binary_arithmetic_operator_t *)aov[18];       //  <
        baov[5][3] = (binary_arithmetic_operator_t *)aov[19];       //  >
        
        baoc[6] = 2;
        baov[6] = new binary_arithmetic_operator_t*[baoc[6]];
        baov[6][0] = (binary_arithmetic_operator_t *)aov[20];       //  ==
        baov[6][1] = (binary_arithmetic_operator_t *)aov[21];       //  !=
        
        baoc[7] = 3;
        baov[7] = new binary_arithmetic_operator_t*[baoc[7]];
        baov[7][0] = (binary_arithmetic_operator_t *)aov[22];       //  &
        baov[7][1] = (binary_arithmetic_operator_t *)aov[23];       //  ^
        baov[7][2] = (binary_arithmetic_operator_t *)aov[24];       //  |
        
        baoc[8] = 11;
        baov[8] = new binary_arithmetic_operator_t*[baoc[8]];
        baov[8][0] = (binary_arithmetic_operator_t *)aov[25];       //  *=
        baov[8][1] = (binary_arithmetic_operator_t *)aov[26];       //  /=
        baov[8][2] = (binary_arithmetic_operator_t *)aov[27];       //  %=
        baov[8][3] = (binary_arithmetic_operator_t *)aov[28];       //  +=
        baov[8][4] = (binary_arithmetic_operator_t *)aov[29];       //  -=
        baov[8][5] = (binary_arithmetic_operator_t *)aov[30];       //  >>=
        baov[8][6] = (binary_arithmetic_operator_t *)aov[31];       //  <<=
        baov[8][7] = (binary_arithmetic_operator_t *)aov[32];       //  &=
        baov[8][8] = (binary_arithmetic_operator_t *)aov[33];       //  ^=
        baov[8][9] = (binary_arithmetic_operator_t *)aov[34];       //  |=
        baov[8][10] = (binary_arithmetic_operator_t *)aov[35];      //  =
    }

    bool arithmetic::is_defined(const string key) {
        this->mutex.lock();
        
        int pos = this->find_key(key) != -1;
        
        this->mutex.unlock();
        
        return pos;
    }

    //  precondition:   expr is non-empty & data is non-null
    //  postcondition:
    size_t arithmetic::prefix(string* data, const string expr) const {
        if (expr.empty())
            throw invalid_argument("empty");
        
        size_t n = split(data, expr);
        
        analyze(n, data);
        
        for (int i = 0; i < 9; ++i) {
            for (int j = 1; j < n - 1; ++j) {
                int k = 0;
                while (k < baoc[i] && baov[i][k]->opcode() != data[j])
                    ++k;
                
                if (k != baoc[i]) {
                    int l = j, p = 0;
                    
                    do {
                        --l;
                        
                        if (data[l] == "(")
                            ++p;
                        
                        else if (data[l] == ")")
                            --p;
                        
                    } while (l > 0 && p);
                    
                    if (l > 0) {
                        int m = 0;
                        while (m < unary_count && aov[m]->opcode() != data[l - 1])
                            ++m;
                        
                        if (m != unary_count)
                            --l;
                    }
                    
                    data[n] = "(";
                    
                    for (size_t m = n; m > l; --m)
                        swap(data[m], data[m - 1]);
                    
                    ++n;
                    
                    int r = ++j;
                    
                    int m = 0;
                    while (m < unary_count && aov[m]->opcode() != data[r + 1])
                        ++m;
                    
                    if (m != unary_count)
                        ++r;
                    
                    p = 0;
                    
                    do {
                        ++r;
                        
                        if (data[r] == "(")
                            ++p;
                        
                        else if (data[r] == ")")
                            --p;
                        
                    } while (r < n - 1 && p);
                    
                    data[n] = ")";
                    
                    for (size_t m = n; m > r + 1; --m)
                        swap(data[m], data[m - 1]);
                    
                    ++n;
                    
                    for (size_t m = j; m > l + 1; --m)
                        swap(data[m], data[m - 1]);
                    
                    ++j;
                }
            }
        }
        
        int i = 0;
        while (i < n) {
            if (data[i] == "(" || data[i] == ")") {
                for (int j = i; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
            } else

                ++i;
        }
        
        return n;
    }

    void arithmetic::remove_key(const string key) {
         this->mutex.lock();
        
        int i = this->find_key(key);
        
#if DEBUG_LEVEL
        if (i == -1)
            null_error(key);
#endif
        for (size_t j = i; j < this->keyc - 1; ++j)
            swap(this->keyv[j], this->keyv[j + 1]);
        
        --this->keyc;
        
        int j = this->find_number(key);
        
        if (j == -1) {
            this->mutex.unlock();
            return;
        }
        
        delete this->numberv[j];
        
        for (size_t k = j; k < this->numberc - 1; ++k)
            swap(this->numberv[k], this->numberv[k + 1]);
        
        this->numberc--;
        this->mutex.unlock();
    }

    void arithmetic::set_state(const size_t state, bool verbose, bool update) {
        this->mutex.lock();
        
        int i = this->find_state(state);
        
#if DEBUG_LEVEL
        if (i == -1)
            null_error(std::to_string(state));
#endif
        for (size_t j = 0; j < this->numberc; ++j) {
            int k = this->find_state_number(i, numberv[j]->key());
            
            if (k == -1 || this->state_numberv[i]->second[k]->state() != this->numberv[j]->state()) {
                if (verbose)
                    if (!this->numberv[j]->suppressed())
                        logger_write("Unused " + string(this->numberv[j]->readonly() ? "constant" : "variable") + " '" + this->numberv[j]->key() + "'");
                
            } else if (update) {
                if (!this->state_numberv[i]->second[k]->readonly())
                    this->state_numberv[i]->second[k]->set_value(this->numberv[j]->value());
                
                if (this->numberv[j]->value())
                    this->state_numberv[i]->second[k]->value();
            }
        }
        
        //  deallocate numbers
        for (size_t j = 0; j < this->numberc; ++j)
            delete this->numberv[j];
        
        delete[] this->numberv;
        
        //  copy numbers
        this->numberc = this->state_numberv[i]->first;
        this->numberv = this->state_numberv[i]->second;
        
        delete this->state_numberv[i];
        
        //  remove numbers get_state
        for (size_t j = i; j < this->statec - 1; ++j)
            swap(this->state_numberv[j], this->state_numberv[j + 1]);
        
        //  deallocate keys
        delete[] this->keyv;
        
        //  copy keys
        this->keyc = std::get<1>(* this->statev_key[i]);
        this->keyv = std::get<2>(* this->statev_key[i]);
        
        delete this->statev_key[i];
        
        //  remove keys get_state
        for (size_t j = i; j < this->statec - 1; ++j)
            swap(this->statev_key[j], this->statev_key[j + 1]);
        
        this->statec--;
        this->mutex.unlock();
    }

    int arithmetic::find_number(const string key) const {
        return find_number(key, 0, numberc);
    }

    int arithmetic::find_number(const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (numberv[start + len]->key() == key)
            return (int)(start + len);
        
        if (numberv[start + len]->key() > key)
            return find_number(key, start, start + len);
        
        return find_number(key, start + len + 1, end);
    }

    int arithmetic::find_state(const size_t key) const {
        int i = (int)statec - 1;
        while (i >= 0 && std::get<0>(* statev_key[i]) != key)
            --i;
        
        return i;
    }

    int arithmetic::find_state_number(const size_t state, const string key) const {
        return find_state_number(state, key, 0, state_numberv[state]->first);
    }

    int arithmetic::find_state_number(const size_t state, const string key, const size_t start, const size_t end) const {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (state_numberv[state]->second[start + len]->key() == key)
            return (int)(start + len);
        
        if (state_numberv[state]->second[start + len]->key() > key)
            return find_state_number(state, key, start, start + len);
        
        return find_state_number(state, key, start + len + 1, end);
    }

    //  precondition:   key is non-empty & double is not nan
    void arithmetic::set_number(const string key, const double value, std::function<void(variable<double>*)> callback) {
#if DEBUG_LEVEL
        if (!is_key(key))
            expect_error("key: " + key);
#endif
        int pos = find_number(key);
        
        if (pos == -1) {
#if DEBUG_LEVEL
            if (is_defined(key))
                defined_error(key);
#endif
            if (is_pow(numberc, 2)) {
                variable<double>** tmp = new variable<double>*[numberc * 2];
                
                for (size_t j = 0; j < numberc; ++j)
                    tmp[j] = numberv[j];
                
                delete[] numberv;
                
                numberv = tmp;
            }
            
            add_key(key);
            
            numberv[numberc] = new variable<double>(key, value, _statec);
            
            for (size_t j = numberc; j > 0 && numberv[j]->key() < numberv[j - 1]->key(); --j)
                swap(numberv[j], numberv[j - 1]);
            
            callback(numberv[numberc++]);
        } else {
            numberv[pos]->set_value(value);
            
            callback(numberv[pos]);
        }
    }

    void arithmetic::set_readonly(const string key, const bool value) {
        int pos = find_number(key);
        
#if DEBUG_LEVEL
        if (i == -1)
            null_error(key);
#endif
        numberv[pos]->readonly() = value;
    }

    //  precondition:   sizeof(data) / sizeof(data[0]) >= expr.length()
    //  postcondition:
    size_t arithmetic::split(string* data, string expr) const {
        if (expr.empty())
            throw invalid_argument("empty");
        
        size_t l = expr.length() + 1;
        
        char str[l];
        
        strcpy(str, expr.c_str());
        
        size_t i;
        for (i = 0; i < l; ++i) {
            if (str[i] == '(' || str[i] == ')') {
                while (i > 0 && isspace(str[i - 1])) {
                    for (size_t j = i - 1; j < l - 1; ++j)
                        swap(str[j], str[j + 1]);
                    --i;
                    --l;
                }
                
                while (i < l - 1 && isspace(str[i + 1])) {
                    for (size_t j = i + 1; j < l - 1; ++j)
                        swap(str[j], str[j + 1]);
                    --l;
                }
            }
        }
        
        expr = str;
            
        //  remove spaces surrounding parentheses
            
        size_t n = 0;
        
        size_t start = 0, end = 0;
        
        i = 0;
        while (i < expr.length()) {
            int j;
            for (j = 0; j < aoc; ++j) {
               if (i + aov[j]->opcode().length() <= expr.length()) {
                   int k = 0;
                   while (k < aov[j]->opcode().length() && aov[j]->opcode()[k] == expr[i + k])
                       ++k;
                   
                   if (k == aov[j]->opcode().length()) {
                       end = i;
                       
                       while (start < end && isspace(expr[start]))
                           ++start;
                       
                       while (start < end && expr[start] == '(' && balance(expr, start, end) > 0) {
                           data[n++] = "(";
                           ++start;
                       }
                       
                       while (end > start && isspace(expr[end - 1]))
                           --end;
                       
                       size_t offset = end;  //  redefinition
                       while (end > start && expr[end - 1] == ')' && balance(expr, start, end) < 0)
                           --end;
                       
                       while (start < end && expr[start] == '(' && expr[end - 1] == ')') {
                           int p = 0;
                           for (size_t q = start + 1; q < end - 1; ++q) {
                               if (expr[q] == '(')
                                   ++p;
                               
                               else if (expr[q] == ')') {
                                   --p;
                                   
                                   if (p < 0)
                                       break;
                               }
                           }
                           
                           if (p)
                               break;
                           
                           ++start;
                           --end;   --offset;
                       }
                       
                       if (start != end)
                           data[n++] = expr.substr(start, end - start);
                       
                       while (end != offset) {
                           data[n++] = ")";
                           ++end;
                       }
                       
                       data[n++] = aov[j]->opcode();
                       
                       start = i + k;
                       i = start;
                       
                       break;
                   }
               }
            }
            
            if (j == aoc)
                ++i;
        }
        
        while (start < expr.length() && isspace(expr[start]))
            ++start;
        
        end = expr.length();
        while (end > start && isspace(expr[end - 1]))
            --end;
        
        i = end;
        while (end > start && expr[end - 1] == ')' && balance(expr, start, end) < 0)
            --end;
        
        while (start < end && expr[start] == '(' && expr[end - 1] == ')') {
            int p = 0;
            for (size_t j = start + end; j < end - 1; ++j) {
                if (expr[j] == '(')
                    ++p;
                else if (expr[j] == ')') {
                    --p;
                    if (p < 0)
                        break;
                }
            }
            
            if (p)
                break;
            
            ++start;
            --end;  --i;
        }
        
        if (start != end)
            data[n++] = expr.substr(start, end - start);
        
        while (end != i) {
            data[n++] = ")";
            end++;
        }
        
        //  merge bitwise shift
        
        for (i = 1; i < n; ++i) {
            if (data[i] == "=") {
                size_t j = unary_count + 1;
                while (j < 14 && aov[j]->opcode() != data[i - 1])
                    ++j;
                
                if (j == 14) {
                    size_t k = 23;
                    while (k < 26 && aov[k]->opcode() != data[i - 1])
                        ++k;
                    
                    if (k != 26) {
                        data[i] = data[i - 1] + data[i];
                        
                        for (size_t l = i - 1; l < n - 1; ++l)
                            swap(data[l], data[l + 1]);
                        
                        --i;
                        --n;
                    }
                } else {
                    data[i] = data[i - 1] + data[i];
                    
                    for (size_t k = i - 1; k < n - 1; ++k)
                        swap(data[k], data[k + 1]);
                    
                    --i;
                    --n;
                }
            }
        }
        
        //  merge assignment operators
        
        if (data[0] == "+" || data[0] == "-") {
            try {
                stod(data[1]);
                
                data[0] += data[1];
                
                for (size_t j = 1; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
                
            } catch (invalid_argument& ia) {
                //  do nothing
            }
        }
        
        //  sign leading number
        
        for (i = 1; i < n - 1; ++i) {
            if (data[i] == "+" || data[i] == "-") {
                try {
                    stod(data[i + 1]);
                    
                    if (data[i - 1] == "(") {
                        data[i] += data[i + 1];
                        
                        for (size_t j = i + 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --n;
                        
                    } else {
                        size_t j = 0;
                        while (j < aoc && aov[j]->opcode() != data[i - 1])
                            ++j;
                        
                        if (j != aoc) {
                            data[i] += data[i + 1];
                            
                            for (size_t k = i + 1; k < n - 1; ++k)
                                swap(data[k], data[k + 1]);
                            --n;
                        }
                    }
                    
                } catch (invalid_argument& ia) {
                    //  do nothing
                }
            }
        }
        
        //  sign numbervc
            
        return n;
    }

    double arithmetic::value(string val) {
#if DEBUG_LEVEL == 2
        if (val.empty())
            throw invalid_argument("empty");
#endif
        if (is_key(val))
            return get_number(val);
            
        return stod(val);
    }

    //  NON-MEMBER FUNCTIONS

    int balance(const string str, size_t start) {
        return balance(str, start, str.length());
    }

    int balance(const string str, const size_t start, const size_t end) {
        int p = 0;
        for (size_t i = start; i < end; ++i) {
            if (str[i] == '(')
                ++p;
            
            else if (str[i] == ')') {
                if (!p)
                    return -1;
                
                --p;
            }
        }
        
        return p ? 1 : 0;
    }
}
