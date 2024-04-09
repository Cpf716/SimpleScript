//
//  common.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/1/22.
//

#ifndef common_h
#define common_h

#include "array.h"
#include <cassert>
#include "error.h"
#include "logger.h"
#include <random>

namespace ss {

    //  NON-MEMBER FUNCTIONS

    std::string decode(const std::string str);

    std::string empty();

    std::string encode(const std::string str);

    std::string filename(const std::string filepath);

    bool is_array(const std::string val);

    bool is_dictionary(ss::array<std::string> arr);

    bool is_dictionary(const std::size_t len, const std::string* arr);

    bool is_int(const double num);

    bool is_number(const std::string val);

    bool is_pow(const std::size_t a, const std::size_t b);

    bool is_string(const std::string val);

    bool is_symbol(const std::string val);

    bool is_table(const std::size_t len, std::string* arr);

    bool is_table(ss::array<std::string> arr);

    std::string ltrim(const std::string str);

    std::size_t merge(std::size_t len, std::string* arr, const std::string pat);

    std::string pattern();

    std::size_t parse(std::string* dst, const std::string src, std::string pat = pattern());

    std::size_t pow2(const std::size_t num);

    std::string raw(const std::string val, std::string pat = empty());

    std::string read(const std::string filename);

    int read(std::string* dst, const std::string src, const std::string pat);

    std::string* resize(const std::size_t len, const std::size_t newlen, std::string* arr);

    std::string rtrim(const double num);

    std::string rtrim(const std::string str);

    std::size_t split(std::string* dst, const std::string src, const std::string pat);

    std::string stringify(ss::array<std::string> arr, std::size_t beg = 0);

    std::string stringify(ss::array<std::string> arr, std::size_t beg, std::size_t end);

    std::string stringify(const std::size_t len, std::string* arr);

    std::size_t tokens(std::string* dst, const std::string src);

    std::size_t tokenize(std::string* dst, const std::string src, const std::string pat);

    std::string tolower(const std::string str);

    std::string toupper(const std::string str);

    std::string trim(const std::string str);

    std::string uuid();

    void write(const std::string filename, const std::string str);

    void write(const std::string filename, const std::size_t len, std::string* arr, const std::string pat);
}

#endif /* commoon_h */
