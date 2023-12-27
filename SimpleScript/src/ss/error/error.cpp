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

    void defined_error(const std::string symbol) {
        throw error(symbol + " is defined");
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

    void type_error(const std::string lhs, const std::string rhs) {
        throw error("Cannot convert from " + lhs + " to " + rhs);
    }

    void write_error(const std::string symbol) {
        throw error(symbol + " is read-only");
    }

    void undefined_error(const std::string symbol) {
        throw error(symbol + " is undefined");
    }

    void unsupported_error(const std::string subject) {
        throw error(subject + " is unsupported");
    }
}
