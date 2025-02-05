//
//  functions.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/1/22.
//

#ifndef functions_h
#define functions_h

#include "array.h"
#include "constants.h"
#include "file_system.h"

#include "error.h"
#include "logger.h"
#include "properties.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <sys/stat.h>       //  mkdir

namespace ss {
    //  NON-MEMBER FIELDS

    enum data_t { array_t, char_t, dictionary_t, int_t, item_t, number_t, string_t, table_t };

    //  NON-MEMBER FUNCTIONS

    size_t       _tokens(std::string* target, const std::string source);

    std::string  encode(const double num);

    std::string  encode(const std::string str);

    std::string  encode_raw(const std::string str);

    std::string  escape(const std::string src);

    std::string  date();

    std::string  decode(const std::string str);

    std::string  decode_raw(const std::string str);

    bool         is_array(const std::string val);

    bool         is_dictionary(ss::array<std::string> arr);

    bool         is_dictionary(const size_t len, const std::string* arr);

    bool         is_int(const double num);

    bool         is_key(const std::string val);

    bool         is_number(const std::string value);

    bool         is_pow(const size_t a, const size_t b);

    bool         is_string(const std::string val);

    bool         is_table(const size_t len, std::string* arr);

    bool         is_table(ss::array<std::string> arr);

    size_t       merge(size_t len, std::string* arr, const std::string sep);

    std::string  null();

    std::string  pad_start(std::string string, size_t pad_length, std::string pad_string);

    std::string  pad_end(std::string string, size_t pad_length, std::string pad_string);

    size_t       parse(std::string* dst, const std::string src, std::string sep = separator());
    
    double       parse_number(const std::string string);

    size_t       pow2(const size_t num);

    std::string  raw(const std::string val);

    std::string  raw(const std::string val, std::string sep);

    std::string* resize(const size_t len, const size_t newlen, std::string* arr);

    size_t       split(std::string* dst, const std::string src, const std::string sep = separator());

    std::string  stringify(ss::array<std::string> arr, size_t beg = 0);

    std::string  stringify(ss::array<std::string> arr, size_t beg, size_t end);

    std::string  stringify(const size_t len, std::string* arr);

    size_t       tokenize(std::string* dst, const std::string src, const std::string pat);

    std::string  tolower(std::string str);

    std::string  to_string(const data_t type);

    size_t       tokens(std::string* dst, const std::string src, const size_t sepc = 0, const std::string* sepv = NULL);

    std::string  toupper(std::string str);

    std::string  trim(const std::string str);

    std::string  trim_end(const std::string str);

    std::string  trim_start(const std::string str);

    void         type_error(const data_t lhs, const data_t rhs);
}

#endif /* commoon_h */
