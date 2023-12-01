//
//  else_if_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef else_if_statement_h
#define else_if_statement_h

#include "control_statement.h"

namespace ss {
    class else_if_statement: public control_statement {
        //  MEMBER FIELDS
        
        string expression;
        
        bool should_return;
    public:
        //  CONSTRUCTORS
        
        else_if_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        bool compare(const string value) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
        
        void exit();
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* else_if_statement_h */
