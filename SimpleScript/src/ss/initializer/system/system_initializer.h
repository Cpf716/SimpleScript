//
//  system_initializer.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef system_initializer_h
#define system_initializer_h

#include "command_processor.h"

namespace ss {
    //  NON-MEMBER FIELDS

    enum event_t { oninterval_t, ontimeout_t };

    //  NON-MEMBER FUNCTIONS

    size_t add_listener(const event_t event, const std::function<void(void)> cb);

    void dispatch(const event_t event);

    void init_system(command_processor* cp);

    void remove_listener(const size_t listener);
}

#endif /* system_initializer_h */
