//
//  switch_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#ifndef switch_statement_h
#define switch_statement_h

#include "control_statement.h"

namespace ss {
    class switch_statement: public control_statement {
        //  MEMBER FIELDS
        
        string expression;        
    public:
        //  CONSTRUCTORS
        
        switch_statement(const string expression, const size_t statementc, statement_t** statementv);
        
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

#endif /* switch_statement_h */
