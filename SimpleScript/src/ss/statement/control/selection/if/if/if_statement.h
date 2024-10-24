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
        
        int position;
    public:
        //  CONSTRUCTORS
        
        if_statement(const string expression, const size_t statementc, statement_t** statementv);
        
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

#endif /* if_statement_h */
