//
//  statement_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/15/23.
//

#ifndef statement_t_h
#define statement_t_h

#include "command_processor.h"

using namespace std;

namespace ss {
    struct statement_t {
        //  MEMBER FIELDS
        
        enum statement_type { break_t, case_t, catch_t, default_t, else_t, else_if_t, exit_t, finally_t, return_t };
        
        //  CONSTRUCTORS
        
        virtual void close() = 0;
        
        //  MEMBER FUNCTIONS
        
        virtual bool analyze(command_processor* cp) const = 0;
        
        virtual bool compare(const statement_type value) const = 0;
        
        virtual string evaluate(command_processor* cp) = 0;
        
        virtual string execute(command_processor* cp) = 0;
        
        virtual void exit() = 0;
        
        virtual size_t get_level() const = 0;
        
        virtual void kill() = 0;
        
        virtual void set_break() = 0;
        
        virtual void set_continue() = 0;
        
        virtual void set_goto(const string key) = 0;
        
        virtual void set_level(const size_t level) = 0;
        
        virtual void set_parent(statement_t* parent) = 0;
        
        virtual void set_paused(const bool value) = 0;
        
        virtual void set_return(const string value) = 0;
    };

    //  NON-MEMBER FUNCTIONS

    bool is_clause(class statement_t* statement);
}

#endif /* statement_h */
