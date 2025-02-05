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
        
        bool continue_flag;
        
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
        
        void set_goto(const string key);
        
        void set_return(const string value);
    };
}

#endif /* for_statement_h */
