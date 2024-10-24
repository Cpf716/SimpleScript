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
        
        bool continue_flag;
    public:
        //  CONSTRUCTORS
        
        do_while_statement(const string expression, const size_t statementc, statement_t** statementv);
        
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

#endif /* do_while_statement_h */
