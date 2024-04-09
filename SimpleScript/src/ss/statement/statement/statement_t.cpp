//
//  statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/14/23.
//

#include "statement_t.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    bool evaluate(const string value) {
        if (ss::is_array(value))
            return true;
        
        if (value.empty())
            return false;
        
        if (is_string(value)) {
            string str = decode(value);
            
            return !str.empty() && str != "undefined";
        }
        
        return stod(value);
    }

    bool is_clause(class statement_t* statement) {
        int i = 0;
        while (i < 4 && !statement->compare(i))
            ++i;
        
        return i != 4;
    }
}
