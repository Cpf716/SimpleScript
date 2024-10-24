//
//  file_statement.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#ifndef file_statement_h
#define file_statement_h

#include "function_t.h"
#include "statement_t.h"

namespace ss {
    class file_statement: public statement_t {
        //  MEMBER FIELDS
        
        function_t* parent = NULL;
        
        string value;
        
        bool pause_flag;
        
        bool return_flag;
        
        size_t statementc;
        statement_t** statementv = NULL;
    public:
        //  CONSTRUCTORS
        
        file_statement(function_t* parent, const size_t statementc, statement_t** statementv);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        bool analyze(command_processor* cp) const;

        bool compare(const statement_type value) const;

        string evaluate(command_processor* cp);

        string execute(command_processor* cp);
        
        void exit();
        
        size_t get_level() const;
        
        void kill();

        void set_break();

        void set_continue();
        
        void set_goto(const string key);
        
        void set_level(const size_t level);

        void set_parent(statement_t* parent);
        
        void set_pause(const bool value);

        void set_return(const string value);
    };
}

#endif /* file_statement_h */
