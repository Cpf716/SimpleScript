//
//  file_system_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef file_system_loader_h
#define file_system_loader_h

#include "command_processor.h"

namespace ss {
    void load_file_system(command_processor* cp);

    void unload_file_system();
}

#endif /* file_system_loader_h */
