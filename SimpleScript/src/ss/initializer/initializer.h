//
//  initializer.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#ifndef initializer_h
#define initializer_h

#include "command_processor.h"
#include "file.h"
#include "system_initializer.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    string call(command_processor* cp, const string symbol, size_t argc = 0, string* argv = NULL);

    void deinitialize();

    void initialize(command_processor* cp);

    bool is_locked();

    void lock();

    void unlock();
}


#endif /* initializer_h */
