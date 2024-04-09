//
//  arithmetic.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#include "arithmetic.h"

namespace ss {
    //  CONSTRUCTORS

    arithmetic::~arithmetic() {
        //  restore backups
        while (backupc)
            restore(std::get<0>(* bu_symbolv[backupc - 1]), false);
        
        //  deallocate backups
        delete[] bu_numberv;
        delete[] bu_symbolv;
        
        //  close numbers tree
        if (number_bst != NULL)
            number_bst->close();
        
        //  dellocate numbers
        for (size_t i = 0; i < numberc; ++i)
            delete numberv[i];
        
        delete[] numberv;
        
        //  close symbols tree
        if (symbol_bst != NULL)
            symbol_bst->close();
        
        //  deallocate symbols
        delete[] symbolv;
        
        //  deallocate arithmetic operators tree
        for (size_t i = 0; i < 9; ++i)
            delete[] baov[i];
        
        delete[] baov;
        
        //  close arithmetic operators
        for (size_t i = 0; i < aoc; ++i)
            aov[i]->close();
        
        delete[] aov;
    }

    arithmetic::arithmetic() {
        initialize();
       
        set_number("E", exp(1));
        set_read_only("E", true);
        
        set_number("false", 0);
        set_read_only("false", true);
        
        set_number("nan", NAN);
        set_read_only("nan", true);
        
        set_number("pi", 2 * acos(0.0));
        set_read_only("pi", true);
        
        set_number("true", 1);
        set_read_only("true", true);
    }

    //  MEMBER FUNCTIONS

    void arithmetic::add_symbol(const string symbol) {
        if (symbol.empty())
            expect_error("symbol: " + symbol);
        
        int i = this->io_symbol(symbol);
        
        if (i != -1)
            defined_error(symbol);
        
        if (is_pow(this->symbolc, 2)) {
            string* tmp = new string[this->symbolc * 2];
            
            for (size_t j = 0; j < this->symbolc; ++j)
                tmp[j] = this->symbolv[j];
            
            delete[] this->symbolv;
            this->symbolv = tmp;
        }
        
        this->symbolv[this->symbolc] = symbol;
        
        size_t j = this->symbolc++;
        
        while (j > 0 && symbol < this->symbolv[j - 1]) {
            swap(this->symbolv[j], this->symbolv[j - 1]);
            
            --j;
        }
        
        if (this->symbol_bst != NULL)
            this->symbol_bst->close();
        
        this->symbol_bst = build(this->symbolv, 0, (int)this->symbolc);
    }

    void arithmetic::analyze(const size_t n, string* data) const {
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

    string arithmetic::backup() {
        string _uuid = to_string(_backupc++);
        
        //  resize backups
        if (is_pow(backupc, 2)) {
            //  resize numbers
            pair<size_t, tuple<string, double, pair<bool, bool>>**>** _bu_numberv = NULL;
            
            _bu_numberv = new pair<size_t, tuple<string, double, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_numberv[i] = bu_numberv[i];
            
            delete[] bu_numberv;
            bu_numberv = _bu_numberv;
            
            //  resize symbols
            tuple<string, size_t, string*>** _bu_symbolv = NULL;
            
            _bu_symbolv = new tuple<string, size_t, string*>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_symbolv[i] = bu_symbolv[i];
            
            delete[] bu_symbolv;
            bu_symbolv = _bu_symbolv;
        }
        
        //  backup numbers
        tuple<string, double, pair<bool, bool>>** _numberv = NULL;
        
        _numberv = new tuple<string, double, pair<bool, bool>>*[pow2(numberc)];
        
        for (size_t i = 0; i < numberc; ++i) {
            string first = std::get<0>(* numberv[i]);
            double second = std::get<1>(* numberv[i]);
            pair<bool, bool> third = std::get<2>(* numberv[i]);
            
            _numberv[i] = new tuple<string, double, pair<bool, bool>>(first, second, third);
        }

        bu_numberv[backupc] = new pair<size_t, tuple<string, double, pair<bool, bool>>**>(numberc, _numberv);
        
        //  backup symbols
        string* _symbolv = new string[pow2(symbolc)];
        
        for (size_t i = 0; i < symbolc; ++i)
            _symbolv[i] = symbolv[i];
        
        bu_symbolv[backupc] = new tuple<string, size_t, string*>(_uuid, symbolc, _symbolv);
        
        backupc++;
        
        return _uuid;
    }

    void arithmetic::backup(const string buid) {
        if (stoi(buid) >= _backupc)
            ss::range_error(buid);
        
        size_t i = 0;
        while (i < backupc && std::get<0>(* bu_symbolv[i]) != buid)
            ++i;
        
        if (i != backupc)
            defined_error(buid);
        
        //  resize backups
        if (is_pow(backupc, 2)) {
            //  resize numbers
            pair<size_t, tuple<string, double, pair<bool, bool>>**>** _bu_numberv = NULL;
            
            _bu_numberv = new pair<size_t, tuple<string, double, pair<bool, bool>>**>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_numberv[i] = bu_numberv[i];
            
            delete[] bu_numberv;
            bu_numberv = _bu_numberv;
            
            //  resize symbols
            tuple<string, size_t, string*>** _bu_symbolv = NULL;
            
            _bu_symbolv = new tuple<string, size_t, string*>*[backupc * 2];
            
            for (size_t i = 0; i < backupc; ++i)
                _bu_symbolv[i] = bu_symbolv[i];
            
            delete[] bu_symbolv;
            bu_symbolv = _bu_symbolv;
        }
        
        //  backup numbers
        tuple<string, double, pair<bool, bool>>** _numberv = NULL;
        
        _numberv = new tuple<string, double, pair<bool, bool>>*[pow2(numberc)];
        
        for (size_t i = 0; i < numberc; ++i) {
            string first = std::get<0>(* numberv[i]);
            double second = std::get<1>(* numberv[i]);
            pair<bool, bool> third = std::get<2>(* numberv[i]);
            
            _numberv[i] = new tuple<string, double, pair<bool, bool>>(first, second, third);
        }

        bu_numberv[backupc] = new pair<size_t, tuple<string, double, pair<bool, bool>>**>(numberc, _numberv);
        
        //  backup symbols
        string* _symbolv = new string[pow2(symbolc)];
        
        for (size_t i = 0; i < symbolc; ++i)
            _symbolv[i] = symbolv[i];
        
        bu_symbolv[backupc] = new tuple<string, size_t, string*>(buid, symbolc, _symbolv);
        
        backupc++;
    }

    void arithmetic::consume(const string symbol) {
        if (symbol.empty())
            expect_error("symbol: " + symbol);
        
        int i = io_number(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        std::get<2>(* numberv[i]).second = true;
    }

    int arithmetic::io_symbol(const string symbol) const {
        if (symbol_bst == NULL)
            return -1;
        
        return index_of(symbol_bst, symbol);
    }

    //  precondition:   expr is non-empty
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
                    uao* u = (uao *)aov[j];
                    
                    d = u->apply(value(s.top()));
                    
                    s.pop();
                } else {
                    string lhs = s.top();   s.pop();
                    
                    bao_t* b = (bao_t *)aov[j];
                    
                    if (j < 25) {
                        d = b->apply(value(lhs), value(s.top()));
                        s.pop();
                    } else {
                        d = j == aoc - 1 ? value(s.top()) : b->apply(value(lhs), value(s.top()));
                        s.pop();
                        set_number(lhs, d);
                    }
                }
                
                s.push(to_string(d));
            }
        }
        
        delete[] data;
        
        return value(s.top());
    }

    //  precondition:   key is non-empty
    double arithmetic::get_number(const string symbol) {
        int i = io_number(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        std::get<2>(* numberv[i]).second = true;
        
        return std::get<1>(* numberv[i]);
    }

    void arithmetic::initialize() {
        bu_numberv = new pair<size_t, tuple<string, double, pair<bool, bool>>**>*[1];
        
        aov = new operator_t*[aoc];
        
        aov[0] = new uao("ABS", [](const double rhs) { return abs(rhs); });
        aov[1] = new uao("CBRT", [](const double rhs) { return cbrt(rhs); });
        aov[2] = new uao("CEIL", [](const double rhs) { return ceil(rhs); });
        aov[3] = new uao("FLOOR", [](const double rhs) { return floor(rhs); });
        aov[4] = new uao("LOG", [](const double rhs) { return std::log(rhs); });
        aov[5] = new uao("SQRT", [](const double rhs) { return sqrt(rhs); });
        //  unary
        
        aov[(unary_count = 6)] = new bao("POW", [](const double lhs, const double rhs) { return pow(lhs ,rhs); });
        aov[7] = new bao("*", [](const double lhs, const double rhs) { return lhs * rhs; });
        aov[8] = new bao("/", [](const double lhs, const double rhs) { return lhs / rhs; });
        aov[9] = new bao("%", [](const double lhs, const double rhs) { return fmod(lhs, rhs); });
        aov[10] = new bao("+", [](const double lhs, const double rhs) { return lhs + rhs; });
        aov[11] = new bao("-", [](const double lhs, const double rhs) { return lhs - rhs; });
        //  arithmetic
        
        aov[12] = new bbao(">>", [](const double lhs, const double rhs) { return (int)lhs << (int)rhs; });
        aov[13] = new bbao("<<", [](const double lhs, const double rhs) { return (int)lhs >> (int)rhs; });
        //  bitwise
        
        aov[14] = new bao("MAX", [](const double lhs, const double rhs) { return lhs > rhs ? lhs : rhs; });
        aov[15] = new bao("MIN", [](const double lhs, const double rhs) { return lhs < rhs ? lhs : rhs; });
        
        aov[16] = new bao("<=", [](const double lhs, const double rhs) { return lhs <= rhs ? 1 : 0; });
        aov[17] = new bao(">=", [](const double lhs, const double rhs) { return lhs >= rhs ? 1 : 0; });
        aov[18] = new bao("<", [](const double lhs, const double rhs) { return lhs < rhs ? 1 : 0; });
        aov[19] = new bao(">", [](const double lhs, const double rhs) { return lhs > rhs ? 1 : 0; });
        //  simply reordering two-tailed operators before one-tailed operators removes the need to merge later
        
        aov[20] = new bao("==", [](const double lhs, const double rhs) { return lhs == rhs ? 1 : 0; });
        aov[21] = new bao("!=", [](const double lhs, const double rhs) { return lhs != rhs ? 1 : 0; });
        //  relational
        
        aov[22] = new bbao("&", [](const double lhs, const double rhs) { return (int)lhs & (int)rhs; });
        aov[23] = new bbao("^", [](const double lhs, const double rhs) { return (int)lhs ^ (int)rhs; });
        aov[24] = new bbao("|", [](const double lhs, const double rhs) { return (int)lhs | (int)rhs; });
        //  bitwise
        
        aov[25] = new bao("*=", [this](const double lhs, const double rhs) { return ((bao *)aov[7])->apply(lhs, rhs); });
        aov[26] = new bao("/=", [this](const double lhs, const double rhs) { return ((bao *)aov[8])->apply(lhs, rhs);; });
        aov[27] = new bao("%=", [this](const double lhs, const double rhs) { return ((bao *)aov[9])->apply(lhs, rhs); });
        aov[28] = new bao("-=", [this](const double lhs, const double rhs) { return ((bao *)aov[11])->apply(lhs, rhs); });
        aov[29] = new bbao(">>=", [this](const double lhs, const double rhs) { return ((bao *)aov[12])->apply(lhs, rhs); });
        aov[30] = new bbao("<<=", [this](const double lhs, const double rhs) { return ((bao *)aov[13])->apply(lhs, rhs); });
        aov[31] = new bbao("&=", [this](const double lhs, const double rhs) { return ((bao *)aov[23])->apply(lhs, rhs); });
        aov[32] = new bbao("^=", [this](const double lhs, const double rhs) { return ((bao *)aov[24])->apply(lhs, rhs); });
        aov[33] = new bbao("|=", [this](const double lhs, const double rhs) { return ((bao *)aov[25])->apply(lhs, rhs); });
        aov[34] = new bao("+=", [this](const double lhs, const double rhs) { return ((bao *)aov[10])->apply(lhs, rhs); });
        aov[35] = new bao("=", [](const double lhs, const double rhs) { return rhs; });
        //  binary
        
        /*
        for (size_t i = 0; i < aoc; ++i)
            cout << aov[i]->opcode() << endl;
         */
        
        baov = new bao_t**[9];
        
        //  requires corresponding size matrix
                
        baoc[0] = 1;
        baov[0] = new bao_t*[baoc[0]];
        baov[0][0] = (bao_t *)aov[6];        //  pow
        
        baoc[1] = 3;
        baov[1] = new bao_t*[baoc[1]];
        baov[1][0] = (bao_t *)aov[7];        //  *
        baov[1][1] = (bao_t *)aov[8];        //  /
        baov[1][2] = (bao_t *)aov[9];        //  %
        
        baoc[2] = 2;
        baov[2] = new bao_t*[baoc[2]];
        baov[2][0] = (bao_t *)aov[10];       //  +
        baov[2][1] = (bao_t *)aov[11];       //  -
        
        baoc[3] = 2;
        baov[3] = new bao_t*[baoc[3]];
        baov[3][0] = (bao_t *)aov[12];       //  >>
        baov[3][1] = (bao_t *)aov[13];       //  <<
        
        baoc[4] = 2;
        baov[4] = new bao_t*[baoc[4]];
        baov[4][0] = (bao_t *)aov[14];       //  max
        baov[4][1] = (bao_t *)aov[15];       //  min
        
        baoc[5] = 4;
        baov[5] = new bao_t*[baoc[5]];
        baov[5][0] = (bao_t *)aov[16];       //  <=
        baov[5][1] = (bao_t *)aov[17];       //  >=
        baov[5][2] = (bao_t *)aov[18];       //  <
        baov[5][3] = (bao_t *)aov[19];       //  >
        
        baoc[6] = 2;
        baov[6] = new bao_t*[baoc[6]];
        baov[6][0] = (bao_t *)aov[20];       //  ==
        baov[6][1] = (bao_t *)aov[21];       //  !=
        
        baoc[7] = 3;
        baov[7] = new bao_t*[baoc[7]];
        baov[7][0] = (bao_t *)aov[22];       //  &
        baov[7][1] = (bao_t *)aov[23];       //  ^
        baov[7][2] = (bao_t *)aov[24];       //  |
        
        baoc[8] = 11;
        baov[8] = new bao_t*[baoc[8]];
        baov[8][0] = (bao_t *)aov[25];       //  *=
        baov[8][1] = (bao_t *)aov[26];       //  /=
        baov[8][2] = (bao_t *)aov[27];       //  %=
        baov[8][3] = (bao_t *)aov[28];       //  +=
        baov[8][4] = (bao_t *)aov[29];       //  -=
        baov[8][5] = (bao_t *)aov[30];       //  >>=
        baov[8][6] = (bao_t *)aov[31];       //  <<=
        baov[8][7] = (bao_t *)aov[32];       //  &=
        baov[8][8] = (bao_t *)aov[33];       //  ^=
        baov[8][9] = (bao_t *)aov[34];       //  |=
        baov[8][10] = (bao_t *)aov[35];      //  =
    }

    bool arithmetic::is_defined(const string symbol) const {
        return this->io_symbol(symbol) != -1;
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

    void arithmetic::remove_symbol(const string symbol) {
        int i = io_symbol(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        for (size_t j = i; j < symbolc - 1; ++j)
            swap(symbolv[j], symbolv[j + 1]);
        
        --symbolc;
        
        symbol_bst->close();
        
        symbol_bst = symbolc ? build(symbolv, 0, (int)symbolc) : NULL;
        
        int j = io_number(symbol);
        
        if (j == -1)
            return;
        
        delete numberv[j];
        
        for (size_t k = j; k < numberc - 1; ++k)
            swap(numberv[k], numberv[k + 1]);
        
        --numberc;
        
        number_bst->close();
        
        string _symbolv[numberc];
        
        for (size_t j = 0; j < numberc; ++j)
            _symbolv[j] = std::get<0>(* numberv[j]);
        
        number_bst = build(_symbolv, 0, (int)numberc);
    }

    void arithmetic::restore(const string uuid, bool verbose, bool update, size_t symbolc, string* symbolv) {
        //  find backup
        size_t i = 0;
        while (i < backupc && std::get<0>(* bu_symbolv[i]) != uuid)
            ++i;
        
        //  no such backup
        if (i == backupc)
            undefined_error(uuid);
        
        for (size_t j = 0; j < numberc; ++j) {
            //  find number
            size_t k = 0;
            while (k < bu_numberv[i]->first && std::get<0>(* numberv[j]) != std::get<0>(* bu_numberv[i]->second[k]))
                ++k;
            
            //  no such number
            if (k == bu_numberv[i]->first) {
                if (verbose)
                    if (!std::get<2>(* numberv[j]).second)
                        logger_write("Unused variable '" + std::get<0>(* numberv[j]) + "'\n");
            } else {
                //  find exception
                size_t l = 0;
                while (l < symbolc && std::get<0>(* numberv[j]) != symbolv[l])
                    ++l;
                
                //  no such exception
                if (l == symbolc) {
                    std::get<1>(* bu_numberv[i]->second[k]) = std::get<1>(* numberv[j]);
                    std::get<2>(* bu_numberv[i]->second[k]).second = std::get<2>(* numberv[j]).second;
                } else if (update) {
                    //  remove exception
                    for (; l < symbolc - 1; ++l)
                        swap(symbolv[l], symbolv[l + 1]);
                    
                    --symbolc;
                }
            }
        }
        
        //  deallocate numbers
        for (size_t j = 0; j < numberc; ++j)
            delete numberv[j];
        
        delete[] numberv;
        
        //  copy numbers
        numberc = bu_numberv[i]->first;
        numberv = bu_numberv[i]->second;
        
        delete bu_numberv[i];
        
        //  remove numbers backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_numberv[j], bu_numberv[j + 1]);
        
        //  rebuild numbers tree
        if (number_bst != NULL)
            number_bst->close();
        
        string _symbolv[numberc];
        
        for (size_t i = 0; i < numberc; ++i)
            _symbolv[i] = std::get<0>(* numberv[i]);
        
        number_bst = numberc ? build(_symbolv, 0, (int)numberc) : NULL;
        
        //  deallocate symbols
        delete[] this->symbolv;
        
        //  copy symbols
        this->symbolc = std::get<1>(* bu_symbolv[i]);
        this->symbolv = std::get<2>(* bu_symbolv[i]);
        
        delete bu_symbolv[i];
        
        //  remove symbols backup
        for (size_t j = i; j < backupc - 1; ++j)
            swap(bu_symbolv[j], bu_symbolv[j + 1]);
        
        //  rebuild symbols tree
        if (symbol_bst != NULL)
            symbol_bst->close();
        
        symbol_bst = this->symbolc ? build(this->symbolv, 0, (int)this->symbolc) : NULL;
        
        --backupc;
    }

    int arithmetic::io_number(const string symbol) const {
        if (number_bst == NULL)
            return -1;
        
        return index_of(number_bst, symbol);
    }

    //  precondition:   key is non-empty & double is not nan
    void arithmetic::set_number(const string symbol, const double value) {
        if (!is_symbol(symbol))
            expect_error("symbol: " + symbol);
        
        int i = io_number(symbol);
        
        if (i == -1) {
            if (is_defined(symbol))
                defined_error(symbol);
            
            if (is_pow(numberc, 2)) {
                tuple<string, double, pair<bool, bool>>** tmp = new tuple<string, double, pair<bool, bool>>*[numberc * 2];
                
                for (size_t j = 0; j < numberc; ++j)
                    tmp[j] = numberv[j];
                
                delete[] numberv;
                numberv = tmp;
            }
            
            numberv[numberc] = new tuple<string, double, pair<bool, bool>> { symbol, value, pair<bool, bool> { false, false }};
                    
            size_t j = numberc++;
            while (j > 0 && symbol < std::get<0>(* numberv[j - 1])) {
                swap(numberv[j], numberv[j - 1]);
                
                --j;
            }
            
            add_symbol(symbol);
            
            if (number_bst != NULL)
                number_bst->close();
            
            string symbolv[numberc];
            
            for (size_t j = 0; j < numberc; ++j)
                symbolv[j] = std::get<0>(* numberv[j]);
            
            number_bst = build(symbolv, 0, (int)numberc);
        } else {
            if (std::get<2>(* numberv[i]).first)
                write_error(symbol);
            
            std::get<1>(* numberv[i]) = value;
        }
    }

    void arithmetic::set_read_only(const string symbol, const bool value) {
        int i = io_number(symbol);
        
        if (i == -1)
            undefined_error(symbol);
        
        std::get<2>(* numberv[i]).first = value;
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
        if (val.empty())
            throw invalid_argument("empty");
        
        if (is_symbol(val))
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
