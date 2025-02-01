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
#include "case_statement.h"
#include "catch_statement.h"
#include "continue_statement.h"
#include "default_statement.h"
#include "define_statement.h"
#include "do_while_statement.h"
#include "echo_statement.h"
#include "else_statement.h"
#include "else_if_statement.h"
#include "exit_statement.h"
#include "file_system_loader.h"
#include "file_statement.h"
#include "finally_statement.h"
#include "for_statement.h"
#include "function_statement.h"
#include "goto_statement.h"
#include "if_statement.h"
#include "mysql_loader.h"
#include "node.h"
#include "return_statement.h"
#include "socket_loader.h"
#include "statement.h"
#include "suppress_statement.h"
#include "switch_statement.h"
#include "throw_statement.h"
#include "try_statement.h"
#include "while_statement.h"

using namespace ss;

namespace ss {
    enum module_t { filesystem_t, mysql_t, socket_t };

    class file: public function_t {
        //  MEMBER FIELDS

        std::function<void(void)>   cb;
        command_processor*          cp = NULL;
        bool                        file_system_flag = false;

        size_t                      filec = 0;
        pair<file*, bool>**         filev = NULL;
        string                      filename;
        size_t                      level = 0;
        bool                         mysql_flag = false;
        function_t*                  parent = NULL;
        atomic<bool>                 is_paused;
        bool                         socket_flag = false;
        file_statement*              target = NULL;
        vector<pair<string, string>> valuev;
        
        //  MEMBER FUNCTIONS
        
        size_t build(statement_t** dst, string* src, const size_t beg, const size_t end) const;
        
        ss::array<string> marshall(const size_t argc, string* argv) const;
    public:
        //  CONSTRUCTORS
        
        file(const string filename, node<string>* parent, command_processor* cp);
        
        void close();
        
        //  MEMBER FUNCTIONS
        
        string call(const size_t argc, string* argv);
        
        void exit();
        
        size_t get_level() const;
        
        void kill();
        
        void set_cp(command_processor* cp);
        
        void set_level(const size_t level);
        
        // Begin Enhancement 1 - Thread safety - 2025-01-22
        void set_paused();
        // End Enhancement 1
        
        void set_paused(const bool value);
        
        void subscribe(std::function<void(void)> cb);
    };
}

#endif /* file_h */
