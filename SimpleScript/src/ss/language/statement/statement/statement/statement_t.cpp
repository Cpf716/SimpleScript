//
//  statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/14/23.
//

#include "statement_t.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    bool is_clause(class statement_t* statement) {
        return statement->compare(statement_t::break_t) ||
            statement->compare(statement_t::case_t) ||
            statement->compare(statement_t::catch_t) ||
            statement->compare(statement_t::default_t) ||
            statement->compare(statement_t::else_t) ||
            statement->compare(statement_t::else_if_t);
    }
}
