//
//  mysql_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef mysql_loader_h
#define mysql_loader_h

#include "mysql.h"
#include "loader_t.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEFS
    
    struct mysql_loader: public loader_t {
        // CONSTRUCTORS
        
        mysql_loader(const bool flag = false);
        
        ~mysql_loader();
        
        // MEMBER FUNCTIONS

        void bind(command_processor* cp);
    private:
        bool flag = false;
    };
    // End Enhancement 1-1
}

#endif /* mysql_loader_h */
