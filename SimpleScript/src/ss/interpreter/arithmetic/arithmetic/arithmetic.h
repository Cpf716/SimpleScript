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
#include <cctype>
#include <cmath>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <typeinfo>
#include "uao.h"

#include "mysql.h"

using namespace std;

namespace ss {
    class arithmetic {
        //  MEMBER FIELDS
        
        bst<pair<string, int>>* number_bst = NULL;
        bst<pair<string, int>>* symbol_bst = NULL;
        
        size_t backupc = 0;
        pair<size_t, tuple<string, double, pair<bool, bool>>**>** bu_numberv = NULL;
        
        size_t bu_symbolc = 0;
        tuple<string, size_t, string*>** bu_symbolv = new tuple<string, size_t, string*>*[1];
        
        tuple<string, double, pair<bool, bool>>** numberv = new tuple<string, double, pair<bool, bool>>*[1];
        size_t numberc = 0;
        
        size_t symbolc = 0;
        string* symbolv = new string[1];
        
        //  MEMBER FUNCTIONS
            
        void analyze(const size_t n, string* data) const;
        
        void initialize();
        //  binary operators require discrete jagged array to set precedence levels
        
        int io_symbol(const string symbol) const;
        
        size_t prefix(string* dst, string src) const;
        //  pass pointer array twice the length of expr
        
        size_t split(string* dst, string src) const;
        
        double value(string val);
    protected:
        //  MEMBER FIELDS
        
        operator_t** aov;
        size_t aoc = 36;
        
        bao_t*** baov;
        size_t baoc[9];
        
        size_t unary_count;
    public:
        //  CONSTRUCTORS
        
        arithmetic();
        
        ~arithmetic();
        
        //  MEMBER FUNCTIONS
        
        string backup();
        
        void consume(const string symbol);
        
        void disable_write(const string symbol);
        
        void drop(const string symbol);
        
        double evaluate(const string expression);
        
        double get_number(const string symbol);
        
        void insert(const string symbol);
        
        int io_number(const string symbol) const;
        
        bool is_defined(const string symbol) const;
        
        void restore(const string uuid, bool verbose = true);
        
        void restore(const string uuid, const bool verbose, size_t symbolc, string* symbolv);
        
        void set_number(const string symbol, const double new_value);
    };

    //  NON-MEMBER FUNCTIONS

    int balance(const string str);

    int balance(const string str, const size_t start);

    int balance(const string str, const size_t start, const size_t end);
}

#endif /* arithmetic_h */
