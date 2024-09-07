//
//  file_system_initializer.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef file_system_initializer_h
#define file_system_initializer_h

#include "command_processor.h"

namespace ss {
    void init_file_system();

    void deinit_file_system();

    void set_file_system(command_processor* cp);
}

#endif /* file_system_initializer_h */
