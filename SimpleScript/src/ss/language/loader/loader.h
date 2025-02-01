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
#include "mysql_loader.h"
#include "system_loader.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEFS

    struct loader {
        // MEMBER FIELDS
        mysql_loader _mysql_loader;
        
        // CONSTRUCTORS
        
        loader(command_processor* cp);
        
        ~loader();
        
        // MEMBER FUNCTIONS
        
        bool call(command_processor* cp, const string key, size_t argc = 0, string* argv = NULL);
        
        void set_mysql(command_processor* cp);
    private:
        ::mutex mutex;
    };
    // End Enhancement 1-1

    //  NON-MEMBER FIELDS

    enum event_t { clear_interval_t, clear_timeout_t, set_interval_t, set_timeout_t };

    //  NON-MEMBER FUNCTIONS

    void send_message(const event_t event, const string* value);

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback);

    void unsubscribe(const size_t subscription);
}


#endif /* loader_h */
