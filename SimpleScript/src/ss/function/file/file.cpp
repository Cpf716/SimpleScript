//
//  file.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/11/23.
//

#include "file.h"

namespace ss {
    //  CONSTRUCTORS

    file::file(const string filename, node<string>* parent, command_processor* cp) {
#if DEBUG_LEVEL
        assert(filename.length());
        assert(parent != NULL);
        assert(cp != NULL);
#endif
        ifstream file;
        
        file.open(filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
    
        this->rename(basename(filename));
        
        size_t n = 0;
        string* src = new string[1];
        
        string line;
        while (getline(file, line)) {
            string tokenv[line.length() + 1];
            size_t tokenc = parse(tokenv, line, "//");
            
            if (tokenc == 1 && tokenv[0].empty())
                continue;
            
            if (is_pow(n, 2))
                src = resize(n, n * 2, src);
            
            src[n++] = tokenv[0];
        }
        
        size_t i = 0;
        while (i < n) {
            string* tokenv = new string[pow_2(src[i].length() + 1)];
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
        
        i = 0;
        while (i < n) {
            string tokenv[src[i].length() + 1];
            size_t tokenc = tokens(tokenv, src[i]);
            
            if (!tokenc || tokenv[0] == "for") {
                ++i;
                continue;
            }
            
            tokenc = parse(tokenv, src[i], ";");
             
            size_t j = 0;
            while (j < tokenc) {
                if (tokenv[j].empty()) {
                    for (size_t k = j; k < tokenc - 1; ++k)
                        swap(tokenv[k], tokenv[k + 1]);
                    
                    --tokenc;
                } else
                    ++j;
            }
            
            if (j) {
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
            } else {
                for (size_t k = i; k < n - 1; ++k)
                    swap(src[k], src[k + 1]);
                
                --n;
            }
        }
        
        ::node<string>* node = new ::node<string>(filename, parent);
        
        ss::array<string> filev;
        ss::array<module_t> modulev;
        
        size_t state = cp->get_state();
        
        cp->set_state();
        
        this->filev = new pair<class file*, bool>*[1];
        
        for (i = 0; i < n; ++i) {
            string tokenv[src[i].length() + 1];
            size_t tokenc = tokens(tokenv, src[i]);
            
            if (tokenv[0] == "include") {
                if (tokenc == 1 || tokenv[1] != "(")
                    expect_error("'(' in 'function' call");
                
                if (tokenv[tokenc - 1] != ")")
                    expect_error("expression");
                
                tokenc = tokens(tokenv, trim_start(src[i].substr(7)));
                
                size_t j;
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
                        for (size_t k = e; k > s + 1; --k) {
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
                    
                    for (size_t k = s + 1; k < tokenc - 1; ++k)
                        swap(tokenv[k], tokenv[k + 1]);
                    
                    --tokenc;
                }
                
                size_t k;
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
                
                if (tokenc >= 3)
                    expect_error("2 argument(s), got " + std::to_string(tokenc));
                
                tokenv[0] = cp->evaluate(tokenv[0]);
                
                if (ss::is_array(tokenv[0]))
                    type_error(array_t, string_t);
                
                if (tokenv[0].empty())
                    null_error();
                
                if (!is_string(tokenv[0]))
                    type_error(double_t, string_t);
                
                tokenv[0] = decode(tokenv[0]);
                
                if (tokenv[0].empty())
                    undefined_error(encode(empty()));
                
                if (tokenv[0] == "fileSystem") {
                    if (tokenc == 2)
                        expect_error("1 argument(s), got 2");
                    
                    if (this->is_file_system) {
                        if (modulev.index_of(filesystem_t) == -1) {
                            modulev.push(filesystem_t);
                         
                            logger_write("'fileSystem' is defined\n");
                        }
                    } else {
                        init_file_system();
                        
                        this->is_file_system = true;
                    }
                    
                    continue;
                }
                
                if (tokenv[0] == "mysql") {
                    if (tokenc == 2)
                        expect_error("1 argument(s), got 2");
                    
                    if (this->is_mysql) {
                        if (modulev.index_of(mysql_t) == -1) {
                            modulev.push(mysql_t);
                         
                            logger_write("'mysql' is defined\n");
                        }
                    } else {
                        init_mysql();
                        
                        this->is_mysql = true;
                    }
                    
                    continue;
                }
                
                if (tokenv[0] == "socket") {
                    if (tokenc == 2)
                        expect_error("1 argument(s), got 2");
                    
                    if (this->is_socket) {
                        if (modulev.index_of(socket_t) == -1) {
                            modulev.push(socket_t);
                         
                            logger_write("'socket' is defined\n");
                        }
                    } else {
                        init_socket();
                        
                        this->is_socket = true;
                    }
                    
                    continue;
                }
                
                if (tokenv[0].length() > 1 && tokenv[0][0] == '@' && tokenv[0][1] == '/') {
                    string tmp = get_base_dir() + tokenv[0].substr(2);
                    
                    if (tokenv[0][tokenv[0].length() - 1] != '/')
                        tmp += ".txt";
                    
                    tokenv[0] = tmp;
                }
                
                vector<string> dst;
                
                read_dirs(dst, tokenv[0]);
                
                if (!dst.size())
                    throw error("No such file: " + tokenv[0]);
                
                if (tokenc == 2) {
                    if (dst.size() != 1)
                        logger_write(dst[0] + " is a directory\n");
                    
                    tokenv[1] = cp->evaluate(tokenv[1]);
                    
                    if (ss::is_array(tokenv[1]))
                        type_error(array_t, string_t);
                    
                    if (tokenv[1].empty())
                        null_error();
                    
                    if (!is_string(tokenv[1]))
                        type_error(double_t, string_t);
                    
                    tokenv[1] = decode(tokenv[1]);
                    
                    string basename = ::basename(dst[0]);
                    
                    if (tokenv[1].length()) {
                        string valuev[1024];
                        size_t valuec = read(valuev, dst[0], tokenv[1]);
                        
                        this->valuev.push_back(pair<string, string>(basename, stringify(valuec, valuev)));
                    } else
                        this->valuev.push_back(pair<string, string>(basename, encode_raw(read(dst[0]))));
                    
                    continue;
                }
                
                for (j = 0; j < dst.size(); ++j) {
                    string basename = ::basename(dst[j]);
                    
                    size_t k = 0;
                    while (k < this->filec && this->filev[k]->first->name() != basename)
                        ++k;
                    
                    if (k != this->filec) {
                        if (filev.index_of(basename) == -1) {
                            logger_write("'" + basename + "' is defined\n");

                            filev.push(basename);
                        }

                        continue;
                    }
                    
                    //  tree ensures file cannot include itself
                    ::node<string>* _parent = parent;
                    
                    while (_parent != NULL) {
                        if (_parent->data() == node->data())
                            throw error("Recursive import: " + _parent->data());
                        
                        _parent = _parent->parent();
                    }
                    
                    if (is_pow(this->filec, 2)) {
                        pair<::file*, bool>** tmp = new pair<::file*, bool>*[this->filec * 2];
                        
                        for (size_t k = 0; k < this->filec; ++k)
                            tmp[k] = this->filev[k];
                        
                        delete[] this->filev;
                        
                        this->filev = tmp;
                    }
                    
                    size_t _state = cp->get_state();
                    
                    cp->set_state();
                    
                    ::file* _file = new ::file(dst[j], node, cp);
                    
                    _file->parent = this;
                    
                    this->filev[this->filec] = new pair<::file*, bool>(_file, true);
                    
                    if (this->filev[this->filec]->first->is_file_system && !this->is_file_system) {
                        init_file_system();
                        
                        this->is_file_system = true;
                    }
                    
                    if (this->filev[this->filec]->first->is_mysql && !this->is_mysql) {
                        init_mysql();
                        
                        this->is_mysql = true;
                    }
                    
                    if (this->filev[this->filec]->first->is_socket && !this->is_socket) {
                        init_socket();
                        
                        this->is_socket = true;
                    }
                    
                    cp->set_state(_state);
                    
                    size_t filec = this->filec++;
                    for (size_t k = 0; k < this->filev[filec]->first->filec; ++k) {
                        size_t l = 0;
                        while (l < this->filec && this->filev[filec]->first->filev[k]->first->name() != this->filev[l]->first->name())
                            ++l;
                        
                        if (l == this->filec) {
                            if (is_pow(this->filec, 2)) {
                                pair<::file*, bool>** tmp = new pair<::file*, bool>*[this->filec * 2];
                                for (size_t m = 0; m < this->filec; ++m)
                                    tmp[m] = this->filev[m];
                                
                                delete[] this->filev;
                                
                                this->filev = tmp;
                            }
                            
                            this->filev[this->filec] = new pair<::file*, bool>(this->filev[filec]->first->filev[k]->first, false);
                            this->filev[this->filec]->first->consume();
                            
                            ++this->filec;
                        } else
                            //  different instance than filev[k]->first
                            this->filev[filec]->first->filev[k]->first->consume();
                    }
                    
                }
                
                continue;
            }
                
            break;
        }
        
        cp->set_state(state);
        
        for (; i > 0; --i) {
            for (size_t j = i - 1; j < n - 1; ++j)
                swap(src[j], src[j + 1]);
            
            --n;
        }
        
        if (!n) {
            if (!this->filec && !this->is_file_system &&
                !this->is_mysql && !this->is_socket)
                logger_write("'file' statement has empty body\n");
            
            else
                //  config. file
                this->consume();
        }
        
        statement_t** statementv = new statement_t*[n];
        size_t statementc = this->build(statementv, src, 0, n);
        
        delete[] src;
        
        this->filename = filename;
        this->set_cp(cp);
        this->target = new file_statement(this, statementc, statementv);
    }

    void file::close() {
        for (size_t i = 0; i < this->filec; ++i) {
            if (this->filev[i]->second)
                this->filev[i]->first->close();
            
            delete this->filev[i];
        }
        
        delete[] this->filev;
        
        this->target->close();
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
            
            if (tokenv[0] == "case") {
                delete[] tokenv;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "switch") {
                        delete[] tokenv;
                        ++p;
                    } else {
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "switch") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "case" || tokenv[0] == "default") {
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
                
                dst[s++] = new case_statement(trim_start(src[i].substr(4)), _s, _dst);
                
                i = k;
            } else if (tokenv[0] == "catch") {
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
                
                dst[s++] = new catch_statement(trim_start(src[i].substr(5)), _s, _dst);
                i = k;
            } else if (tokenv[0] == "default") {
                delete[] tokenv;
                
                if (tokenc > 1)
                    expect_error("';' after expression");
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    if (tokenv[0] == "switch")
                        ++p;
                    else {
                        tokenv = new string[src[k].length() + 1];
                        tokenc = tokens(tokenv, src[k]);
                        
                        if (tokenc > 1 && tokenv[0] == "end" && tokenv[1] == "switch") {
                            delete[] tokenv;
                            
                            --p;
                            
                            if (!p)
                                break;
                        } else if (p == 1) {
                            if (tokenv[0] == "case" || tokenv[0] == "default") {
                                delete[] tokenv;
                                expect_error("'end switch'");
                            }
                            
                            delete[] tokenv;
                        } else
                            delete[] tokenv;
                    }
                }
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new default_statement(_s, _dst);
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
                
                dst[s++] = new do_while_statement(trim_start(src[i].substr(j)), _s, _dst);
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
                
                dst[s++] = new else_if_statement(trim_start(src[i].substr(j)), _s, _dst);
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
                
                dst[s++] = new for_statement(trim_start(src[i].substr(3)), _s, _dst);
                
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
                
                dst[s++] = new function_statement(trim_start(src[i].substr(4)), _s, _dst);
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
                
                dst[s++] = new if_statement(trim_start(src[i].substr(2)), _s, _dst);
                
                i = k + 1;
            } else if (tokenv[0] == "switch") {
                delete[] tokenv;
                
                size_t k;   int p = 1;
                for (k = i + 1; k < end; ++k) {
                    tokenv = new string[src[k].length() + 1];
                    tokenc = tokens(tokenv, src[k]);
                    
                    if (tokenv[0] == "switch") {
                        delete[] tokenv;
                        ++p;
                    } else if (tokenc >= 2 && tokenv[0] == "end" && tokenv[1] == "switch") {
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
                    expect_error("'end switch'");
                
                statement_t** _dst = new statement_t*[k - i - 1];
                size_t _s = build(_dst, src, i + 1, k);
                
                dst[s++] = new switch_statement(trim_start(src[i].substr(6)), _s, _dst);
                
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
                
                dst[s++] = new while_statement(trim_start(src[i].substr(5)), _s, _dst);
                i = k + 1;
            } else {
                if (tokenv[0] == "assert") {
                    delete[] tokenv;
                    dst[s] = new assert_statement(trim_start(src[i].substr(6)));
                } else if (src[i] == "break") {
                    delete[] tokenv;
                    dst[s] = new break_statement();
                } else if (tokenv[0] == "suppress") {
                    delete[] tokenv;
                    dst[s] = new suppress_statement(trim_start(src[i].substr(8)));
                } else if (src[i] == "continue") {
                    delete[] tokenv;
                    dst[s] = new continue_statement();
                } else if (tokenv[0] == "define") {
                    delete[] tokenv;
                    dst[s] = new define_statement(trim_start(src[i].substr(6)));
                } else if (tokenv[0] == "echo") {
                    delete[] tokenv;
                    dst[s] = new echo_statement(trim_start(src[i].substr(4)));
                } else if (src[i] == "exit") {
                    delete[] tokenv;
                    dst[s] = new exit_statement();
                } else if (tokenv[0] == "return") {
                    delete[] tokenv;
                    dst[s] = new return_statement(trim_start(src[i].substr(6)));
                } else if (tokenv[0] == "sleep") {
                    delete[] tokenv;
                    dst[s] = new sleep_statement(trim_start(src[i].substr(5)));
                } else if (tokenv[0] == "throw") {
                    delete[] tokenv;
                    dst[s] = new throw_statement(trim_start(src[i].substr(5)));
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
        size_t state = this->cp->get_state();
        
        this->cp->set_state();
        
        size_t _state = this->cp->get_state();
        
        for (size_t i = 0; i < this->valuev.size(); ++i) {
            if (ss::is_array(this->valuev[i].second))
                this->cp->set_array(this->valuev[i].first, this->valuev[i].second);
                
            else if (this->valuev[i].second.empty() || is_string(this->valuev[i].second))
                this->cp->set_string(this->valuev[i].first, this->valuev[i].second);
            else
                this->cp->set_number(this->valuev[i].first, stod(this->valuev[i].second));
        }
        
        if (this->is_file_system)
            set_file_system(cp);
        
        if (this->is_mysql)
            set_mysql(cp);
        
        if (this->is_socket)
            set_socket(cp);

        this->cp->set_function(this);
        
        for (size_t i = 0; i < this->filec; ++i)
            this->cp->set_function(this->filev[i]->first);
        
        ss::array<string> arr = this->marshall(argc, argv);
        
        this->cp->set_array("argv", stringify(arr));
        this->cp->evaluate("shrink argv");
        this->cp->consume("argv");
        
        this->should_pause = false;
        
        string result = this->target->execute(this->cp);
        
//        while (this->should_pause);
        
        this->consume();
        this->cp->set_state(_state);
        this->cp->set_state(state);
        
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
        
        for (size_t i = 0; i < this->filec; ++i)
            if (this->filev[i]->second)
                this->filev[i]->first->kill();
    }

    ss::array<string> file::marshall(const size_t argc, string* argv) const {
        ss::array<string> data = ss::array<string>(argc * 2 + 1);
        
        size_t j = 0;
        for (size_t i = 0; i < argc; ++i) {
            string valuev[argv[i].length() + 1];
            size_t valuec = parse(valuev, argv[i]);
            
            if (valuec > j)
                j = valuec;
            
            data.push(std::to_string(valuec));
            
            for (size_t k = 0; k < valuec; ++k)
                data.push(valuev[k]);
        }
        
        for (size_t i = 0; i < argc; ++i) {
            size_t k = stoi(data[i * (j + 1)]);
            
            for (size_t l = 0; l < j - k; ++l)
                data.insert(i * (j + 1) + k + l + 1, empty());
        }
        
        data.insert(0, std::to_string(j + 1));
        
        return data;
    }

    void file::set_cp(command_processor* cp) {
        this->cp = cp;
    }

    void file::set_level(const size_t level) {
        this->level = level + 1;
        
        if (this->parent != NULL)
            this->parent->set_level(this->get_level());
    }

    void file::set_pause(const bool pause) {
        this->should_pause = pause;
        
        this->target->set_pause(pause);
        
        for (size_t i = 0; i < this->filec; ++i)
            if (this->filev[i]->second)
                this->filev[i]->first->set_pause(pause);
    }
}
