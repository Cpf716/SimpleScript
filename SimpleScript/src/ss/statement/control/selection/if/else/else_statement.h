//
//  else_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#ifndef else_statement_h
#define else_statement_h

#include "control_statement.h"

namespace ss {
    class else_statement: public control_statement {
    public:
        //  CONSTRUCTORS
        
        else_statement(const size_t statementc, statement_t** statementv);
        
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

#endif /* else_statement_h */
