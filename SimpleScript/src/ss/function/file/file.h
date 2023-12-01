//
//  file.h
//  SimpleScript
//
//  Created by Corey Ferguson on 4/27/23.
//

#ifndef file_h
#define file_h

#include "assert_statement.h"
#include "break_statement.h"
#include "catch_statement.h"
#include "consume_statement.h"
#include "continue_statement.h"
#include "do_while_statement.h"
#include "echo_statement.h"
#include "else_statement.h"
#include "else_if_statement.h"
#include "exception_statement.h"
#include "file_statement.h"
#include "finally_statement.h"
#include "for_statement.h"
#include "function_statement.h"
#include "if_statement.h"
#include "node.h"
#include "return_statement.h"
#include "sleep_statement.h"
#include "statement.h"
#include "try_statement.h"
#include "while_statement.h"

using namespace ss;

namespace ss {
    class file: public function_t {
        //  MEMBER FIELDS
        
        string filename;
        
        size_t functionc = 0;
        pair<file*, bool>** functionv = NULL;
        
        interpreter* ssu = NULL;
        
        statement_t* target = NULL;
        
        //  MEMBER FUNCTIONS
        
        size_t build(statement_t** dst, string* src, const size_t si, const size_t ei) const;
        
        ss::array<string> marshall(const size_t argc, string* argv) const;
    public:
        //  CONSTRUCTORS
        
        file(const string filename, node<string>* parent, interpreter* ssu);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        string call(const size_t argc, string* argv);
        
        void exit();
    };
}

#endif /* file_h */
