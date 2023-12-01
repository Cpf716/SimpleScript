//
//  logic.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#include "logic.h"

namespace ss {
    //  CONSTRUCTORS

    logic::logic() {
        lov = new operator_t*[loc];
        
        lov[0] = new ulo("!", [this](const string rhs) {
            return arithmetic::evaluate(rhs) ? 0 : 1;
        });
        
        lov[1] = new blo("&&", [this](const string lhs, const string rhs) {
            return arithmetic::evaluate(lhs) && arithmetic::evaluate(rhs) ? 1 : 0;
        });
        
        lov[2] = new blo("||", [this](const string lhs, const string rhs) {
            return arithmetic::evaluate(lhs) || arithmetic::evaluate(rhs) ? 1 : 0;
        });
        
        lov[3] = new blo("=", [this](const string lhs, const string rhs) {
            return arithmetic::evaluate(rhs);
        });
    }

    logic::~logic() {
        for (size_t i = 0; i < loc; ++i)
            lov[i]->close();
        
        delete[] lov;
    }

    //  MEMBER FUNCTIONS

    void logic::analyze(const size_t n, string *data) const {
        size_t start = 0, end = n;
        while (start < end && data[start] == "(" && data[end - 1] == ")") {
            int p = 0;
            for (size_t i = start + 1; i < end - 1; ++i) {
                if (data[i] == "(")
                    ++p;
                else if (data[i] == ")")  {
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
        
        for (size_t i = start; i < end; ++i)
            if (data[i].length() != 1 && (balance(data[i]) < 0 || balance(data[i]) > 0))
                type_error("boolean", "double");
        
        while (start < end && data[start] == "(")
            ++start;
                
        size_t i = 0;
        while (i < loc && lov[i]->opcode() != data[start])
            ++i;
            //  test leading term for operator
        
        if (i == loc)
            ++start;
            //  operand
        else {
            //  operator
            
            if (i)
                throw error("Syntax error on token \"" + data[start] + "\", expression expected before this token");
                //  illegal binary operator
            
            size_t j;
            for (j = start + 1; j < end; ++j) {
                while (j < end && data[j] == "(")
                    ++j;
                    //  ignore opening parentheses
                
                while (j < end && data[j] == ")")
                    ++j;
                
                if (j == end)
                    break;
                
                size_t k = 0;
                while (k < loc && lov[k]->opcode() != data[j])
                    ++k;
                    //  test terms subsequent unary operator for operators
                
                if (k == loc) {
                    start = j + 1;
                    break;
                }
                //  operand
                
                if (k) {
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
                size_t k = j - 1;
                while (k > 0 && data[k] != lov[0]->opcode())
                    --k;
                
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
            while (i < loc && data[start] != lov[i]->opcode())
                ++i;
                //  test term for operator
            
            if (!i)
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
                
                size_t k = 0;
                while (k < loc && lov[k]->opcode() != data[j])
                    ++k;
                    //  test terms subsequent binary operator for operators
                
                if (k == loc) {
                    start = j + 1;
                    break;
                }
                //  operand
                
                if (k) {
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
                size_t k = j - 1;
                while (k > 0 && data[k] != lov[0]->opcode())
                    --k;
                
                throw invalid_argument("Syntax error on token \"" + data[k] + "\", expression expected after this token");
            }
                //  expression ends in operator
        }
    }

    double logic::evaluate(const string expression) {
        if (expression.empty())
            throw invalid_argument("empty");
        
        if (balance(expression) > 0)
            throw invalid_argument("Syntax error, insert \")\" to complete expr body");
        
        if (balance(expression) < 0)
            throw invalid_argument("Syntax error on token \")\", delete this token");
        
        string* data = new string[expression.length() * 2];
        
        size_t n = prefix(data, expression);
        
        stack<string> s = stack<string>();
        
        for (int i = (int)n - 1; i >= 0; --i) {
            size_t j = 0;
            while (j < loc && lov[j]->opcode() != data[i])
                ++j;
            
            if (j == loc)
                s.push(data[i]);
            else {
                double d;
                
                if (!j) {
                    ulo* u = (ulo *)lov[j];
                                
                    d = u->apply(s.top());
                    
                    s.pop();
                } else {
                    string lhs = s.top();   s.pop();
                    
                    blo* b = (blo *)lov[j];
                    
                    if (j == loc - 1) {
                        d = arithmetic::evaluate(s.top());
                        
                        s.pop();
                        
                        set_number(lhs, d);
                        
                    } else {
                        d = b->apply(lhs, s.top());
                        
                        s.pop();
                    }
                }
                
                s.push(to_string(d));
            }
        }
        
        delete[] data;
        
        return arithmetic::evaluate(s.top());
    }

    size_t logic::merge(int n, string* data) const {
        int i;
        for (i = 0; i < n - 1; ++i) {
            if (data[i + 1] == lov[loc - 1]->opcode()) {
                //  =
                
                if (data[i] == lov[0]->opcode() || data[i] == lov[3]->opcode()) {
                    data[i] += data[i + 1];
                    
                    for (size_t j = i + 1; j < n - 1; ++j)
                        swap(data[j], data[j + 1]);
                    --n;
                    
                    if (i > 0) {
                        data[i - 1] += data[i];
                        
                        for (size_t j = i; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        --n;
                        --i;
                    }
                    
                    if (i < n - 1) {
                        data[i] += data[i + 1];
                        
                        for (size_t j = i + 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        --n;
                    }
                    
                    //  ==, !=
                } else {
                    size_t j;
                    for (j = 15; j < 17; ++j) {
                        if (data[i].length() >= aov[j]->opcode().length()) {
                            string opcode = aov[j]->opcode();
                            
                            size_t k = 0;
                            while (k < opcode.length() - 1 && opcode[k] == data[i][data[i].length() - opcode.length() + k + 1])
                                ++k;
                                                
                            if (k == opcode.length() - 1) {
                                for (size_t l = 0; l < 2; ++l) {
                                    data[i] += data[i + 1];
                                    
                                    for (size_t m = i + 1; m < n - 1; ++m)
                                        swap(data[m], data[m + 1]);
                                    
                                    --n;
                                }
                                
                                break;
                            }
                            //  <=, >=
                                
                        }
                    }
                    
                    if (j == 17) {
                        size_t k;
                        for (k = 25; k < aoc - 1; ++k) {
                            if (data[i].length() >= aov[k]->opcode().length() - 1) {
                                string opcode = aov[k]->opcode();
                                                        
                                size_t l = 0;
                                while (l < opcode.length() - 1 && opcode[l] == data[i][data[i].length() - opcode.length() + l + 1])
                                    ++l;
                                
                                if (l == opcode.length() - 1) {
                                    for (size_t m = 0; m < 2; ++m) {
                                        data[i] += data[i + 1];
                                        
                                        for (size_t p = i + 1; p < n - 1; ++p)
                                            swap(data[p], data[p + 1]);
                                        
                                        --n;
                                    }
                                    
                                    break;
                                }
                                //  compound assignment
                            }
                        }
                    }
                }
            }
                
        }
        
        for (i = 1; i < n - 1; ++i) {
            if (data[i] == lov[loc - 1]->opcode()) {
                //  =
                
                int l = (int)data[i - 1].length() - 1;
                int p = -1;
                
                do {
                    if (data[l] == "(")
                        ++p;
                    
                    else if (data[l] == ")")
                        --p;
                    
                    if (!p)
                        break;
                    
                    --l;
                    
                } while (l >= 0);
                
                if (l == -1) {
                    p = 1;
                    size_t r = 0;
                    
                    do {
                        if (data[i + 1][r] == '(')
                            ++p;
                        
                        else if (data[i + 1][r] == ')')
                            --p;
                        
                        if (!p)
                            break;
                        
                        ++r;
                        
                    } while (r < data[i + 1].length());
                    
                    while (r < data[i + 1].length() && data[i + 1][r] == ')')
                        ++r;
                    
                    if (r != data[i + 1].length()) {
                        r += data[i].length();
                        data[i] += data[i + 1];
                        
                        for (size_t j = i + 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --n;
                        
                        do {
                            r += data[i - 1].length();
                            data[i] = data[i - 1] + data[i];
                            
                            for (size_t j = i - 1; j < n - 1; ++j)
                                swap(data[j], data[j + 1]);
                            
                            --i;    --n;
                            
                        } while (balance(data[i], 0, r + 1) < 0);
                        
                        while (balance(data[i], r + 1, data[i].length()) > 0) {
                            data[i] += data[i + 1];
                            
                            for (size_t j = i + 1; j < n - 1; ++j)
                                swap(data[j], data[j + 1]);
                            --n;
                        }
                    }
                } else {
                    data[i] = data[i - 1] + data[i];
                    
                    for (size_t j = i - 1; j < n - 1; ++j)
                        swap(data[j], data[j + 1]);
                    
                    --i;    --n;
                    
                    while (i > 0 && data[i - 1] == "(") {
                        data[i] = "(" + data[i];
                        
                        for (size_t j = i - 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --i;    --n;
                    }
                    
                    do {
                        data[i] += data[i + 1];
                        
                        for (size_t j = i + 1; j < n - 1; ++j)
                            swap(data[j], data[j + 1]);
                        
                        --n;
                            
                    } while (balance(data[i]) > 0);
                }
            }
        }
        
        return n;
    }

    size_t logic::prefix(string* data, const string expr) const {
        if (expr.empty())
            throw invalid_argument("empty");
        
        size_t n = split(data, expr);
        
        analyze(n, data);
        
        size_t i;
        for (i = 1; i < loc; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                if (data[j] == lov[i]->opcode()) {
                    size_t l = j;   int p = 0;
                    
                    do {
                        --l;
                        
                        if (data[l] == "(")
                            ++p;
                        
                        else if (data[l] == ")")
                            --p;
                        
                    } while (l > 0 && p);
                    
                    while (l > 0 && data[l - 1] == lov[0]->opcode())
                        --l;
                    
                    data[n] = "(";
                    
                    for (size_t k = n; k > l; --k)
                        swap(data[k], data[k - 1]);
                    
                    n++;
                    
                    size_t r = ++j;
                    
                    while (r < n - 1 && data[r + 1] == lov[0]->opcode())
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
                    
                    for (size_t k = n; k > r + 1; --k)
                        swap(data[k], data[k - 1]);
                    
                    ++n;
                    
                    for (size_t k = j; k > l + 1; --k)
                        swap(data[k], data[k - 1]);
                    
                    ++i;
                }
            }
        }
        
        i = 0;
        while (i < n) {
            if (data[i] == "(" || data[i] == ")") {
                for (size_t j = i; j < n - 1; ++j)
                    swap(data[j], data[j + 1]);
                
                --n;
            } else
                ++i;
        }
        
        return n;
    }

    size_t logic::split(string* data, string expr) const {
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
            for (j = 0; j < loc; ++j) {
               if (i + lov[j]->opcode().length() <= expr.length()) {
                   int k = 0;
                   while (k < lov[j]->opcode().length() && lov[j]->opcode()[k] == expr[i + k])
                       ++k;
                   
                   if (k == lov[j]->opcode().length()) {
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
                       
                       data[n++] = lov[j]->opcode();
                       
                       start = i + k;
                       i = start;
                       
                       break;
                   }
               }
            }
            
            if (j == loc)
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
            for (size_t j = start + 1; j < end - 1; ++j) {
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
        
        return merge((int)n, data);
    }
}
