//
//  catch_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/26/23.
//

#ifndef catch_statement_h
#define catch_statement_h

#include "control_statement.h"

namespace ss {
    class catch_statement: public control_statement {
        //  MEMBER FIELDS
        
        string symbol;
    public:
        //  CONSTRUCTORS
        
        catch_statement(const string symbol, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;
        
        bool compare(const statement_type value) const;
        
        string evaluate(command_processor* cp);
        
        string execute(command_processor* cp);
                
        void set_break();
        
        void set_continue();
        
        void set_return(const string value);
    };
}

#endif /* catch_statement_h */
