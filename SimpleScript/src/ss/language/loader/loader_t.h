//
//  session_t.h
//  SimpleScript
//
//  Created by Corey Ferguson on 1/23/25.
//

#include "command_processor.h"

namespace ss {
    struct loader_t {
        virtual void bind(command_processor* cp) = 0;
    protected:
        vector<function_t*> value;
    };
}
