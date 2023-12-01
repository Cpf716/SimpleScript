//
//  statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/14/23.
//

#include "statement_t.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    bool evaluate(const string result) {
        if (ss::is_array(result))
            return true;
        
        if (result.empty())
            return false;
        
        if (is_string(result)) {
            string str = decode(result);
            
            return !str.empty() && str != "undefined";
        }
        
        return stod(result);
    }

    bool is_clause(class statement_t* statement) {
        return statement->compare("catch") ||
            statement->compare("else") ||
            statement->compare("elseif") ||
            statement->compare("finally");
    }
}
