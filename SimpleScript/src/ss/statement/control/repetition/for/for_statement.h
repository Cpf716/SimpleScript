//
//  for_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/10/23.
//

#ifndef for_statement_h
#define for_statement_h

#include "control_statement.h"

namespace ss {
    class for_statement: public control_statement {
        //  MEMBER FIELDS
        
        size_t expressionc;
        string* expressionv = NULL;
        
        bool should_continue;
        
        string* valuev = NULL;
    public:
        //  CONSTRUCTORS
        
        for_statement(const string specifier, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;
        
        bool compare(const statement_type value) const;
        
        string evaluate(command_processor* cp);
        
        string execute(command_processor* cp);
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* for_statement_h */
