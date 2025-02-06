//
//  file_system_exception.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/21/24.
//

#ifndef file_system_exception_h
#define file_system_exception_h

#include "exception.h"

namespace ss {
    struct file_system_exception: public ::exception {
        //  CONSTRUCTORS
        
        file_system_exception(const std::string message) : ::exception(message) { }
    };
}

#endif /* file_system_exception_h */
