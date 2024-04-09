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
#include "define_statement.h"
#include "do_while_statement.h"
#include "echo_statement.h"
#include "else_statement.h"
#include "else_if_statement.h"
#include "exit_statement.h"
#include "file_statement.h"
#include "finally_statement.h"
#include "for_statement.h"
#include "function_statement.h"
#include "if_statement.h"
#include "node.h"
#include "return_statement.h"
#include "sleep_statement.h"
#include "statement.h"
#include "throw_statement.h"
#include "try_statement.h"
#include "while_statement.h"

using namespace ss;

namespace ss {
    class file: public function_t {
        //  MEMBER FIELDS
        
        string filename;
        
        size_t functionc = 0;
        pair<file*, bool>** functionv = NULL;
        
        size_t level = 0;
        
        function_t* parent = NULL;
        
        interpreter* ssu = NULL;
        
        file_statement* target = NULL;
        
        //  MEMBER FUNCTIONS
        
        size_t build(statement_t** dst, string* src, const size_t beg, const size_t end) const;
        
        ss::array<string> marshall(const size_t argc, string* argv) const;
    public:
        //  CONSTRUCTORS
        
        file(const string filename, node<string>* parent, interpreter* ssu);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        string call(const size_t argc, string* argv);
        
        void exit();
        
        size_t get_level() const;
        
        void kill();
        
        void set_level(const size_t level);
    };

    //  NON-MEMBER FUNCTIONS

    string base_dir();
}

#endif /* file_h */
