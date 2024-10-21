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

    enum event_t { offinterval, offtimeout, oninterval, ontimeout };

    //  NON-MEMBER FUNCTIONS

    string call(command_processor* cp, const string symbol, size_t argc = 0, string* argv = NULL);

    bool is_locked();

    void load(command_processor* cp);

    void lock();

    void notify(const event_t event, const string* value);

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback);

    void unload();

    void unlock();

    void unsubscribe(const size_t subscription);
}


#endif /* loader_h */
