//
//  loader_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 1/23/25.
//


#ifndef loader_t_h
#define loader_t_h

#include "command_processor.h"

namespace ss {
    struct loader_t {
        virtual void bind(command_processor* cp) = 0;
    protected:
        vector<function_t*> value;
    };
}

#endif /* loader_t_h */
