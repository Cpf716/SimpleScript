//
//  try_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/26/23.
//

#ifndef try_statement_h
#define try_statement_h

#include "control_statement.h"

namespace ss {
    class try_statement: public control_statement {
        //  MEMBER FIELDS
        
        int index;
    public:
        //  CONSTRUCTORS
        
        try_statement(const size_t statementc, statement_t** statementv);
        
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

#endif /* try_statement_h */
