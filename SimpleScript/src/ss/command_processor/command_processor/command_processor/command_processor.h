//
//  command_processor.h
//  SimpleScript
//
//  Created by Corey Ferguson on 11/8/22.

#ifndef command_processor_h
#define command_processor_h

#include "binary_universal_operator.h"
#include "function.h"
#include "logic.h"
#include "socket.h"     //  sleep_for
#include "stack.h"
#include "ternary_universal_operator.h"
#include "unary_universal_operator.h"
#include <algorithm>    //  tolower, toupper
#include <filesystem>

using namespace std::chrono;

namespace ss {
    struct command_processor: public logic {
        //  CONSTRUCTORS
        
        command_processor();
        
        ~command_processor();
        
        //  MEMBER FUNCTIONS
        
        void add_listener(const string key, const std::function<void(const string)> callback);
        
        void apply(const std::function<void(const bool)> cb);
        
        string call(const string key, const size_t argc = 0, string* argv = NULL);
        
        void consume(const string key);
        
        string evaluate(const string expression);
        
        size_t get_state();
        
        string get_string(const string key);
        
        bool is_mutating(const string expression) const;
        
        void kill();
        
        void remove_listener(const string key);
        
        void remove_key(const string key);
        
        void save_state();
        
        void set_array(const string key, const string value);
        
        void set_array(const string key, const size_t index, const string value);
        
        void set_function(function_t* function);
        
        void set_pause(const bool value);
        
        void set_read_only(const string key, const bool value);
        
        void set_state();
        
        void set_state(const size_t state, bool verbose = true, bool update = true);
        
        void set_string(const string key, const string value);
        
        void stack_push(function_t* function);
        
        string stack_trace();
    private:
        //  MEMBER FIELDS
        
        const std::string SEPARATORS[7] = { "!", "(", ")", ",", ".", ";", "^" };
            
        size_t additive_pos;
        size_t additive_assignment_pos;
        size_t aggregate_pos;        //  lambda
        size_t arithmetic_pos;
        size_t assignment_pos;
        size_t bitwise_pos;
        size_t cell_pos;             //  ternary
        size_t col_pos;
        size_t conditional_pos;      //  ternary
        size_t const_pos;
        size_t contains_pos;
        size_t reserve_pos;
        size_t direct_assignment_pos;
        size_t equality_pos;
        size_t fill_pos;
        size_t filter_pos;           //  lambda
        size_t find_pos;             //  lambda
        size_t find_index_pos;       //  lambda
        size_t format_pos;
        size_t index_of_pos;
        size_t indexer_pos;
        size_t insert_pos;           //  ternary
        size_t join_pos;
        size_t last_index_of_pos;
        size_t logical_pos;
        size_t map_pos;              //  lambda
        size_t nullish_coalescing_pos;
        size_t resize_pos;
        size_t row_pos;
        size_t relational_pos;
        size_t sequencer_pos;
        size_t shrink_pos;
        size_t slice_pos;            //  hybrid
        size_t splice_pos;           //  hybrid
        size_t substr_pos;           //  hybrid
        size_t tospliced_pos;        //  hybrid
        size_t unary_count;
        size_t var_pos;
        
        size_t arrayc = 0;
        tuple<string, ss::array<string>, pair<bool, bool>, size_t>** arrayv = NULL;
        
        size_t buocc;
        size_t* buocv = NULL;
        binary_universal_operator*** buov = NULL;
        
        string expression;
        
        size_t functionc = 0;
        function_t** functionv = NULL;
        
        int state = -1;
        
        class ss::stack<function_t*> stack = ss::stack<function_t*>();
        
        size_t statec = 0;
        
        pair<size_t, tuple<string, ss::array<string>*, pair<bool, bool>, size_t>**>** state_arrayv = NULL;
        
        pair<size_t, function_t**>** state_functionv = NULL;
        
        size_t* state_numberv = NULL;
        
        pair<size_t, tuple<string, string, pair<bool, bool>, size_t>**>** state_stringv = NULL;
        
        size_t stringc = 0;
        tuple<string, string, pair<bool, bool>, size_t>** stringv = NULL;
        
        size_t uoc = 0;
        operator_t** uov = NULL;

        //  MEMBER FUNCTIONS
        
        string _call(function_t* function, const size_t argc = 0, string* argv = NULL);
        
        int _get_state(const string key) const;
        
        void _set_array(const string key, const size_t valuec, string* valuev);
                        
        string element(string val);
        
        void get_state(const size_t state);
        
        void initialize();
        
        int io_array(const string key) const;
        
        int io_array(const string key, const size_t start, const size_t end) const;
        
        int io_function(const string key) const;
        
        int io_function(const string key, const size_t start, const size_t end) const;
        
        int io_state_array(const size_t state, const string key) const;
        
        int io_state_array(const size_t state, const string key, size_t start, size_t end) const;
        
        int io_state_function(const size_t state, const string key) const;
        
        int io_state_function(const size_t state, const string key, size_t start, size_t end) const;
        
        int io_state_string(const size_t state, const string key) const;
        
        int io_state_string(const size_t state, const string key, const size_t start, const size_t end) const;
        
        int io_string(const string key) const;
        
        int io_string(const string key, const size_t start, const size_t end) const;
        
        int merge(int n, string* data) const;
        
        int merge_numbers(int n, string* data) const;
        
        int prefix(string* dst, const string src) const;        
    };

    //  NON-MEMBER FUNCTIONS

    bool evaluate(const string value);
}

#endif /* command_processor_h */
