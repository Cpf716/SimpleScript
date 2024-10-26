//
//  preferences.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/21/24.
//

#define DEBUG_LEVEL 0

#ifndef preferences_h
#define preferences_h

#include "error.h"
#include "logger.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sys/stat.h>       //  mkdir

namespace ss {
    std::string datetime();
    
    std::string decode(const std::string str);

    std::string decode_raw(const std::string str);

    std::string encode(const std::string str);

    std::string encode_raw(const std::string str);

    std::string escape(const std::string src);

    bool exists(const std::string src);

    std::string get_base_dir();

    std::string get_main();

    std::string get_sep();

    size_t get_tablen();

    bool is_int(const double num);

    bool is_number(const std::string val);

    bool is_pow(const size_t a, const size_t b);

    void load_preferences();

    size_t merge(size_t len, std::string* arr, const std::string sep);

    std::string null();

    size_t parse(std::string* dst, const std::string src, std::string sep = get_sep());

    double parse_number(const std::string str);

    size_t pow2(const size_t num);

    std::string raw(const std::string val);

    std::string raw(const std::string val, std::string sep);

    int read(std::string* dst, const std::string src, const std::string sep);

    size_t split(std::string* dst, const std::string src, const std::string sep = get_sep());

    size_t tokens(std::string* dst, const std::string src, const size_t sepc = 0, const std::string* sepv = NULL);

    std::string trim(const std::string str);

    std::string encode(const double num);
}

#endif /* preferences_h */
