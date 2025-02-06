//
//  error.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/13/23.
//

#ifndef error_h
#define error_h

#include "constants.h"

namespace ss {
    // TYPEDEF

    enum data_t { array_t, char_t, dictionary_t, int_t, item_t, number_t, string_t, table_t };

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

    void        defined_error(const std::string key);

    void        expect_error(const std::string subject);

    void        null_error();

    void        operation_error();

    void        range_error(const std::string message);

    std::string to_string(const data_t type);

    void        type_error(const data_t lhs, const data_t rhs);

    void        type_error(const std::string lhs, const std::string rhs);

    void        undefined_error(const std::string key);

    void        unsupported_error(const std::string subject);

    void        write_error(const std::string key);
}


#endif /* error_h */
