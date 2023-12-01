//
//  exception.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/30/23.
//

#ifndef exception_h
#define exception_h

#include <iostream>

namespace ss {
    class exception: public std::exception {
        std::string message;
    public:
        //  CONSTRUCTORS
        
        exception(const std::string message);
        
        //  MEMBER FUNCTIONS
        
        const char* what() const throw();
    };
}

#endif /* exception_h */
