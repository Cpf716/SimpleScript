//
//  error.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/13/23.
//

#ifndef error_h
#define error_h

#include <iostream>

namespace ss {
    class error: public std::exception {
        //  MEMBER FIELDS
        
        std::string message;
    public:
        //  CONSTRUCTORS
        
        error(const std::string message);
        
        //  MEMBER FUNCTIONS
        
        const char* what() const throw();
    };

    //  NON-MEMBER FUNCTIONS

    void defined_error(const std::string key);

    void expect_error(const std::string subject);

    void null_error();

    void operation_error();

    void range_error(const std::string message);

    void type_error(const std::string lhs, const std::string rhs);

    void undefined_error(const std::string key);

    void unsupported_error(const std::string subject);

    void write_error(const std::string key);
}


#endif /* error_h */
