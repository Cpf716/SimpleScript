//
//  function_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/7/23.
//

#ifndef function_statement_h
#define function_statement_h

#include "control_statement.h"

namespace ss {
    class function_statement: public control_statement, function_t {
        //  MEMBER FIELDS
        
        size_t expressionc;
        string* expressionv = NULL;
        
        size_t optionalc = 0;
        
        string result;
        
        interpreter* ssu = NULL;
        
        //  MEMBER FUNCTIONS
        
        void set_value(const string symbol, const string value);
    public:
        //  CONSTRUCTORS
        
        function_statement(const string specifier, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(interpreter* ssu) const;
        
        string call(const size_t argc, string* argv);
        
        bool compare(const string value) const;
        
        string evaluate(interpreter* ssu);
        
        string execute(interpreter* ssu);
                
        void set_break();
        
        void set_continue();
        
        void set_return(const string result);
    };
}

#endif /* function_statement_h */
