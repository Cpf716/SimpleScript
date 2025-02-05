//
//  socket_exception.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/29/23.
//

#ifndef socket_exception_h
#define socket_exception_h

#include "exception.h"

namespace ss {
    namespace integration {
        struct socket_exception: public ::exception {
            //  CONSTRUCTORS
            
            socket_exception(const std::string message) : ::exception(message) { }
        };
    }
}

#endif /* socket_exception_h */
