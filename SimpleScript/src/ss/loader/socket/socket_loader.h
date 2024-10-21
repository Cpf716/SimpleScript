//
//  socket_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef socket_loader_h
#define socket_loader_h

#include "command_processor.h"

namespace ss {
    void load_socket();

    void set_socket(command_processor* cp);

    void unload_socket();
}

#endif /* socket_loader_h */
