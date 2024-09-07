//
//  logic.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/29/22.
//

#ifndef logic_h
#define logic_h

#include "arithmetic.h"
#include "blo.h"
#include "ulo.h"

using namespace std;

namespace ss {
    class logic: public arithmetic {
        //  MEMBER FUNCTIONS
        
        void analyze(const size_t n, string* data) const;
        
        size_t merge(int n, string* data) const;
        
        size_t prefix(string* dst, const string src) const;
        
        size_t split(string* dst, const string src) const;
    protected:
        //  MEMBER FIELDS
        
        size_t loc = 4;
        operator_t** lov = NULL;
    public:
        //  CONSTRUCTORS
        
        logic();
        
        ~logic();
        
        //  MEMBER FUNCTIONS
        
        double evaluate(const string expression);
    };
}

#endif /* logic_h */
