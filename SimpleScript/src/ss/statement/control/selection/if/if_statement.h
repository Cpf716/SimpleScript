//
//  if_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef if_statement_h
#define if_statement_h

#include "else_if_statement.h"
#include "else_statement.h"

namespace ss {
    class if_statement: public control_statement {
        //  MEMBER FIELDS
        
        string expression;
        
        int index;
    public:
        //  CONSTRUCTORS
        
        if_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        bool compare(const string value) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* if_statement_h */
