//
//  main.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/9/23.
//

#include "arithmetic.h"

using namespace ss;

int main() {
    arithmetic au;
    
    bool flag = true;
    
    while (1) {
        if (flag)
            cout << "?\t";
        
        string str;
        getline(cin, str);
        
        if ((flag = !str.empty()))
            cout << "=\t" << au.evaluate(str) << endl;
    }
}
