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
        
        command_processor* cp = NULL;
        
        size_t expressionc;
        string* expressionv = NULL;
        
        size_t optionalc = 0;
        
        string result;
        
        //  MEMBER FUNCTIONS
        
        void set_value(const string symbol, const string value);
    public:
        //  CONSTRUCTORS
        
        function_statement(const string specifier, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;
        
        string call(const size_t argc, string* argv);
        
        bool compare(const statement_type value) const;
        
        string evaluate(command_processor* cp);
        
        string execute(command_processor* cp);
        
        void exit();
        
        size_t get_level() const;
        
        void kill();
                
        void set_break();
        
        void set_continue();
        
        void set_level(const size_t level);
        
        void set_pause(const bool pause);
        
        void set_return(const string result);
    };
}

#endif /* function_statement_h */
