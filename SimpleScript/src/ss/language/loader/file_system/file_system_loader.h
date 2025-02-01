//
//  file_system_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef file_system_loader_h
#define file_system_loader_h

#include "loader_t.h"

namespace ss {
    
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEFS

    struct file_system_loader: public loader_t {
        // CONSTRUCTORS
        
        file_system_loader(const bool flag = false);
        
        ~file_system_loader();
        
        // MEMBER FUNCTIONS
        
        void bind(command_processor* cp);
    private:
        bool flag = false;
    };
    // End Enhancement 1-1
}

#endif /* file_system_loader_h */
