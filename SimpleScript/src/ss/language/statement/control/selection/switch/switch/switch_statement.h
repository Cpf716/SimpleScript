//
//  switch_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#ifndef switch_statement_h
#define switch_statement_h

#include "case_statement.h"
#include "control_statement.h"

namespace ss {
    class switch_statement: public control_statement {
        //  MEMBER FIELDS
        
        size_t                 casec = 0;
        pair<string, size_t>** casev = NULL;
        command_processor*     cp = NULL;
        string                 expression;
        bool                   goto_flag;
    public:
        //  CONSTRUCTORS
        
        switch_statement(const string expression, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;
        
        bool compare(const statement_type value) const;
        
        string evaluate(command_processor* cp);
        
        string execute(command_processor* cp);
        
        int find_case(const string key);
        
        int find_case(const string key, const size_t beg, const size_t end);
        
        void set_break();
        
        void set_continue();
        
        void set_goto(const string key);
        
        void set_return(const string value);
    };
}

#endif /* switch_statement_h */
