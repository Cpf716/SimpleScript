//
//  exception.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/30/23.
//

#include "exception.h"

namespace ss {
    //  CONSTRUCTORS

    exception::exception(const std::string message) {
        this->message = message;
    }

    //  MEMBER FUNCTIONS

    const char* exception::what() const throw() {
        return message.c_str();
    }
}
