//
//  mysql_exception.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/17/24.
//

#ifndef mysql_exception_h
#define mysql_exception_h

#include "exception.h"

namespace integration {
    class mysql_exception: public ::exception {
    public:
        //  CONSTRUCTORS
        
        mysql_exception(const std::string message) : ::exception(message) { }
    };
}

#endif /* mysql_exception_h */
