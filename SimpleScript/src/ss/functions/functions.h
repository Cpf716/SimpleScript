//
//  functions.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/1/22.
//

#ifndef functions_h
#define functions_h

#include "array.h"
#include "file_system.h"

namespace ss {
    //  NON-MEMBER FIELDS

    enum data_t { array_t, char_t, dictionary_t, int_t, item_t, number_t, string_t, table_t };

    //  NON-MEMBER FUNCTIONS

    std::string escape(const std::string src);

    bool is_array(const std::string val);

    bool is_dictionary(ss::array<std::string> arr);

    bool is_dictionary(const size_t len, const std::string* arr);

    bool is_key(const std::string val);

    bool is_string(const std::string val);

    bool is_table(const size_t len, std::string* arr);

    bool is_table(ss::array<std::string> arr);

    std::string* resize(const size_t len, const size_t newlen, std::string* arr);

    std::string stringify(ss::array<std::string> arr, size_t beg = 0);

    std::string stringify(ss::array<std::string> arr, size_t beg, size_t end);

    std::string stringify(const size_t len, std::string* arr);    

    size_t tokenize(std::string* dst, const std::string src, const std::string pat);

    std::string tolower(std::string str);

    std::string to_string(const data_t type);

    std::string toupper(std::string str);

    std::string trim_end(const std::string str);

    std::string trim_start(const std::string str);

    void type_error(const data_t lhs, const data_t rhs);

    size_t write(const std::string filename, const size_t len, std::string* arr, const std::string sep);
}

#endif /* commoon_h */
