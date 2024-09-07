//
//  default_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#ifndef default_statement_h
#define default_statement_h

#include "control_statement.h"

namespace ss {
    struct default_statement: public control_statement {
        default_statement(const size_t statementc, statement_t** statementv);
        
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

#endif /* default_statement_h */
