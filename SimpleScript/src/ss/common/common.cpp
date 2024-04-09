//
//  common.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 11/4/22.
//

#include "common.h"

namespace ss {
    //  NON-MEMBER FIELDS

    const std::string delim[] = { "!", "(", ")", ",", ".", ":", ";" };

    const std::pair<std::string, char> escape[] = {
        std::pair<std::string, char>("\\n", '\n'),
        std::pair<std::string, char>("\\r", '\r'),
        std::pair<std::string, char>("\\t", '\t')
    };

    //  NON-MEMBER FUNCTIONS

    std::string decode(const std::string str) {
        int l = 0;
        while (l < str.length() && str[l] != '\"')
            ++l;
        
        if (l == str.length())
            return str;
            
        std::size_t n = str.length() + 1;
        char dst[n];
        
        strcpy(dst, str.c_str());
        
        for (std::size_t j = l; j < n - 1; ++j)
            std::swap(dst[j], dst[j + 1]);
        
        --n;
        
        int i = l, r = -1;
        while (i < n - 1) {
            if (dst[i] == '\"') {
                r = i + 1;
                while (r < n - 1 && dst[r] == '\"')
                    ++r;
                
                if ((r - i) % 2)
                    break;
                
                i = r;
            } else
                ++i;
        }
                
        if (i == n - 1) {
            logger_write("Missing terminating '\"' character: (" + str + ")\n");
            
            //  BEGIN
            
            for (int j = l; j < (int)n - 2; ++j) {
                std::size_t k;
                for (k = 0; k < sizeof(escape) / sizeof(escape)[0]; ++k) {
                    std::size_t m = 0;
                    while (m < 2 && dst[j + m] == escape[k].first[m])
                        ++k;
                    
                    if (m == 2) {
                        dst[j] = escape[k].second;
                        
                        for (std::size_t p = j + 1; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                        
                        break;
                    }
                }
            }
            
            //  END
            
            for (int j = l; j < (int)n - 1; ++j) {
                if (dst[j] == '\"') {
                    std::size_t k = j + 1;
                    while (k < n - 1 && dst[k] == '\"')
                        ++k;
                    
                    std::size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (std::size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                    }
                    
                    j += m;
                }
            }
        } else {
            for (std::size_t j = --r; j < n - 1; ++j)
                std::swap(dst[j], dst[j + 1]);
            
            --n;
            --r;
            
            //  BEGIN
            
            for (int j = l; j < r; ++j) {
                std::size_t k;
                for (k = 0; k < sizeof(escape) / sizeof(escape)[0]; ++k) {
                    std::size_t m = 0;
                    while (m < 2 && dst[j + m] == escape[k].first[m])
                        ++m;
                    
                    if (m == 2) {
                        dst[j] = escape[k].second;
                        
                        for (std::size_t p = j + 1; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                        
                        break;
                    }
                }
            }
            
            //  END
                        
            for (int j = l; j < r; ++j) {
                if (dst[j] == '\"') {
                    std::size_t k = j + 1;
                    while (k < r && dst[k] == '\"')
                        ++k;
                    
                    std::size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (std::size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                        --r;
                    }
                    
                    j += m;
                }
            }
                          
            std::size_t j = r;
            while (j < n - 1) {
                if (dst[j] == '\"') {
                    for (std::size_t k = j; k < n - 1; ++k)
                        std::swap(dst[k], dst[k + 1]);
                    --n;
                } else
                    ++j;
            }
        }
        
        return std::string(dst);
    }

    std::string empty() {
        return std::string();
    }

    std::string encode(const std::string str) {
        std::size_t i = 0;
        std::size_t n = str.length();
        
        char data[n++ * 2 + 3];
        
        strcpy(data, str.c_str());
        
        data[n] = '\"';
        
        for (i = n; i > 0; --i)
            std::swap(data[i], data[i - 1]);
        ++n;
        
        for (i = 1; i < n - 1; ++i) {
            if (data[i] == '\"') {
                data[n] = '\"';
                
                for (std::size_t j = n; j > i + 1; --j)
                    std::swap(data[j], data[j - 1]);
                
                ++i;
                ++n;
            }
        }
        
        data[n] = '\"';
        
        std::swap(data[n], data[n - 1]);
        
        ++n;
        
        return std::string(data);
    }

    std::string filename(const std::string filepath) {
        std::size_t ei = filepath.length();
        while (ei > 0 && filepath[ei - 1] != '.')
            --ei;
        
        if (!ei)
            ei = filepath.length();
        
        std::size_t si = ei;
        while (si > 0 && filepath[si - 1] != '/')
            --si;
        
        if (filepath[ei - 1] == '.')
            --ei;
        
        return filepath.substr(si, ei - si);
    }

    bool is_array(const std::string val) {
        std::string data[val.length() + 1];
        
        return parse(data, val) != 1;
    }

    bool is_dictionary(ss::array<std::string> arr) {
        if (arr.size() % 2 == 0) {
            std::size_t i = 0;
            while (i < (std::size_t)floor(arr.size() / 2) && is_string(arr[i * 2]) && arr[i * 2].length() != 2)
                ++i;
            
            if (i == (std::size_t)floor(arr.size() / 2)) {
                for (i = 0; i < (std::size_t)floor(arr.size() / 2); ++i) {
                    std::size_t j = i + 1;
                    while (j < (std::size_t)floor(arr.size() / 2) && arr[i * 2] != arr[j * 2])
                        ++j;
                    
                    if (j != (std::size_t)floor(arr.size() / 2))
                        break;
                }
                
                return i == (std::size_t)floor(arr.size() / 2);
            }
        }
        
        return false;
    }

    bool is_dictionary(const std::size_t len, const std::string* arr) {
        if (len % 2 == 0) {
            std::size_t i = 0;
            while (i < (std::size_t)floor(len / 2) && is_string(arr[i * 2]) && arr[i * 2].length() != 2)
                ++i;
            
            if (i == (std::size_t)floor(len / 2)) {
                for (i = 0; i < (std::size_t)floor(len / 2); ++i) {
                    std::size_t j = i + 1;
                    while (j < (std::size_t)floor(len / 2) && arr[i * 2] != arr[j * 2])
                        ++j;
                    
                    if (j != (std::size_t)floor(len / 2))
                        break;
                }
                
                return i == (std::size_t)floor(len / 2);
            }
        }
        
        return false;
    }

    bool is_int(const double val) {
        return val - (long)val == 0;
    }

    bool is_number(const std::string val) {
        if (val.empty())
            return false;
        
        size_t i = 0;
        if (val[i] == '+' || val[i] == '-')
            ++i;
        
        size_t j = i;
        while (j < val.length() && val[j] != '.')
            ++j;
        
        size_t k = j == val.length() ? i : j;
        while (k < val.length() && val[k] != 'E' && val[k] != 'e')
            ++k;
        
        size_t l = j < k ? j : k, m = i;
        
        while (m < l && isdigit(val[m]))
            ++m;
        
        if (m != l)
            return false;
        
        size_t n = l - i;
        
        if (j != val.length()) {
            m = j + 1;
            while (m < k && isdigit(val[m]))
                ++m;
            
            if (m != k)
                return false;
            
            n += k - j - 1;
        }
        
        if (n == 0)
            return false;
        
        if (k != val.length()) {
            size_t l = k + 1;
            
            if (l == val.length() || (val[l] != '+' && val[l] != '-'))
                return false;
            
            ++l;
            
            if (l == val.length())
                return false;
            
            while (l < val.length() && isdigit(val[l]))
                ++l;
            
            if (l != val.length())
                return false;
        }
        
        return true;
    }

    bool is_pow(const std::size_t a, const std::size_t b) {
        if (!b)
            return !a || a == 1;
        
        if (!a)
            return false;
        
        return is_int(log(a) / log(b));
    }

    bool is_string(const std::string val) {
        std::size_t i = 0;
        while (i < val.length() && val[i] != '\"')
            ++i;
        
        return i != val.length();
    }

    bool is_symbol(const std::string str) {
        if (str.empty())
            return false;
        
        std::size_t i = 0;
        while (i < floor(str.length() / 2) && str[i] == '`' && str[str.length() - i - 1] == '`')
            ++i;
        
        if (str[i] != '_' && !isalpha(str[i]))
            return false;
        
        std::size_t j = i + 1;
        while (j < str.length() - i && (str[j] == '_' || isalnum(str[j])))
            ++j;
        
        return j == str.length() - i;
    }

    bool is_table(ss::array<std::string> arr) {
        if (arr.size() > 1 && (!arr[0].empty() && !is_string(arr[0]))) {
            double d = stod(arr[0]);
            
            if (d - (int)d == 0 && d >= 1)
                return (arr.size() - 1) % (std::size_t)d == 0;
        }
        
        return false;
    }

    bool is_table(const std::size_t len, std::string* arr) {
        if (len > 1 && (!arr[0].empty() && !is_string(arr[0]))) {
            double d = stod(arr[0]);
            
            if (d - (int)d == 0 && d >= 1)
                return (len - 1) % (std::size_t)d == 0;
        }
        
        return false;
    }

    //  postcondition:  removes leading spaces
    std::string ltrim(const std::string str) {
        std::size_t l = str.length() + 1;
        char _str[l];
        
        strcpy(_str, str.c_str());
        
        while (l > 1 && isspace(_str[0])) {
            for (std::size_t i = 0; i < l - 1; ++i)
                std::swap(_str[i], _str[i + 1]);
            --l;
        }
        
        return std::string(_str);
    }

    std::size_t merge(std::size_t len, std::string* arr, const std::string pat) {
        for (int i = 0; i < (int)len - 1; ++i) {
            std::size_t l = 0;
            while (l < arr[i].length() && arr[i][l] != '\"')
                ++l;
            
            if (l != arr[i].length()) {
                bool f = true;
                
                std::size_t j = l + 1;
                while (j < arr[i].length()) {
                    if (arr[i][j] == '\"') {
                        std::size_t r = j + 1;
                        while (r < arr[i].length() && arr[i][r] == '\"')
                            ++r;
                        
                        if ((r - j) % 2 == 0)
                            j = r;
                        else {
                            f = false;
                            break;
                        }
                    } else
                        ++j;
                }
                
                if (f) {
                    bool g = true;
                    
                    while (g && i < len - 1) {
                        std::size_t j = 0;
                        while (j < arr[i + 1].length()) {
                            if (arr[i + 1][j] == '\"') {
                                std::size_t r = j + 1;
                                while (r < arr[i + 1].length() && arr[i + 1][r] == '\"')
                                    ++r;
                                
                                if ((r - j) % 2 == 0)
                                    j = r;
                                else {
                                    g = false;
                                    break;
                                }
                            } else
                                ++j;
                        }
                        
                        arr[i] += pat + arr[i + 1];
                        
                        for (std::size_t k = i + 1; k < len - 1; ++k)
                            std::swap(arr[k], arr[k + 1]);
                        
                        --len;
                    }
                }
            }
        }
        
        return len;
    }

    std::size_t parse(std::string* dst, const std::string src, std::string pat) {
        std::size_t n = split(dst, src, pat);
        
        n = merge(n, dst, pat);
        
        for (std::size_t i = 0; i < n; ++i)
            dst[i] = trim(dst[i]);
     
        return n;
    }

    std::string pattern() {
        return ",";
    }

    std::size_t pow2(const std::size_t num) {
        if (!num)
            return 1;
        
        return pow(2, ceil(log(num) / log(2)));
    }

    std::string raw(const std::string val, std::string pat) {
        if (val.empty())
            return val;
        
        if (is_number(val))
            return rtrim(stod(val));
        
        size_t n = val.length();
        char str[n + 1];
        
        strcpy(str, val.c_str());
        
        //  remove escape character
        size_t i;
        for (i = 0; i < n - 1; ++i) {
            if (val[i] != '\\' || val[i + 1] != ' ')
                continue;
            
            for (size_t j = i; j < n; ++j)
                std::swap(str[j], str[j + 1]);
            
            --n;
        }
        
        std::string _str(str);
        
        if (!pat.empty()) {
            for (i = 0; i <= _str.length() - pat.length(); ++i) {
                size_t j = 0;
                while (j < pat.length() && pat[j] == _str[i + j])
                    ++j;
                
                if (j == pat.length())
                    break;
            }
            
            if (i != _str.length() - pat.length() + 1)
                _str = decode(str);
        }
        
        return encode(_str);
    }

    int read(std::string* dst, const std::string src, const std::string pat) {
        assert(!src.empty());
        assert(!pat.empty());
        
        std::ifstream file;
        
        file.open(src);
        
        if (!file)
            return -1;
        
        int n = 0;
        
        std::string line;
        while (getline(file, line))
            dst[n++] = line;
        
        file.close();
        
        std::size_t p = n;
        
        n = (int)merge(n, dst, "\n");
        
        std::size_t i = 0, m = 0;
        while (i < n) {
            std::string valuev[dst[i].length() + 1];
            std::size_t valuec = parse(valuev, dst[i], pat);
            
            if (valuec > m)
                m = valuec;
            
            dst[n] = std::to_string(valuec);
            
            for (std::size_t j = n; j > i; --j)
                std::swap(dst[j], dst[j - 1]);
            
            ++n;
            ++i;
            
            dst[i] = raw(valuev[0], pat);
            
            for (std::size_t j = 1; j < valuec; ++j) {
                dst[n] = raw(valuev[j], pat);
                
                for (std::size_t k = n; k > i + j; --k)
                    std::swap(dst[k], dst[k - 1]);
                
                ++n;
            }
            
            i += valuec;
        }
        
        ++m;
        
        for (std::size_t i = 0; i < p; ++i) {
            std::size_t j = stoi(dst[i * m]);
            for (std::size_t k = 0; k < m - j - 1; ++k) {
                dst[n] = empty();
                
                for (std::size_t l = n; l > i * m + j + k + 1; --l)
                    std::swap(dst[l], dst[l - 1]);
                
                n++;
            }
        }
        
        dst[n] = std::to_string(m);
        
        for (std::size_t i = n; i > 0; --i)
            std::swap(dst[i], dst[i - 1]);
        
        ++n;
        
        return n;
    }

    std::string read(const std::string filename) {
        std::ifstream file;
        
        file.open(filename);
        
        if (!file)
            return "undefined";
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        return ss.str();
    }

    std::string* resize(const std::size_t len, const std::size_t newlen, std::string* arr) {
        std::size_t upplim = newlen > len ? len : newlen;
        
        std::string* tmp = new std::string[newlen];
        
        for (std::size_t i = 0; i < upplim; ++i)
            tmp[i] = arr[i];
        
        delete[] arr;
        
        return tmp;
    }

    std::string rtrim(const double num) {
        std::string str = std::to_string(num);
        
        std::size_t n = str.length();
        while (n > 0 && str[n - 1] == '0')
            --n;
        
        if (str[n - 1] == '.')
            --n;
        
        return str.substr(0, n);
    }

    //  postcondition:  removes trailing spaces
    std::string rtrim(const std::string str) {
        std::size_t l = str.length() + 1;
        char _str[l];
        strcpy(_str, str.c_str());
        
        while (l > 1 && isspace(_str[l - 2])) {
            std::swap(_str[l - 2], _str[l - 1]);
            --l;
        }
        
        return std::string(_str);
    }

    std::size_t split(std::string* dst, const std::string src, const std::string pat) {
        std::size_t s = 0, n = 0;
        for (int e = 0; e <= (int)src.length() - (int)pat.length(); ++e) {
            std::size_t i = 0;
            while (i < pat.length() && src[e + i] == pat[i])
                ++i;
            
            if (i == pat.length()) {
                dst[n++] = src.substr(s, e - s);
                s = e + i;
            }
        }
        
        dst[n++] = src.substr(s);

        return n;
    }

    std::string stringify(ss::array<std::string> arr, std::size_t beg) { return stringify(arr, beg, arr.size() - beg); }

    std::string stringify(const std::size_t len, std::string* arr) {
        if (!len)
            return empty();
        
        std::stringstream ss;
        
        if (len) {
            for (std::size_t i = 0; i < len - 1; ++i)
                ss << arr[i] << pattern();
            
            ss << arr[len - 1];
        }
        
        return ss.str();
    }

    std::string stringify(ss::array<std::string> arr, std::size_t beg, std::size_t len) {
        std::stringstream ss;
        
        if (len) {
            std::size_t i, n = beg + len;
            for (i = beg; i < n - 1; ++i)
                ss << arr[i] << pattern();
            
            ss << arr[i];
        }
        
        return ss.str();
    }

    //  split, but preserves the pattern
    std::size_t tokenize(std::string* dst, const std::string src, const std::string pat) {
        std::size_t s = 0, n = 0;    int e;
        for (e = 0; e <= (int)src.length() - (int)pat.length(); ++e) {
            std::size_t i = 0;
            while (i < pat.length() && src[e + i] == pat[i])
                ++i;
            
            if (i == pat.length()) {
                if (s != e)
                    dst[n++] = src.substr(s, e - s);
                
                dst[n++] = pat;
                s = e + i;
            }
        }
        
        if (s != src.length())
            dst[n++] = src.substr(s);

        return n;
    }

    std::size_t tokens(std::string* dst, const std::string src) {
        std::size_t n = 0, s = 0;
        for (std::size_t e = 0; e < src.length(); ++e) {
            while (e < src.length() && isspace(src[e]))
                ++e;
            
            if (s != e)
                dst[n++] = src.substr(s, e - s);
            
            s = e;
            while (e < src.length() && !isspace(src[e]))
                ++e;
            
            if (s != e)
                dst[n++] = src.substr(s, e - s);
            
            s = e;
        }
        
        std::size_t i = 0;
        while (i < n) {
            std::size_t tokenc = 0;
            std::string tokenv[dst[i].length() + 1];
            
            s = 0;  std::size_t e = 0;
            while (e < dst[i].length()) {
                std::size_t j;
                for (j = 0; j < sizeof (delim) / sizeof(delim[0]); ++j) {
                    if (e + delim[j].length() > dst[i].length())
                        continue;
                    
                    std::size_t k = 0;
                    while (k < delim[j].length() && dst[i][e + k] == delim[j][k])
                        ++k;
                    
                    if (k == delim[j].length())
                        break;
                }
                
                if (j == sizeof (delim) / sizeof(delim[0]))
                    ++e;
                else {
                    if (s != e)
                        tokenv[tokenc++] = dst[i].substr(s, e - s);
                    
                    tokenv[tokenc++] = delim[j];
                    s = ++e;
                }
            }
            
            if (s != dst[i].length())
                tokenv[tokenc++] = dst[i].substr(s);
            
            dst[i] = tokenv[0];
            
            for (std::size_t j = 1; j < tokenc; ++j) {
                dst[n] = tokenv[j];
                
                for (std::size_t k = n;  k > i + j; --k)
                    std::swap(dst[k], dst[k - 1]);
                
                ++n;
            }
            
            i += tokenc;
        }
        
        n = merge(n, dst, empty());
        
        int p = 0;
        for (i = 0; i < n; ++i) {
            if (dst[i] == "(")
                ++p;
            else if (dst[i] == ")") {
                if (!p)
                    throw error("Syntax error on token \")\", delete this token");
                    
                --p;
            }
        }
        
        if (p)
            throw error("Syntax error, insert \")\" to complete expr body");
        
        i = 0;
        while (i < n) {
            dst[i] = trim(dst[i]);
            
            if (dst[i].empty()) {
                for (std::size_t j = i; j < n - 1; ++j)
                    std::swap(dst[j], dst[j + 1]);
                
                --n;
            } else
                ++i;
        }
        
        return n;
    }

    std::string tolower(const std::string str) {
        char data[str.length() + 1];
        
        strcpy(data, str.c_str());
        
        for (std::size_t i = 0; i < str.length(); ++i)
            if (data[i] >= 65 && data[i] < 91)
                data[i] += 32;
        
        return std::string(data);
    }

    std::string toupper(const std::string str) {
        char data[str.length() + 1];
        
        strcpy(data, str.c_str());
        
        for (std::size_t i = 0; i < str.length(); ++i)
            if (data[i] >= 97 && data[i] < 123)
                data[i] -= 32;
        
        return std::string(data);
    }

    std::string trim(const std::string str) {
        std::size_t n = str.length() + 1;
        char data[n];
        
        strcpy(data, str.c_str());
        
        while (n > 1 && isspace(data[n - 2])) {
            std::swap(data[n - 2], data[n - 1]);
            --n;
        }
        
        while (n > 1 && isspace(data[0])) {
            for (std::size_t i = 0; i < n - 1; ++i)
                std::swap(data[i], data[i + 1]);
            --n;
        }
            
        return std::string(data);
    }

    std::string uuid() {
        std::ostringstream os;
        
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<> uid;
        
        for (std::size_t i = 0; i < 8; ++i)
            os << std::hex << uid(gen) % 16;
        
        os << '-';
        
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t j = 0; j < 4; ++j)
                os << uid(gen) % 16;
            
            os << '-';
        }
        
        for (std::size_t i = 0; i < 12; ++i)
            os << std::hex << uid(gen) % 16;
        
        return os.str();
    }

    void write(const std::string filename, const std::size_t len, std::string* arr, const std::string pat) {
        assert(!filename.empty());
        
        std::ofstream file;
        
        file.open(filename);
        
        for (std::size_t i = 0; i < (len - 1) / stoi(arr[0]); ++i) {
            for (std::size_t j = 0; j < stoi(arr[i * stoi(arr[0]) + 1]) - 1; ++j) {
                std::string value = arr[i * stoi(arr[0]) + j + 2];
                
                value = decode(value);
                
                std::size_t k;
                for (k = 0; k < value.length(); ++k) {
                    if (value[k] == '\"')
                        break;
                    
                    if (k > value.length() - pat.length())
                        continue;
                    
                    size_t l = 0;
                    while (l < pat.length() && pat[l] == value[k + l])
                        ++l;
                    
                    if (l == pat.length())
                        break;
                }
                
                if (k != value.length())
                    value = encode(value);
                
                file << value << pat;
            }
            
            std::string value = arr[i * stoi(arr[0]) + stoi(arr[i * stoi(arr[0]) + 1]) + 1];
            
            value = decode(value);
            
            std::size_t j;
            for (j = 0; j < value.length(); ++j) {
                if (value[j] == '\"')
                    break;
                
                if (j > value.length() - pat.length())
                    continue;
                
                std::size_t k = 0;
                while (k < pat.length() && pat[k] == value[j + k])
                    ++k;
                
                if (k == pat.length())
                    break;
            }
            
            if (j != value.length())
                value = encode(value);
                
            file << value << "\n";
        }
        
        file.close();
    }

    void write(const std::string filename, const std::string str) {
        std::ofstream file;
        
        file.open(filename);
        
        file << str;
        
        file.close();
    }
}
