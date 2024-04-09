//
//  do_while_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/20/23.
//

#ifndef do_while_statement_h
#define do_while_statement_h

#include "control_statement.h"

namespace ss {
    class do_while_statement: public control_statement {
        //  MEMBER FIELDS
        
        string expression;
        
        bool should_continue;
    public:
        //  CONSTRUCTORS
        
        do_while_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        bool compare(const int value) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* do_while_statement_h */
