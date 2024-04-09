//
//  file.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/11/23.
//

#include "file.h"

namespace ss {
    //  CONSTRUCTORS

    file::file(const string filename, node<string>* parent, interpreter* ssu) {
        ifstream file;
        file.open(filename);
        
        if (!file.is_open())
            throw error("No such file: " + filename);
    
        this->rename(::filename(filename));
        
        size_t n = 0;
        string* src = new string[1];
        
        //  line breaks take supreme precedence
        string line;
        while (getline(file, line)) {
            string tokenv[line.length() + 1];
            size_t tokenc = tokenize(tokenv, line, "//");
            
            tokenc = merge(tokenc, tokenv, empty());
            
            if (tokenc == 0)
                continue;
            
            size_t i = 0;
            while (i < tokenc && tokenv[i] != "//")
                ++i;
            
            if (i == 0)
                continue;
            
            while (tokenc > i)
                --tokenc;
            
            line = trim(tokenv[0]);
            
            for (size_t i = 1; i < tokenc; ++i)
                line += trim(tokenv[i]);
            
            if (is_pow(n, 2))
                src = resize(n, n * 2, src);
            
            src[n++] = line;
        }
        
        //  block comments are third
        size_t i = 0;
        while (i < n) {
            string* tokenv = new string[pow2(src[i].length() + 1)];
            size_t tokenc = tokenize(tokenv, src[i], "/*");
            
            tokenc = merge(tokenc, tokenv, empty());
            
            size_t j = 0;
            while (j < tokenc) {
                string _tokenv[tokenv[j].length() + 1];
                size_t _tokenc = tokenize(_tokenv, tokenv[j], "*/");
                
                _tokenc = merge(_tokenc, _tokenv, empty());
                tokenv[j] = trim(_tokenv[0]);
                
                for (size_t k = 1; k < _tokenc; ++k) {
                    if (is_pow(tokenc, 2))
                        tokenv = resize(tokenc, tokenc * 2, tokenv);
                    
                    tokenv[tokenc] = trim(_tokenv[k]);
                    
                    for (size_t l = tokenc; l > j + k; --l)
                        swap(tokenv[l], tokenv[l - 1]);
                    
                    ++tokenc;
                }
                
                j += _tokenc;
            }
            
            size_t _tokenc = tokenc;
            
            j = 0;
            while (j < tokenc) {
                if (tokenv[j] == "/*") {
                    size_t k = j + 1;
                    while (k < tokenc && tokenv[k] != "*/")
                        ++k;
                    
                    if (k == tokenc)
                        break;
                    else {
                        for (size_t l = j; l <= k; ++l) {
                            for (size_t m = j; m < tokenc - 1; ++m)
                                swap(tokenv[m], tokenv[m + 1]);
                            
                            --tokenc;
                        }
                    }
                } else
                    ++j;
            }
            
            if (tokenc != _tokenc) {
                while (tokenc > 1) {
                    tokenv[0] += tokenv[1];
                    
                    for (j = 1; j < tokenc - 1; ++j)
                        swap(tokenv[j], tokenv[j + 1]);
                    
                    --tokenc;
                }
            }
            
            if (!tokenc) {
                for (size_t j = i; j < n - 1; ++j)
                    swap(src[j], src[j + 1]);
                
                --n;
            } else {
                src[i] = tokenv[0];
                
                for (j = 1; j < tokenc; ++j) {
                    if (is_pow(n, 2))
                        src = resize(n, n * 2, src);
                    
                    src[n] = tokenv[j];
                    
                    for (size_t k = n; k > i + j; --k)
                        swap(src[k], src[k - 1]);
                    
                    ++n;
                }
                
                i += tokenc;
            }
            
            delete[] tokenv;
        }
        
        i = 0;
        while (i < n) {
            if (src[i] == "/*") {
                size_t j = i + 1;
                while (j < n && src[j] != "*/")
                    ++j;
                
                if (j == n) {
                    logger_write("Missing terminating '*/' character\n");
                    --j;
                }
                
                for (size_t k = i; k <= j; ++k) {
                    for (size_t l = i; l < n - 1; ++l)
                        swap(src[l], src[l + 1]);
                    
                    --n;
                }
            } else
                ++i;
        }
        
        i = 0;
        while (i < n && src[i] != "*/")
            ++i;
        
        if (i != n)
            throw error("Unexpected token: */");
        
        //  semicolons are fourth
        i = 0;
        while (i < n) {
            string tokenv[src[i].length() + 1];
            size_t tokenc = parse(tokenv, src[i], ";");
             
            size_t j = 0;
            while (j < tokenc) {
                if (tokenv[j].empty()) {
                    for (size_t k = j; k < tokenc - 1; ++k)
                        swap(tokenv[k], tokenv[k + 1]);
                    
                    --tokenc;
                } else
                    ++j;
            }
            
            if (!j) {
                for (size_t k = i; k < n - 1; ++k)
                    swap(src[k], src[k + 1]);
                
                --n;
            } else {
                src[i] = tokenv[0];
                
                for (size_t j = 1; j < tokenc; ++j) {
                    if (is_pow(n, 2))
                        src = resize(n, n * 2, src);
                    
                    src[n] = tokenv[j];
                    
                    for (size_t k = n; k > i + j; --k)
                        swap(src[k], src[k - 1]);
                    
                    ++n;
                }
                
                i += tokenc;
            }
        }
        
        ::node<string>* node = new ::node<string>(filename, parent);
        
        ss::array<string> arr;
        
        string buid = ssu->backup();
        
        ssu->reload();
        
        this->functionv = new pair<class file*, bool>*[1];
        
        for (i = 0; i < n; ++i) {
            string tokenv[src[i].length() + 1];
            size_t tokenc = tokens(tokenv, src[i]);
            
            if (tokenv[0] == "include") {
                if (tokenc == 1)
                    expect_error("expression");
                
                string value = ltrim(src[i].substr(7));
                
                if (value[0] != '(') {
                    cout << value << endl;
                    
                    expect_error("1 argument(s), got 0");
                }
                
                size_t j = 1, k = 1;
                for (; j < value.length(); ++j) {
                    if (value[j] == '(')
                        ++k;
                    else if (value[j] == ')')
                        --k;
                    
                    if (!k)
                        break;
                }
                
                if (k || j != value.length() - 1)
                    expect_error("expression");
                
                tokenc = tokens(tokenv, value);
                
                for (j = 0; j < tokenc - 1; ++j)
                    swap(tokenv[j], tokenv[j + 1]);
                
                --tokenc;
                --tokenc;
                
                size_t e = 0, s = e;   j = 0;
                for (; e < tokenc; ++e) {
                    if (tokenv[e] == "(")
                        ++j;
                    else if (tokenv[e] == ")")
                        --j;
                    else if (!j && tokenv[e] == ",") {
                        for (k = e; k > s + 1; --k) {
                            tokenv[s] += " " + tokenv[s + 1];
                            
                            for (size_t l = s + 1; l < tokenc - 1; ++l)
                                swap(tokenv[l], tokenv[l + 1]);
                            
                            --e;
                            --tokenc;
                        }
                        
                        s = e + 1;
                    }
                }
                
                for (j = e; j > s + 1; --j) {
                    tokenv[s] += " " + tokenv[s + 1];
                    
                    for (k = s + 1; k < tokenc - 1; ++k)
                        swap(tokenv[k], tokenv[k + 1]);
                    
                    --tokenc;
                }
                
                for (j = 0, k = (size_t)floor(tokenc / 2) + 1; j < k; ++j)
                    if (tokenv[j * 2] == ",")
                        throw error("Underflow");
                
                if (tokenv[tokenc - 1] == ",")
                    throw error("Underflow");
                
                for (j = 0, k = (size_t)floor(tokenc / 2); j < k; ++j) {
                    for (size_t l = j + 1; l < tokenc - 1; ++l)
                        swap(tokenv[l], tokenv[l + 1]);
                    
                    --tokenc;
                }
                
                if (!tokenc)
                    expect_error("1 argument(s), got 0");
                
                if (tokenc > 2)
                    expect_error("2 argument(s), got " + to_string(tokenc));
                
                tokenv[0] = ssu->evaluate(tokenv[0]);
                
                if (ss::is_array(tokenv[0]))
                    type_error("array", "string");
                
                if (tokenv[0].empty())
                    null_error();
                
                if (!is_string(tokenv[0]))
                    type_error("double", "string");
                
                tokenv[0] = decode(tokenv[0]);
                
                if (tokenc == 1)
                    tokenv[tokenc++] = ::filename(tokenv[0]);
                
                else if (!is_symbol(tokenv[1]))
                    expect_error("symbol in 'include' statement specificer");

                j = 0;
                while (j < this->functionc && this->functionv[j]->first->name() != tokenv[1])
                    ++j;
                
                if (j != this->functionc) {
                    if (arr.index_of(tokenv[1]) == -1) {
                        logger_write("'" + tokenv[1] + "' is defined\n");

                        arr.push(tokenv[1]);
                    }

                    continue;
                }
                
                //  tree ensures file cannot include itself
                ::node<string>* _parent = parent;
                
                while (_parent != NULL) {
                    if (_parent->data() == node->data())
                        throw error(_parent->data());
                    
                    _parent = _parent->parent();
                }
                    
                if (is_pow(this->functionc, 2)) {
                    pair<::file*, bool>** tmp = new pair<::file*, bool>*[this->functionc * 2];
                    for (size_t k = 0; k < this->functionc; ++k)
                        tmp[k] = this->functionv[k];
                    
                    delete[] this->functionv;
                    this->functionv = tmp;
                }
                
                if (tokenv[0].length() > 1 && tokenv[0][0] == '@' && tokenv[0][1] == '/')
                    tokenv[0] = base_dir() + tokenv[0].substr(2) + ".txt";
                
                string _buid = ssu->backup();
                
                ssu->reload();
                
                ::file* file = new ::file(tokenv[0], node, ssu);
                
                file->parent = this;
                file->rename(tokenv[1]);
                
                this->functionv[this->functionc] = new pair<::file*, bool>(file, true);
                
                ssu->restore(_buid);
                
                size_t functionc = this->functionc++;
                for (j = 0; j < this->functionv[functionc]->first->functionc; ++j) {
                    size_t k = 0;
                    while (k < this->functionc && this->functionv[functionc]->first->functionv[j]->first->name() != this->functionv[k]->first->name())
                        ++k;
                    
                    if (k == this->functionc) {
                        if (is_pow(this->functionc, 2)) {
                            pair<::file*, bool>** tmp = new pair<::file*, bool>*[this->functionc * 2];
                            for (size_t l = 0; l < this->functionc; ++l)
                                tmp[l] = this->functionv[l];
                            
                            delete[] this->functionv;
                            this->functionv = tmp;
                        }
                        
                        this->functionv[this->functionc] = new pair<::file*, bool>(this->functionv[functionc]->first->functionv[j]->first, false);
                        this->functionv[this->functionc]->first->consume();
                        
                        ++this->functionc;
                    } else
                        //  different instance than functionv[k]->first
                        this->functionv[functionc]->first->functionv[j]->first->consume();
                }
                
                continue;
            }
                
            break;
        }
        
        ssu->restore(buid);
        
        for (; i > 0; --i) {
            for (size_t j = i - 1; j < n - 1; ++j)
                swap(src[j], src[j + 1]);
            
            --n;
        }
        
        if (!n) {
            if (!this->functionc)
                logger_write("'file' statement has empty body\n");
            
            else
                //  configuration file
                this->consume();
        }
        
        statement_t** statementv = new statement_t*[n];
        size_t statementc = this->build(statementv, src, 0, n);
        
        delete[] src;
        
        this->filename = filename;
        this->ssu = ssu;
        this->target = new file_statement(this, statementc, statementv);
    }

    void file::close() {
        for (size_t i = 0; i < this->functionc; ++i)
            if (this->functionv[i]->second)
                this->functionv[i]->first->close();
        
        delete[] this->functionv;
    }

    //  MEMBER FUNCTIONS

    size_t file::build(statement_t** dst, string* src, const size_t beg, const size_t end) const {
        size_t i = beg, s = 0;
        while (i < end) {
            string* tokenv = new string[src[i].length() + 1];
            size_t tokenc = tokens(tokenv, src[i]);
            
            if (tokenv[0] == "include") {
                delete[] tokenv;
                expect_error("expression");
            }
            
            if (tokenv[0] == "catch") {
                delete[] tokenv;
                
                if (tokenc > 2)
                    expect_error("';' after expression");
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "try") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "try") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "catch") {
                                delete[] tokenv;
                                expect_error("'end try'");
                            }
                            
                            if (tokenv[0] == "finally") {
                                delete[] tokenv;
                                break;
                            }
                            
                            delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new catch_statement(ltrim(src[i].substr(5)), _s, _dst);
                i = k;
            } else if (tokenc > 1 && tokenv[0] == "do" && tokenv[1] == "while") {
                delete[] tokenv;
                
                size_t j;
                for (j = 0; j <= src[i].length() - 5; ++j) {
                    size_t k = 0;
                    while (k < 5 && src[i][j + k] == ("while")[k])
                        ++k;
                    
                    if (k == 5)
                        break;
                }
                
                j += 5;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "while") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1) {
                            if (tokenv[0] == "do" && tokenv[1] == "while") {
                                delete[] tokenv;
                                ++p;
                            } else if (tokenv[0] == "end" && tokenv[1] == "while") {
                                delete[] tokenv;
                                
                                if (tokenc > 2)
                                    expect_error("';' after expression");
                                
                                --p;
                                
                                if (!p) break;
                            } else
                                delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                if (k == end)
                    expect_error("'end while'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new do_while_statement(ltrim(src[i].substr(j)), _s, _dst);
                i = k + 1;
            } else if (tokenc > 1 && tokenv[0] == "else" && tokenv[1] == "if") {
                delete[] tokenv;
                
                size_t j;
                for (j = 0; j <= src[i].length() - 2; ++j) {
                    size_t k = 0;
                    while (k < 2 && src[i][j + k] == ("if")[k])
                        ++k;
                    
                    if (k == 2)
                        break;
                }
                
                j += 2;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "if") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "if") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "else") {
                                delete[] tokenv;
                                break;
                            } else
                                delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                    
                }
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new else_if_statement(ltrim(src[i].substr(j)), _s, _dst);
                i = k;
            } else if (tokenv[0] == "else") {
                delete[] tokenv;
                
                if (tokenc > 1)
                    expect_error("';' after expression");
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "if") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "if") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "else") {
                                delete[] tokenv;
                                expect_error("'end if'");
                            }
                            
                            delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new else_statement(_s, _dst);
                i = k;
            } else if (tokenv[0] == "finally") {
                delete[] tokenv;
                
                if (tokenc > 1)
                    expect_error("';' after expression");
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    if (tokenv[0] == "try")
                        ++p;
                    else {
                        tokenv = new string[src[k].length() + 1];
                        tokenc = tokens(tokenv, src[k]);
                        
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "try") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "catch" || tokenv[0] == "finally") {
                                delete[] tokenv;
                                expect_error("'end try'");
                            }
                            
                            delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new finally_statement(_s, _dst);
                i = k;
            } else if (tokenv[0] == "for") {
                delete[] tokenv;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenc > 1 && tokenv[0] == "for") {
                        delete[] tokenv;
                        ++p;
                    } else if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "for") {
                        delete[] tokenv;
                        
                        if (tokenc > 2)
                            expect_error("';' after expression");
                        
                        --p;
                        
                        if (!p)
                            break;
                    } else
                        delete[] tokenv;
                }
                
                if (k == end)
                    expect_error("'end for'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new for_statement(ltrim(src[i].substr(3)), _s, _dst);
                
                i = k + 1;
            } else if (tokenv[0] == "func") {
                delete[] tokenv;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "func") {
                        delete[] tokenv;
                        ++p;
                    } else if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "func") {
                        delete[] tokenv;
                        
                        if (tokenc > 2)
                            expect_error("';' after expression");
                        
                        --p;
                        
                        if (!p)
                            break;
                    } else
                        delete[] tokenv;
                }
                
                if (k == end)
                    expect_error("'end func'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new function_statement(ltrim(src[i].substr(4)), _s, _dst);
                i = k + 1;
            } else if (tokenv[0] == "if") {
                delete[] tokenv;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "if") {
                        delete[] tokenv;
                        ++p;
                    } else if (tokenc >= 2 && tokenv[0] == "end" && tokenv[1] == "if") {
                        delete[] tokenv;
                        
                        if (tokenc > 2)
                            expect_error("';' after expression");
                        
                        --p;
                        
                        if (!p)
                            break;
                    } else
                        delete[] tokenv;
                }
                
                if (k == end)
                    expect_error("'end if'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new if_statement(ltrim(src[i].substr(2)), _s, _dst);
                
                i = k + 1;
            } else if (tokenv[0] == "try") {
                delete[] tokenv;
                
                if (tokenc > 1)
                    expect_error("';' after expression");
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "try") {
                        delete[] tokenv;
                        ++p;
                    } else if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "try") {
                        delete[] tokenv;
                        
                        if (tokenc > 2)
                            expect_error("';' after expression");
                        
                        --p;
                        
                        if (!p)
                            break;
                    } else
                        delete[] tokenv;
                }
                
                if (k == end)
                    expect_error("'end try'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new try_statement(_s, _dst);
                i = k + 1;
            } else if (tokenv[0] == "while") {
                delete[] tokenv;
        
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "while") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1) {
                            if (tokenv[0] == "do" && tokenv[1] == "while") {
                                delete[] tokenv;
                                ++p;
                            } else if (tokenv[0] == "end" && tokenv[1] == "while") {
                                delete[] tokenv;
                                
                                if (tokenc > 2)
                                    expect_error("';' after expression");
                                
                                --p;
                                
                                if (!p)
                                    break;
                            } else
                                delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                if (k == end)
                    expect_error("'end while'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new while_statement(ltrim(src[i].substr(5)), _s, _dst);
                i = k + 1;
            } else {
                if (tokenv[0] == "assert") {
                    delete[] tokenv;
                    dst[s] = new assert_statement(ltrim(src[i].substr(6)));
                } else if (src[i] == "break") {
                    delete[] tokenv;
                    dst[s] = new break_statement();
                } else if (tokenv[0] == "consume") {
                    delete[] tokenv;
                    dst[s] = new consume_statement(ltrim(src[i].substr(8)));
                } else if (src[i] == "continue") {
                    delete[] tokenv;
                    dst[s] = new continue_statement();
                } else if (tokenv[0] == "define") {
                    delete[] tokenv;
                    dst[s] = new define_statement(ltrim(src[i].substr(6)));
                } else if (tokenv[0] == "echo") {
                    delete[] tokenv;
                    dst[s] = new echo_statement(ltrim(src[i].substr(4)));
                } else if (src[i] == "exit") {
                    delete[] tokenv;
                    dst[s] = new exit_statement();
                } else if (tokenv[0] == "return") {
                    delete[] tokenv;
                    dst[s] = new return_statement(ltrim(src[i].substr(6)));
                } else if (tokenv[0] == "sleep") {
                    delete[] tokenv;
                    dst[s] = new sleep_statement(ltrim(src[i].substr(5)));
                } else if (tokenv[0] == "throw") {
                    delete[] tokenv;
                    dst[s] = new throw_statement(ltrim(src[i].substr(5)));
                } else {
                    delete[] tokenv;
                    dst[s] = new statement(src[i]);
                }
                
                ++i;
                ++s;
            }
        }
        
        return s;
    }

    string file::call(const size_t argc, string* argv) {
        string buid = this->ssu->backup();
        
        this->ssu->reload();
        
        string _buid = this->ssu->backup();
        
        this->ssu->set_function(this);
        
        for (size_t i = 0; i < this->functionc; ++i)
            this->ssu->set_function(this->functionv[i]->first);
        
        ss::array<string> arr = this->marshall(argc, argv);
        
        this->ssu->set_array("argv", stringify(arr));
        this->ssu->evaluate("shrink argv");
        this->ssu->consume("argv");
        
        string result = this->target->execute(this->ssu);
        
        this->consume();
        
        this->ssu->restore(_buid);
        this->ssu->restore(buid);
        
        return result;
    }

    void file::exit() {
        if (this->parent == NULL)
            this->kill();
        else
            this->parent->exit();
    }

    size_t file::get_level() const {
        return this->level;
    }

    void file::kill() {
        this->target->kill();
        
        for (size_t i = 0; i < this->functionc; ++i)
            if (this->functionv[i]->second)
                this->functionv[i]->first->kill();
    }

    ss::array<string> file::marshall(const size_t argc, string* argv) const {
        ss::array<string> data = ss::array<string>(argc * 2 + 1);
        
        size_t j = 1;
        for (size_t i = 0; i < argc; ++i) {
            string valuev[argv[i].length() + 1];
            size_t valuec = parse(valuev, argv[i]);
            
            if (valuec > j)
                j = valuec;
            
            data.push(to_string(valuec));
            
            for (size_t k = 0; k < valuec; ++k)
                data.push(valuev[k]);
        }
        
        for (size_t i = 0; i < argc; ++i) {
            size_t k = stoi(data[i * (j + 1)]);
            
            for (size_t l = 0; l < j - k; ++l)
                data.insert(i * (j + 1) + k + l + 1, empty());
        }
        
        data.insert(0, to_string(j + 1));
        data.insert(1, to_string(1));
        data.insert(2, encode(this->filename));
        
        for (size_t k = 1; k < j; ++k)
            data.insert(k + 2, empty());
        
        return data;
    }

    void file::set_level(const size_t level) {
        this->level = level + 1;
        
        if (this->parent != NULL)
            this->parent->set_level(this->get_level());
    }

    //  NON-MEMBER FUNCTIONS

    string base_dir() {
        return "/Library/Application Support/SimpleScript/ssl/";
    }
}
