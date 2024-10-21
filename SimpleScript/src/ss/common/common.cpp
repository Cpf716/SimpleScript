//
//  common.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 11/4/22.
//

#include "common.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    bool is_array(const std::string val) {
        std::string data[val.length() + 1];
        
        return parse(data, val) != 1;
    }

    bool is_dictionary(ss::array<std::string> arr) {
#if DEBUG_LEVEL
        assert(arr.size());
#endif
        if (arr.size() % 2 == 0) {
            size_t i = 0;
            while (i < (size_t)floor(arr.size() / 2) && is_string(arr[i * 2]) && arr[i * 2].length() != 2)
                ++i;
            
            if (i == (size_t)floor(arr.size() / 2)) {
                for (i = 0; i < (size_t)floor(arr.size() / 2); ++i) {
                    size_t j = i + 1;
                    while (j < (size_t)floor(arr.size() / 2) && arr[i * 2] != arr[j * 2])
                        ++j;
                    
                    if (j != (size_t)floor(arr.size() / 2))
                        break;
                }
                
                return i == (size_t)floor(arr.size() / 2);
            }
        }
        
        return false;
    }

    bool is_dictionary(const size_t len, const std::string* arr) {
#if DEBUG_LEVEL
        assert(len);
        assert(arr != NULL);
#endif
        if (len % 2 == 0) {
            size_t i = 0;
            while (i < (size_t)floor(len / 2) && is_string(arr[i * 2]) && arr[i * 2].length() != 2)
                ++i;
            
            if (i == (size_t)floor(len / 2)) {
                for (i = 0; i < (size_t)floor(len / 2); ++i) {
                    size_t j = i + 1;
                    while (j < (size_t)floor(len / 2) && arr[i * 2] != arr[j * 2])
                        ++j;
                    
                    if (j != (size_t)floor(len / 2))
                        break;
                }
                
                return i == (size_t)floor(len / 2);
            }
        }
        
        return false;
    }

    bool is_string(const std::string val) {
        size_t i = 0;
        while (i < val.length() && val[i] != '\"')
            ++i;
        
        return i != val.length();
    }

    bool is_symbol(const std::string str) {
        if (str.empty())
            return false;
        
        size_t i = 0;
        while (i < floor(str.length() / 2) && str[i] == '`' && str[str.length() - i - 1] == '`')
            ++i;
        
        if (str[i] != '_' && !isalpha(str[i]))
            return false;
        
        size_t j = i + 1;
        while (j < str.length() - i && (str[j] == '_' || isalnum(str[j])))
            ++j;
        
        return j == str.length() - i;
    }

    bool is_table(ss::array<std::string> arr) {
#if DEBUG_LEVEL
        assert(arr.size());
#endif
        if (!arr[0].empty() && !is_string(arr[0])) {
            double num = stod(arr[0]);
            
            if (is_int(num) && num >= 1)
                return (arr.size() - 1) % (size_t)num == 0;
        }
        
        return false;
    }

    bool is_table(const size_t len, std::string* arr) {
#if DEBUG_LEVEL
        assert(len);
        assert(arr != NULL);
#endif
        if (!arr[0].empty() && !is_string(arr[0])) {
            double num = stod(arr[0]);
            
            if (is_int(num) && num >= 1)
                return (len - 1) % (size_t)num == 0;
        }
        
        return false;
    }

    std::string* resize(const size_t len, const size_t newlen, std::string* arr) {
#if DEBUG_LEVEL
        assert(arr != NULL);
#endif
        size_t end = newlen > len ? len : newlen;
        
        std::string* tmp = new std::string[newlen];
        
        for (size_t i = 0; i < end; ++i)
            tmp[i] = arr[i];
        
        delete[] arr;
        
        return tmp;
    }

    std::string stringify(ss::array<std::string> arr, size_t beg) {
#if DEBUG_LEVEL
        assert(beg <= arr.size());
#endif
        return stringify(arr, beg, arr.size() - beg);
    }

    std::string stringify(const size_t len, std::string* arr) {
#if DEBUG_LEVEL
        assert(arr != NULL);
#endif
        std::stringstream ss;
        
        if (len) {
            for (size_t i = 0; i < len - 1; ++i)
                ss << arr[i] << get_sep();
            
            ss << arr[len - 1];
        }
        
        return ss.str();
    }

    std::string stringify(ss::array<std::string> arr, size_t beg, size_t len) {
        std::stringstream ss;
        
        if (len) {
            size_t i, n = beg + len;
            for (i = beg; i < n - 1; ++i)
                ss << arr[i] << get_sep();
            
            ss << arr[i];
        }
        
        return ss.str();
    }

    //  splits but preserves pat
    size_t tokenize(std::string* dst, const std::string src, const std::string pat) {
#if DEBUG_LEVEL
        assert(dst != NULL);
        assert(pat.length());
#endif
        size_t s = 0, n = 0;    int e;
        for (e = 0; e <= (int)src.length() - (int)pat.length(); ++e) {
            size_t i = 0;
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

    std::string tolower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        
        return str;
    }

    std::string to_string(const data_t data_type) {
        switch (data_type) {
            case array_t:
                return "array";
            case char_t:
                return "char";
            case dictionary_t:
                return "dictionary";
            case double_t:
                return "double";
            case int_t:
                return "int";
            case string_t:
                return "string";
            case table_t:
                return "table";
            case undefined_t:
                return "undefined";
        }
        
        return null();
    }

    std::string toupper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        
        return str;
    }

    //  postcondition:  removes trailing spaces
    std::string trim_end(const std::string str) {
        size_t e = str.length();
        while (e > 0 && isspace(str[e - 1]))
            --e;
            
        return str.substr(0, e);
    }

    //  postcondition:  removes leading spaces
    std::string trim_start(const std::string str) {
        size_t s = 0;
        while (s < str.length() && isspace(str[s]))
            ++s;
            
        return str.substr(s);
    }

    void type_error(const data_t lhs, const data_t rhs) {
        ss::type_error(to_string(lhs), to_string(rhs));
    }

    size_t write(const std::string filename, const size_t len, std::string* arr, const std::string sep) {
#if DEBUG_LEVEL
        assert(!filename.empty());
        assert(len);
        assert(arr != NULL);
#endif
        std::ostringstream ss;
        
        for (size_t i = 0; i < (len - 1) / stoi(arr[0]); ++i) {
            for (size_t j = 0; j < stoi(arr[i * stoi(arr[0]) + 1]) - 1; ++j) {
                std::string value = arr[i * stoi(arr[0]) + j + 2];
                
                value = escape(decode(value));
                
                size_t k;
                for (k = 0; k < value.length(); ++k) {
                    if (value[k] == '\"')
                        break;
                    
                    if (k > value.length() - sep.length())
                        continue;
                    
                    size_t l = 0;
                    while (l < sep.length() && sep[l] == value[k + l])
                        ++l;
                    
                    if (l == sep.length())
                        break;
                }
                
                if (k != value.length())
                    value = encode(value);
                
                ss << value << sep;
            }
            
            std::string value = arr[i * stoi(arr[0]) + stoi(arr[i * stoi(arr[0]) + 1]) + 1];
            
            value = escape(decode(value));
            
            size_t j;
            for (j = 0; j < value.length(); ++j) {
                if (value[j] == '\"')
                    break;
                
                if (j > value.length() - sep.length())
                    continue;
                
                size_t k = 0;
                while (k < sep.length() && sep[k] == value[j + k])
                    ++k;
                
                if (k == sep.length())
                    break;
            }
            
            if (j != value.length())
                value = encode(value);
                
            ss << value << "\n";
        }
        
        std::ofstream file;
        
        file.open(filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        file << ss.str();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
        
        file.close();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
        
        return ss.str().length();
    }
}
