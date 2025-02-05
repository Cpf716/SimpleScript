//
//  properties.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 2/4/25.
//

#include "properties.h"

namespace ss {
    // NON-MEMBER FUNCTIONS

    std::string environment() {
        return "/Library/Application Support/SimpleScript/example";
    }

    std::string extension() {
        return ".txt";
    }

    std::string library() {
        return "/Library/Application Support/SimpleScript/ssl";
    }

    std::string library_prefix() {
        return "@";
    }

    std::string logs() {
        return "/tmp/SimpleScript";
    }

    std::string main() {
        return "main.txt";
    }

    std::string path_separator() {
        return "/";
    }

    std::string separator() {
        return ",";
    }

    size_t tab_length() {
        return 4;
    }

    std::string temporary() {
        return "/tmp";
    }
}
