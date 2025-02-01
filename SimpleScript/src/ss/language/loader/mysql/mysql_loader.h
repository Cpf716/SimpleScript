//
//  mysql_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef mysql_loader_h
#define mysql_loader_h

#include "command_processor.h"
#include "mysql.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEFS
    
    struct mysql_loader {
        // MEMBER FIELDS
        
        vector<function_t*> value;
        
        // CONSTRUCTORS
        
        mysql_loader();
        
        ~mysql_loader();
        
        // MEMBER FUNCTIONS

        void set_value(command_processor* cp);
    };
    // End Enhancement 1-1
}

#endif /* mysql_loader_h */
