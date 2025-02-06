//
//  error.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/13/23.
//

#include "error.h"

namespace ss {
    //  CONSTRUCTORS

    error::error(const std::string message) {
        this->message = message;
    }

    //  MEMBER FUNCTIONS

    const char* error::what() const throw() {
        return this->message.c_str();
    }

    //  NON-MEMBER FUNCTIONS

    void defined_error(const std::string key) {
        throw error(key + " is defined");
    }

    void expect_error(const std::string subject) {
        throw error("Expected " + subject);
    }

    void null_error() {
        throw error("null");
    }

    void operation_error() {
        throw error("invalid operation");
    }

    void range_error(const std::string message) {
        throw error("Out of range: " + message);
    }

    std::string to_string(const data_t type) {
        switch (type) {
            case array_t:
                return "array";
            case char_t:
                return "character";
            case dictionary_t:
                return "dictionary";
            case int_t:
                return "integer";
            case item_t:
                return "item";
            case number_t:
                return "number";
            case string_t:
                return "string";
            case table_t:
                return "table";
        }
        
        return null();
    }

    void type_error(const data_t lhs, const data_t rhs) {
        ss::type_error(to_string(lhs), to_string(rhs));
    }

    void type_error(const std::string lhs, const std::string rhs) {
        throw error("Cannot convert from " + lhs + " to " + rhs);
    }

    void write_error(const std::string key) {
        throw error(key + " is read-only");
    }

    void undefined_error(const std::string key) {
        throw error(key + " is not defined");
    }

    void unsupported_error(const std::string subject) {
        throw error(subject + " is unsupported");
    }
}
