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
        
        bool should_return;
    public:
        //  CONSTRUCTORS
        
        catch_statement(const string symbol, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        bool compare(const string value) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
        
        void exit();
        
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* catch_statement_h */
