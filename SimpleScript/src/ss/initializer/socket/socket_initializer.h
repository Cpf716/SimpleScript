//
//  socket_initializer.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef socket_initializer_h
#define socket_initializer_h

#include "command_processor.h"

namespace ss {
    void init_socket();

    void deinit_socket();

    void set_socket(command_processor* cp);
}

#endif /* socket_initializer_h */
