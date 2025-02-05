//
//  loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#ifndef loader_h
#define loader_h

#include "command_processor.h"
#include "file.h"
#include "message.h"
#include "system_loader.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    bool call(command_processor* cp, const string key, size_t argc = 0, string* argv = NULL);

    void load(command_processor* cp);

    void unload();
}


#endif /* loader_h */
