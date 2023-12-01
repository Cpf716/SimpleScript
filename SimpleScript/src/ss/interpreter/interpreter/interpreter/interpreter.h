//
//  interpreter.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.

#ifndef interpreter_h
#define interpreter_h

#include <algorithm>    //  tolower, toupper
#include "buo.h"
#include <cstdio>   //  remove
#include "exception.h"
#include "function.h"
#include "logic.h"
#include "socket.h" //  socket
#include "stack.h"
#include "tuo.h"
#include "uuo.h"

using namespace std::chrono;

namespace ss {
    class interpreter: public logic {
        //  MEMBER FIELDS
    
        size_t aggregate_oi;        //  lambda
        size_t assignment_oi;       //
        size_t cell_oi;
        size_t col_oi;
        size_t concat_oi;           //
        size_t conditional_oi;      //  ternary
        size_t const_oi;
        size_t contains_oi;
        size_t reserve_oi;
        size_t equality_oi;
        size_t fill_oi;
        size_t filter_oi;           //  lambda
        size_t find_oi;             //  lambda
        size_t format_oi;
        size_t index_of_oi;
        size_t indexer_oi;          //
        size_t insert_oi;           //  ternary
        size_t join_oi;
        size_t last_index_of_oi;
        size_t map_oi;              //  lambda
        size_t optional_oi;
        size_t resize_oi;
        size_t row_oi;
        size_t relational_oi;
        size_t shrink_oi;
        size_t slice_oi;            //  either binary or ternary
        size_t splice_oi;           //  either binary or ternary
        size_t substr_oi;           //  either binary or ternary
        size_t tospliced_oi;        //  ternary
        size_t unary_count;
        
        bst<pair<string, int>>* array_bst = NULL;
        bst<pair<string, int>>* function_bst = NULL;
        bst<pair<string, int>>* string_bst = NULL;
        
        size_t arrayc = 0;
        tuple<string, ss::array<string>, pair<bool, bool>>** arrayv = NULL;
        
        size_t functionc = 0;
        function_t** functionv = NULL;
        
        size_t stringc = 0;
        tuple<string, string, pair<bool, bool>>** stringv = NULL;
        
        size_t backupc = 0;
        pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>>**>** bu_arrayv = NULL;
        pair<size_t, function_t**>** bu_functionv = NULL;
        string* bu_numberv = NULL;
        pair<size_t, tuple<string, string, pair<bool, bool>>**>** bu_stringv = NULL;
        
        buo*** buov = NULL;
        size_t buoc[7];
        
        operator_t** uov = NULL;
        size_t uoc = 0;
        
        string buid = EMPTY;
        
        string current_expression;
        
        ss::stack<size_t> stack_trace = ss::stack<size_t>();
        
        string types[8] { "array", "char", "double", "int", "string", "undefined", "dictionary", "table" };

        //  MEMBER FUNCTIONS
        
        string element(string val);
        
        void initialize();
        
        int io_array(const string symbol) const;
        
        int io_function(const string symbol) const;
        
        int io_string(const string symbol) const;
        
        int merge(int n, string* data) const;
        
        int merge_numbers(int n, string* data) const;
        
        int prefix(string* dst, const string src) const;
        
        void save();
        
        int split(string* dst, string src) const;
        
        void type_error(const size_t lhs, const size_t rhs);
        
        void write();
    public:
        
        //  CONSTRUCTORS
        
        interpreter();
        
        ~interpreter();
        
        //  MEMBER FUNCTIONS
        
        string backup();
        
        void consume(const string symbol);
        
        void drop(const string symbol);
        
        string evaluate(const string expression);
        
        void exit();
        
        ss::array<string> get_array(const string symbol);
        
        string get_string(const string symbol);
        
        bool is_mutating(const string expression) const;
        
        void print_stack_trace();
        
        void reload();
        
        void restore(const string uuid, bool verbose = true);
        
        void restore(const string uuid, const bool verbose, const size_t symbolc, string* symbolv);
        
        void set_array(const string symbol, const size_t index, const string value);
        
        void set_function(function_t* new_function);
        
        void set_string(const string symbol, const string value);
    };
}

#endif /* interpreter_h */
