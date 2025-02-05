//
//  global_functions.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/17/24.
//

#ifndef global_functions_h
#define global_functions_h

bool is_number(const std::string value) {
    if (value.empty())
        return false;
    
    int i = 0;
    if (value[i] == '+' || value[i] == '-')
        ++i;
    
    int j = i;
    while (j < value.length() && value[j] != '.')
        ++j;
    
    int k = j == value.length() ? i : j;
    while (k < value.length() && value[k] != 'E' && value[k] != 'e')
        ++k;
    
    int l = j < k ? j : k, m = i;
    
    if (m < l) {
        if (!isdigit(value[m]))
            return false;
        
        ++m;
        
        //  before decimal
        while (m < l - 1 && (value[m] == '_' || isdigit(value[m])))
            ++m;
        
        if (m < l) {
            if (!isdigit(value[m]))
                return false;
            
            ++m;
        }
    }
    
    if (m != l)
        return false;
    
    size_t n = l - i;
    
    //  after decimal and before exponent
    if (j != value.length()) {
        m = j + 1;
        
        if (m < k) {
            if (!isdigit(value[m]))
                return false;
            
            ++m;
            
            while (m < k - 1 && (value[m] == '_' || isdigit(value[m])))
                ++m;
            
            if (m < k) {
                if (!isdigit(value[m]))
                    return false;
                
                ++m;
            }
        }
        
        if (m != k)
            return false;
        
        n += k - j - 1;
    }
    
    if (n == 0)
        return false;
    
    //  after exponent
    if (k != value.length()) {
        size_t l = k + 1;
        
        if (l == value.length() || (value[l] != '+' && value[l] != '-'))
            return false;
        
        ++l;
        
        if (l == value.length())
            return false;
        
        if (l < value.length()) {
            if (!isdigit(value[l]))
                return false;
            
            ++l;
            
            while (l < value.length() -  1 && (value[l] == '_' || isdigit(value[l])))
                ++l;
            
            if (l < value.length()) {
                if (!isdigit(value[l]))
                    return false;
                
                ++l;
            }
        }
        
        if (l != value.length())
            return false;
    }
    
    return true;
}

#endif /* global_functions_h */
