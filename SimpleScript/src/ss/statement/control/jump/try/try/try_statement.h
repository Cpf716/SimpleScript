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
        
        size_t position;
    public:
        //  CONSTRUCTORS
        
        try_statement(const size_t statementc, statement_t** statementv);
        
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

#endif /* try_statement_h */
