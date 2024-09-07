//
//  preferences.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 4/21/24.
//

#include "preferences.h"

namespace ss {
    //  NON-MEMBER FIELDS

    std::string base_dir = "/Library/Application Support/SimpleScript/ssl/";

    std::string sep = ",";

    size_t tablen = 4;

    //  CONSTANTS

    const std::pair<std::string, std::string> ESCAPE_CHARACTERS[] {
        std::pair<std::string, std::string>("\\n", "\n"),
        std::pair<std::string, std::string>("\\r", "\r")
    };

    const std::string MONTHS[] {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

    const std::string SEPARATORS[] = { "!", "(", ")", ",", ".", ";", "^" };

    //  NON-MEMBER FUNCTIONS

    std::string datetime() {
        time_t now = time(0);
                
        char* dt = ctime(&now);
        
        //  tokens
        std::string src = std::string(dt);
        std::string dst[src.length() + 1];
        
        size_t beg = 0, len = 0;
        
        for (size_t end = 0; end < src.length(); ++end) {
            while (end < src.length() && isspace(src[end]))
                ++end;
            
            beg = end;
            while (end < src.length() && !isspace(src[end]))
                ++end;
            
            if (beg != end)
                dst[len++] = src.substr(beg, end - beg);
        }
        
        //  remove weekday
        for (size_t i = 0; i < len - 1; ++i)
            std::swap(dst[i], dst[i + 1]);
        
        --len;
        
        //  sort year
        for (size_t i = len - 1; i > 0; --i)
            std::swap(dst[i], dst[i - 1]);
        
        //  replace month
        size_t i = 0;
        while (i < sizeof(MONTHS) / sizeof(MONTHS[0]) && MONTHS[i] != dst[1])
            ++i;
        
        dst[1] = std::to_string(i + 1);
        
        for (size_t i = 1; i < 3; ++i)
            if (dst[i].length() == 1)
                dst[i] = "0" + dst[i];
        
        std::ostringstream ss;
        
        for (i = 0; i < 2; ++i)
            ss << dst[i] << "-";
        
        ss << dst[i] << " " << dst[3];
        
        return ss.str();
    }

    std::string decode(const std::string str) {
        int l = 0;
        while (l < str.length() && str[l] != '\"')
            ++l;
        
        if (l == str.length())
            return str;
            
        size_t n = str.length() + 1;
        char* dst = new char[pow_2(n)];
        
        strcpy(dst, str.c_str());
        
        for (size_t i = l; i < n - 1; ++i)
            std::swap(dst[i], dst[i + 1]);
        
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
            
            int j = l;
            while (j < n - 2) {
                if (dst[j] == '\\') {
                    if (dst[j + 1] == 't') {
                        if (j == l || dst[j - 1] != '\\') {
                            for (int k = (int)n; k < n + get_tablen() - (j + 1) % get_tablen() - 4; ++k) {
                                if (is_pow(k, 2)) {
                                    char* tmp = new char[pow_2(k * 2)];
                                    
                                    for (size_t m = 0; m < n; ++m)
                                        tmp[m] = dst[m];
                                    
                                    delete[] dst;
                                    
                                    dst = tmp;
                                }
                            }
                            
                            for (size_t k = 0; k < 2; ++k)
                                dst[j + k] = ' ';
                            
                            for (size_t k = 0; k < get_tablen() - (j + 1) % get_tablen(); ++k) {
                                dst[n] = ' ';
                                
                                for (size_t m = n; m > j + k + 2; --m)
                                    std::swap(dst[m], dst[m - 1]);
                                
                                ++n;
                                ++r;
                            }
                            
                            j += get_tablen();
                        } else
                            j += 3;
                    } else {
                        size_t k;
                        for (k = 0; k < sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS)[0]; ++k) {
                            if (j + ESCAPE_CHARACTERS[k].first.length() <= n - 1) {
                                size_t m = 1;
                                while (m < ESCAPE_CHARACTERS[k].first.length() && dst[j + m] == ESCAPE_CHARACTERS[k].first[m])
                                    ++m;
                                
                                if (m == ESCAPE_CHARACTERS[k].first.length()) {
                                    if (j == l || dst[j - 1] != '\\') {
                                        //  resize, if necessary
                                        int p;
                                        for (p = (int)n; p < n + ESCAPE_CHARACTERS[k].second.length() - ESCAPE_CHARACTERS[k].first.length(); ++p) {
                                            if (is_pow(p, 2)) {
                                                char* tmp = new char[pow_2(p * 2)];
                                                
                                                for (size_t q = 0; q < n; ++q)
                                                    tmp[q] = dst[q];
                                                
                                                delete[] dst;
                                                
                                                dst = tmp;
                                            }
                                        }
                                        
                                        //  erase pattern
                                        for (p = 0; p < ESCAPE_CHARACTERS[k].first.length(); ++p) {
                                            for (size_t q = j; q < n - 1; ++q)
                                                std::swap(dst[q], dst[q + 1]);
                                            
                                            --n;
                                            --r;
                                        }
                                        
                                        //  insert new pattern
                                        for (p = 0; p < ESCAPE_CHARACTERS[k].second.length(); ++p) {
                                            dst[n] = ESCAPE_CHARACTERS[k].second[p];
                                            
                                            for (size_t q = n; q > j + p; --q)
                                                std::swap(dst[q], dst[q - 1]);
                                            
                                            ++n;
                                            ++r;
                                        }
                                        
                                        j += ESCAPE_CHARACTERS[k].second.length();
                                    } else
                                        j += ESCAPE_CHARACTERS[k].first.length() + 1;
                                    
                                    break;
                                }
                            }
                        }
                        
                        if (k == sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS)[0])
                            ++j;
                    }
                } else
                    ++j;
            }
            
            for (int j = l; j < (int)n - 1; ++j) {
                if (dst[j] == '\"') {
                    size_t k = j + 1;
                    while (k < n - 1 && dst[k] == '\"')
                        ++k;
                    
                    size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                    }
                    
                    j += m;
                }
            }
        } else {
            for (size_t j = --r; j < n - 1; ++j)
                std::swap(dst[j], dst[j + 1]);
            
            --n;
            
            int j = l;
            while (j < r - 1) {
                if (dst[j] == '\\') {
                    if (dst[j + 1] == 't') {
                        if (j == l || dst[j - 1] != '\\') {
                            for (int k = (int)n; k < n + get_tablen() - (j + 1) % get_tablen() - 4; ++k) {
                                if (is_pow(k, 2)) {
                                    char* tmp = new char[pow_2(k * 2)];
                                    
                                    for (size_t m = 0; m < n; ++m)
                                        tmp[m] = dst[m];
                                    
                                    delete[] dst;
                                    
                                    dst = tmp;
                                }
                            }
                            
                            for (size_t k = 0; k < 2; ++k)
                                dst[j + k] = ' ';
                            
                            for (size_t k = 0; k < get_tablen() - (j + 1) % get_tablen(); ++k) {
                                dst[n] = ' ';
                                
                                for (size_t m = n; m > j + k + 2; --m)
                                    std::swap(dst[m], dst[m - 1]);
                                
                                ++n;
                                ++r;
                            }
                            
                            j += get_tablen();
                        } else
                            j += 3;
                    } else {
                        size_t k;
                        for (k = 0; k < sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS)[0]; ++k) {
                            if (j + ESCAPE_CHARACTERS[k].first.length() <= r) {
                                size_t m = 1;
                                while (m < ESCAPE_CHARACTERS[k].first.length() && dst[j + m] == ESCAPE_CHARACTERS[k].first[m])
                                    ++m;
                                
                                if (m == ESCAPE_CHARACTERS[k].first.length()) {
                                    if (j == l || dst[j - 1] != '\\') {
                                        //  resize, if necessary
                                        int p;
                                        for (p = (int)n; p < n + ESCAPE_CHARACTERS[k].second.length() - ESCAPE_CHARACTERS[k].first.length() - 2; ++p) {
                                            if (is_pow(p, 2)) {
                                                char* tmp = new char[pow_2(p * 2)];
                                                
                                                for (size_t q = 0; q < n; ++q)
                                                    tmp[q] = dst[q];
                                                
                                                delete[] dst;
                                                
                                                dst = tmp;
                                            }
                                        }
                                        
                                        //  erase pattern
                                        for (p = 0; p < ESCAPE_CHARACTERS[k].first.length(); ++p) {
                                            for (size_t q = j; q < n - 1; ++q)
                                                std::swap(dst[q], dst[q + 1]);
                                            
                                            --n;
                                            --r;
                                        }
                                        
                                        //  insert new pattern
                                        for (p = 0; p < ESCAPE_CHARACTERS[k].second.length(); ++p) {
                                            dst[n] = ESCAPE_CHARACTERS[k].second[p];
                                            
                                            for (size_t q = n; q > j + p; --q)
                                                std::swap(dst[q], dst[q - 1]);
                                            
                                            ++n;
                                            ++r;
                                        }
                                        
                                        j += ESCAPE_CHARACTERS[k].second.length();
                                    } else
                                        j += ESCAPE_CHARACTERS[k].first.length() + 1;
                                    
                                    break;
                                }
                            }
                        }
                        
                        if (k == sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS[0]))
                            ++j;
                    }
                } else
                    ++j;
            }
                        
            for (int j = l; j < r; ++j) {
                if (dst[j] == '\"') {
                    size_t k = j + 1;
                    while (k < r && dst[k] == '\"')
                        ++k;
                    
                    size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                        --r;
                    }
                    
                    j += m;
                }
            }
                          
            j = r;
            while (j < n - 1) {
                if (dst[j] == '\"') {
                    for (size_t k = j; k < n - 1; ++k)
                        std::swap(dst[k], dst[k + 1]);
                    --n;
                } else
                    ++j;
            }
        }
        
        std::string result = std::string(dst);
        
        delete[] dst;
        
        return result;
    }

    std::string decode_raw(const std::string str) {
        int l = 0;
        while (l < str.length() && str[l] != '\"')
            ++l;
        
        if (l == str.length())
            return str;
            
        size_t n = str.length() + 1;
        char* dst = new char[pow_2(n)];
        
        strcpy(dst, str.c_str());
        
        for (size_t i = l; i < n - 1; ++i)
            std::swap(dst[i], dst[i + 1]);
        
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
            
            for (int j = l; j < (int)n - 1; ++j) {
                if (dst[j] == '\"') {
                    size_t k = j + 1;
                    while (k < n - 1 && dst[k] == '\"')
                        ++k;
                    
                    size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                    }
                    
                    j += m;
                }
            }
        } else {
            for (size_t j = --r; j < n - 1; ++j)
                std::swap(dst[j], dst[j + 1]);
            
            --n;
                        
            for (int j = l; j < r; ++j) {
                if (dst[j] == '\"') {
                    size_t k = j + 1;
                    while (k < r && dst[k] == '\"')
                        ++k;
                    
                    size_t m;
                    for (m = 0; m < (k - j) / 2; ++m) {
                        for (size_t p = j; p < n - 1; ++p)
                            std::swap(dst[p], dst[p + 1]);
                        
                        --n;
                        --r;
                    }
                    
                    j += m;
                }
            }
                          
            size_t j = r;
            while (j < n - 1) {
                if (dst[j] == '\"') {
                    for (size_t k = j; k < n - 1; ++k)
                        std::swap(dst[k], dst[k + 1]);
                    --n;
                } else
                    ++j;
            }
        }
        
        std::string result = std::string(dst);
        
        delete[] dst;
        
        return result;
    }

    std::string empty() {
        return std::string();
    }

    std::string encode(const std::string str) {
        size_t n = str.length();
        
        char data[n++ * 2 + 3];
        
        strcpy(data, str.c_str());
        
        data[n] = '\"';
        
        for (size_t i = n; i > 0; --i)
            std::swap(data[i], data[i - 1]);
        
        ++n;
        
        for (size_t i = 1; i < n - 1; ++i) {
            if (data[i] == '\"') {
                data[n] = '\"';
                
                for (size_t j = n; j > i + 1; --j)
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

    std::string encode_raw(const std::string str) {
        size_t n = str.length();
        
        char data[n++ * 2 + 3];
        
        strcpy(data, str.c_str());
        
        data[n] = '\"';
        
        for (size_t i = n; i > 0; --i)
            std::swap(data[i], data[i - 1]);
        
        ++n;
        
        size_t i = 1;
        while (i < n - 2) {
            if (data[i] == '\\') {
                if (data[i + 1] == 't') {
                    data[n] = '\\';
                    
                    for (size_t j = n++; j > i; --j)
                        std::swap(data[j], data[j - 1]);
                        
                    i += 3;
                } else {
                    size_t j;
                    for (j = 0; j < sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS[0]); ++j) {
                        if (i + ESCAPE_CHARACTERS[j].first.length() <= n - 1) {
                            size_t k = 1;
                            while (k < ESCAPE_CHARACTERS[j].first.length() && data[i + k] == ESCAPE_CHARACTERS[j].first[k])
                                ++k;
                            
                            if (k == ESCAPE_CHARACTERS[j].first.length()) {
                                data[n] = '\\';
                                
                                for (size_t l = n++; l > i; --l)
                                    std::swap(data[l], data[l - 1]);
                                
                                i += ESCAPE_CHARACTERS[j].first.length() + 1;
                                
                                break;
                            }
                        }
                    }
                    
                    if (j == sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS[0]))
                        ++i;
                }
            } else
                ++i;
        }
        
        for (size_t i = 1; i < n - 1; ++i) {
            if (data[i] == '\"') {
                data[n] = '\"';
                
                for (size_t j = n; j > i + 1; --j)
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

    std::string escape(const std::string src) {
        int n = (int)src.length() + 1;
        char str[n];
        
        strcpy(str, src.c_str());
        
        int i = 0;
        while (i < n - 2) {
            size_t j = 0;
            while (j < 2 && str[i + j] == ("\\\\")[j])
                ++j;
            
            if (j == 2) {
                if (i + j <= n - 1) {
                    if (src[i + j] == 't') {
                        for (size_t k = i; k < n - 1; ++k)
                            std::swap(str[k], str[k + 1]);
                        
                        --n;
                        
                        i += 2;
                    } else {
                        size_t k;
                        for (k = 0; k < sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS)[0]; ++k) {
                            if (i + ESCAPE_CHARACTERS[k].first.length() + 1 <= n - 1) {
                                size_t l = 1;
                                while (l < ESCAPE_CHARACTERS[k].first.length() && str[i + l + 1] == ESCAPE_CHARACTERS[k].first[l])
                                    ++l;
                                
                                if (l == ESCAPE_CHARACTERS[k].first.length()) {
                                    for (size_t m = i; m < n - 1; ++m)
                                        std::swap(str[m], str[m + 1]);
                                    
                                    --n;
                                    
                                    i += ESCAPE_CHARACTERS[i].second.length();
                                    
                                    break;
                                }
                            }
                        }
                        
                        if (k == sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS)[0])
                            ++i;
                    }
                } else
                    ++i;
            } else
                ++i;
        }
        
        return std::string(str);
    }

    bool exists(const std::string src) {
    #if DEBUG_LEVEL
        assert(src.length());
    #endif
        std::ifstream file;
        
        file.open(src);
        
        bool exists = !!file;
        
        file.close();
        
        return exists;
    }

    std::string get_base_dir() {
        return base_dir;
    }

    std::string get_main() {
        return "__main.txt";
    }

    size_t get_pref(std::string* dst, const size_t num, std::string* arr, const std::string sep) {
#if DEBUG_LEVEL
        assert(dst != NULL);
        assert(num >= 1);
        assert(arr != NULL);
        assert(sep.length());
#endif
        for (size_t i = 0; i < (num - 1) / stoi(arr[0]); ++i) {
            if (stoi(arr[i * stoi(arr[0]) + 1]) >= 2) {
                if (arr[i * stoi(arr[0]) + 2] == sep) {
                    size_t _num = stoi(arr[i * stoi(arr[0]) + 1]) - 1;
                    
                    for (size_t j = 0; j < _num; ++j)
                        dst[j] = arr[i * stoi(arr[0]) + j + 3];
                    
                    return _num;
                }
            }
        }
        
        return 0;
    }

    std::string get_sep() {
        return sep;
    }

    size_t get_tablen() {
        return tablen;
    }

    bool is_int(const double val) {
        return val - (long)val == 0;
    }

    bool is_number(const std::string val) {
        if (val.empty())
            return false;
        
        int i = 0;
        if (val[i] == '+' || val[i] == '-')
            ++i;
        
        int j = i;
        while (j < val.length() && val[j] != '.')
            ++j;
        
        int k = j == val.length() ? i : j;
        while (k < val.length() && val[k] != 'E' && val[k] != 'e')
            ++k;
        
        int l = j < k ? j : k, m = i;
        
        if (m < l) {
            if (!isdigit(val[m]))
                return false;
            
            ++m;
            
            //  before decimal
            while (m < l - 1 && (val[m] == '_' || isdigit(val[m])))
                ++m;
            
            if (m < l) {
                if (!isdigit(val[m]))
                    return false;
                
                ++m;
            }
        }
        
        if (m != l)
            return false;
        
        size_t n = l - i;
        
        //  after decimal and before exponent
        if (j != val.length()) {
            m = j + 1;
            
            if (m < k) {
                if (!isdigit(val[m]))
                    return false;
                
                ++m;
                
                while (m < k - 1 && (val[m] == '_' || isdigit(val[m])))
                    ++m;
                
                if (m < k) {
                    if (!isdigit(val[m]))
                        return false;
                    
                    ++m;
                }
            }
            
            if (m != k)
                return false;
            
            n += k - j - 1;
        }
        
        if (n == 0)
            return false;
        
        //  after exponent
        if (k != val.length()) {
            size_t l = k + 1;
            
            if (l == val.length() || (val[l] != '+' && val[l] != '-'))
                return false;
            
            ++l;
            
            if (l == val.length())
                return false;
            
            if (l < val.length()) {
                if (!isdigit(val[l]))
                    return false;
                
                ++l;
                
                while (l < val.length() -  1 && (val[l] == '_' || isdigit(val[l])))
                    ++l;
                
                if (l < val.length()) {
                    if (!isdigit(val[l]))
                        return false;
                    
                    ++l;
                }
            }
            
            if (l != val.length())
                return false;
        }
        
        return true;
    }

    bool is_pow(const size_t a, const size_t b) {
        if (!b)
            return !a || a == 1;
        
        if (!a)
            return false;
        
        return is_int(log(a) / log(b));
    }

    void init_preferences() {
        if (!exists("/tmp/SimpleScript"))
            if (mkdir(std::string("/tmp/SimpleScript").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ))
                throw file_system_exception(strerror(errno));
        
        std::string now = datetime();
        
        std::string tokenv[now.length() + 1];
        
        tokens(tokenv, now);
        
        std::ostringstream ss;
        
        ss << "/tmp/SimpleScript/" << tokenv[0] << "-";

        std::string time[3];
        
        split(time, tokenv[1], ":");
        
        for (size_t i = 0; i < 3; ++i)
            ss << time[i];
        
        ss << ".log";
        
        logger_open(ss.str());
        
        std::string arr[1024];
        size_t num = read(arr, get_base_dir() + "prefs.csv", get_sep());
        
        if (num == 1)
            return;
        
        std::string valuev[1024];
        size_t valuec;
        
        valuec = get_pref(valuev, num, arr, encode("sep"));
        
        if (valuec == 1) {
            sep = decode_raw(valuev[0]);
            
            size_t i = 0;
            while (i < sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS[0]) && sep != ESCAPE_CHARACTERS[i].first)
                ++i;
            
            if (i == sizeof(ESCAPE_CHARACTERS) / sizeof(ESCAPE_CHARACTERS[0])) {
                if (sep == "\\t")
                    sep = "\t";
            } else
                sep = ESCAPE_CHARACTERS[i].second;
            
#if DEBUG_LEVEL
            logger_write("sep:\t" + escape(sep) + "\r\n");
#endif
        }
        
        valuec = get_pref(valuev, num, arr, encode("tablen"));
        
        if (valuec == 1 && is_number(valuev[0])) {
            double _num = stod(valuev[0]);
            
            if (is_int(_num) && _num >= 0) {
                tablen = (size_t)_num;
                
#if DEBUG_LEVEL
                logger_write("tablen:\t" + trim_end(_num) + "\r\n");
#endif
            }
        }
    }

    size_t merge(size_t len, std::string* arr, const std::string sep) {
#if DEBUG_LEVEL
        assert(arr != NULL);
#endif
        for (int i = 0; i < (int)len - 1; ++i) {
            size_t l = 0;
            while (l < arr[i].length() && arr[i][l] != '\"')
                ++l;
            
            if (l != arr[i].length()) {
                bool f = true;
                
                size_t j = l + 1;
                while (j < arr[i].length()) {
                    if (arr[i][j] == '\"') {
                        size_t r = j + 1;
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
                        size_t j = 0;
                        while (j < arr[i + 1].length()) {
                            if (arr[i + 1][j] == '\"') {
                                size_t r = j + 1;
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
                        
                        arr[i] += sep + arr[i + 1];
                        
                        for (size_t k = i + 1; k < len - 1; ++k)
                            std::swap(arr[k], arr[k + 1]);
                        
                        --len;
                    }
                }
            }
        }
        
        return len;
    }

    size_t parse(std::string* dst, const std::string src, std::string sep) {
#if DEBUG_LEVEL
        assert(dst != NULL);
        assert(sep.length());
#endif
        size_t n = split(dst, src, sep);
        
        n = merge(n, dst, sep);
        
        for (size_t i = 0; i < n; ++i)
            dst[i] = trim(dst[i]);
     
        return n;
    }

    double parse_number(const std::string str) {
        int num = (int)str.length() + 1;
        char arr[num];
        
        strcpy(arr, str.c_str());
        
        int i = 1;
        while (i < num - 2) {
            if (arr[i] == '_') {
                for (size_t j = i; j < num - 1; ++j)
                    std::swap(arr[j], arr[j + 1]);
                
                --num;
            } else
                ++i;
        }
        
        return stod(std::string(arr));
    }

    size_t pow_2(const size_t num) {
        if (!num)
            return 1;
        
        return pow(2, ceil(log(num) / log(2)));
    }

    std::string raw(const std::string val) {
        if (val.empty())
            return val;
        
        if (is_number(val))
            return trim_end(parse_number(val));
        
        return encode_raw(val);
    }

    std::string raw(const std::string val, std::string sep) {
#if DEBUG_LEVEL
        assert(sep.length());
#endif
        if (val.empty())
            return val;
        
        if (is_number(val))
            return trim_end(parse_number(val));
        
        std::string str = val;
        
        size_t i;
        for (i = 0; i <= str.length() - sep.length(); ++i) {
            size_t j = 0;
            while (j < sep.length() && sep[j] == str[i + j])
                ++j;
            
            if (j == sep.length())
                break;
        }
        
        if (i != str.length() - sep.length() + 1)
            str = decode_raw(str);
        
        return encode_raw(str);
    }

    int read(std::string* dst, const std::string src, const std::string sep) {
#if DEBUG_LEVEL
        assert(dst != NULL);
        assert(!src.empty());
        assert(!sep.empty());
#endif
        std::ifstream file;
        
        file.open(src);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        int n = 0;
        
        std::string line;
        while (getline(file, line))
            dst[n++] = line;
        
        file.close();
        
        size_t p = n;
        
        n = (int)merge(n, dst, "\n");
        
        size_t i = 0, m = 0;
        while (i < n) {
            std::string valuev[dst[i].length() + 1];
            size_t valuec = parse(valuev, dst[i], sep);
            
            if (valuec > m)
                m = valuec;
            
            dst[n] = std::to_string(valuec);
            
            for (size_t j = n; j > i; --j)
                std::swap(dst[j], dst[j - 1]);
            
            ++n;
            ++i;
            
            dst[i] = raw(valuev[0], sep);
            
            for (size_t j = 1; j < valuec; ++j) {
                dst[n] = raw(valuev[j], sep);
                
                for (size_t k = n; k > i + j; --k)
                    std::swap(dst[k], dst[k - 1]);
                
                ++n;
            }
            
            i += valuec;
        }
        
        ++m;
        
        for (size_t i = 0; i < p; ++i) {
            int j = stoi(dst[i * m]);
            for (size_t k = 0; k < m - j - 1; ++k) {
                dst[n] = empty();
                
                for (size_t l = n; l > i * m + j + k + 1; --l)
                    std::swap(dst[l], dst[l - 1]);
                
                n++;
            }
        }
        
        dst[n] = std::to_string(m);
        
        for (size_t i = n; i > 0; --i)
            std::swap(dst[i], dst[i - 1]);
        
        ++n;
        
        return n;
    }

    size_t split(std::string* dst, const std::string src, const std::string sep) {
#if DEBUG_LEVEL
        assert(dst != NULL);
        assert(sep.length());
#endif
        size_t s = 0, n = 0;
        for (int e = 0; e <= (int)src.length() - (int)sep.length(); ++e) {
            size_t i = 0;
            while (i < sep.length() && src[e + i] == sep[i])
                ++i;
            
            if (i == sep.length()) {
                dst[n++] = src.substr(s, e - s);
                s = e + i;
            }
        }
        
        dst[n++] = src.substr(s);

        return n;
    }

    size_t tokens(std::string* dst, const std::string src) {
    #if DEBUG_LEVEL
        assert(dst != NULL);
    #endif
        size_t n = 0, s = 0;
        for (size_t e = 0; e < src.length(); ++e) {
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
        
        size_t i = 0;
        while (i < n) {
            size_t tokenc = 0;
            std::string tokenv[dst[i].length() + 1];
            
            s = 0;  size_t e = 0;
            while (e < dst[i].length()) {
                size_t j;
                for (j = 0; j < sizeof (SEPARATORS) / sizeof(SEPARATORS[0]); ++j) {
                    if (e + SEPARATORS[j].length() > dst[i].length())
                        continue;
                    
                    size_t k = 0;
                    while (k < SEPARATORS[j].length() && dst[i][e + k] == SEPARATORS[j][k])
                        ++k;
                    
                    if (k == SEPARATORS[j].length())
                        break;
                }
                
                if (j == sizeof (SEPARATORS) / sizeof(SEPARATORS[0]))
                    ++e;
                else {
                    if (s != e)
                        tokenv[tokenc++] = dst[i].substr(s, e - s);
                    
                    tokenv[tokenc++] = SEPARATORS[j];
                    s = ++e;
                }
            }
            
            if (s != dst[i].length())
                tokenv[tokenc++] = dst[i].substr(s);
            
            dst[i] = tokenv[0];
            
            for (size_t j = 1; j < tokenc; ++j) {
                dst[n] = tokenv[j];
                
                for (size_t k = n;  k > i + j; --k)
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
                for (size_t j = i; j < n - 1; ++j)
                    std::swap(dst[j], dst[j + 1]);
                
                --n;
            } else
                ++i;
        }
        
        return n;
    }

    std::string trim(const std::string str) {
        size_t s = 0;
        while (s < str.length() && isspace(str[s]))
            ++s;
        
        size_t e = str.length();
        while (e > s && isspace(str[e - 1]))
            --e;
            
        return str.substr(s, e - s);
    }

    std::string trim_end(const double num) {
        std::string str = std::to_string(num);
        
        size_t n = str.length();
        while (n > 0 && str[n - 1] == '0')
            --n;
        
        if (str[n - 1] == '.')
            --n;
        
        return str.substr(0, n);
    }
}
