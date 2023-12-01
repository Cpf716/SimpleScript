//
//  statement_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/15/23.
//

#ifndef statement_t_h
#define statement_t_h

#include "interpreter.h"

using namespace std;

namespace ss {
    struct statement_t {
        //  CONSTRUCTORS
        
        virtual void close() = 0;
        
        //  MEMBER FUNCTIONS
        
        virtual bool analyze(interpreter* ssu) const = 0;
        
        virtual bool compare(const string value) const = 0;
        
        virtual string evaluate(interpreter* ssu) = 0;
        
        virtual string execute(interpreter* ssu) = 0;
        
        virtual void exit() = 0;
        
        virtual void set_break() = 0;
        
        virtual void set_continue() = 0;
        
        virtual void set_parent(statement_t* parent) = 0;
        
        virtual void set_return(const string result) = 0;
    };

    //  NON-MEMBER FUNCTIONS

    bool evaluate(const string result);

    bool is_clause(class statement_t* statement);
}

#endif /* statement_h */
