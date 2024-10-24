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
        
        bool continue_flag;
    public:
        //  CONSTRUCTORS
        
        while_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;
        
        string evaluate(command_processor* cp);
        
        string execute(command_processor* cp);
        
        bool compare(const statement_type value) const;
        
        void set_break();
        
        void set_continue();
        
        void set_goto(const string key);
        
        void set_return(const string value);
    };
}

#endif /* while_statement_h */
