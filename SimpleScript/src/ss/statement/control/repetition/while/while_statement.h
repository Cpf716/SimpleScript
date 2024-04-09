//
//  while_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef while_statement_h
#define while_statement_h

#include "control_statement.h"

namespace ss {
    class while_statement: public control_statement {
        //  MEMBER FIELDS
        
        string expression;
        
        bool should_continue;
    public:
        //  CONSTRUCTORS
        
        while_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
        
        bool compare(const int value) const;
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* while_statement_h */
