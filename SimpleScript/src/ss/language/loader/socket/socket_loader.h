//
//  socket_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef socket_loader_h
#define socket_loader_h

#include "loader_t.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    struct socket_loader: public loader_t {
        // CONSTRUCTORS
        
        socket_loader(const bool flag = false);
        
        ~socket_loader();
        
        // MEMBER FUNCTIONS
        
        void bind(command_processor* cp);
    private:
        bool flag = false;
    };
    // End Enhancement 1-1
}

#endif /* socket_loader_h */
