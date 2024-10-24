//
//  arithmetic.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/28/22.
//

#ifndef arithmetic_h
#define arithmetic_h

#include "bao.h"
#include "bbao.h"
#include "mysql.h"
#include "uao.h"
#include <atomic>
#include <cctype>
#include <cmath>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <typeinfo>

using namespace std;

namespace ss {
    class arithmetic {
        //  MEMBER FIELDS

        size_t numberc = 0;
        tuple<string, double, pair<bool, bool>, size_t>** numberv = NULL;
        
        size_t statec = 0;
        pair<size_t, tuple<string, double, pair<bool, bool>, size_t>**>** state_numberv = NULL;
        
        tuple<size_t, size_t, string*>** statev_key;
        
        size_t keyc = 0;
        string* keyv = NULL;
        
        //  MEMBER FUNCTIONS
            
        void analyze(const size_t n, string* data) const;
        
        void initialize();
        //  binary operators require discrete jagged array to set precedence levels
        
        int io_number(const string key) const;
        
        int io_number(const string key, const size_t start, const size_t end) const;
        
        int io_state(const size_t state) const;
        
        int io_state_number(const size_t state, const string key) const;
        
        int io_state_number(const size_t state, const string key, const size_t start, const size_t end) const;
        
        int io_key(const string key) const;
        
        int io_key(const string key, const size_t start, const size_t end) const;
        
        size_t prefix(string* dst, string src) const;
        
        size_t split(string* dst, string src) const;
        
        double value(string val);
    protected:
        //  MEMBER FIELDS
        
        size_t _statec = 0;
        
        operator_t** aov;
        size_t aoc = 36;
        
        bao_t*** baov;
        size_t baoc[9];
        
        size_t additive_pos;
        size_t assignment_pos;
        size_t bitwise_pos;
        size_t relational_pos;
        size_t unary_count;
        
        atomic<bool> is_locked = false;
        
        //  MEMBER FUNCTIONS
        
        int _get_state(const string key) const;
        
        void get_state(const size_t state);
    public:
        //  CONSTRUCTORS
        
        arithmetic();
        
        ~arithmetic();
        
        //  MEMBER FUNCTIONS
        
        void add_key(const string key);

        void consume(const string key);
        
        double evaluate(const string expression);
        
        double get_number(const string key);
        
        size_t get_state();
        
        bool is_defined(const string key) const;
        
        void remove_key(const string key);
        
        void set_number(const string key, const double new_value);
        
        void set_read_only(const string key, const bool value);
        
        void set_state(const size_t state, bool verbose = true, bool update = true);
    };

    //  NON-MEMBER FUNCTIONS

    int balance(const string str, size_t start = 0);

    int balance(const string str, const size_t start, const size_t end);
}

#endif /* arithmetic_h */
