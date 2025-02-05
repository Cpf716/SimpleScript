//
//  main.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/9/23.
//

#include "logic.h"

using namespace ss;

int main() {
    logic alu;
    
    bool flag = true;
    
    while (1) {
        if (flag)
            cout << "?\t";
        
        string str;
        getline(cin, str);
        
        if ((flag = !str.empty()))
            cout << "=\t" << alu.evaluate(str) << endl;
    }
}
