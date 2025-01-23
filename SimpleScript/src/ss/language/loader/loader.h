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
#include "system_loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    enum event_t { clear_interval_t, clear_timeout_t, set_interval_t, set_timeout_t };

    //  NON-MEMBER FUNCTIONS

    bool call(command_processor* cp, const string key, size_t argc = 0, string* argv = NULL);

    void load(command_processor* cp);

    void send_message(const event_t event, const string* value);

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback);

    void unload();

    void unsubscribe(const size_t subscription);
}


#endif /* loader_h */
