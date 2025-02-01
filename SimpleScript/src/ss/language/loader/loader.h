//
//  loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#ifndef loader_h
#define loader_h

#include "file.h"
// Begin Enhancement 1-1 - Thread safety - 2025-01-23
#include "file_system_loader.h"
#include "mysql_loader.h"
#include "socket_loader.h"
// End Enhancement 1-1
#include "system_loader.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEFS

    struct loader {
        // CONSTRUCTORS
        
        loader(command_processor* cp, const bool flag = false);
        
        ~loader();
        
        // MEMBER FUNCTIONS
        
        bool call(command_processor* cp, const string key, size_t argc = 0, string* argv = NULL);
        
        void get_file_system();
        
        void bind_file_system(command_processor* cp);
        
        void get_mysql();
        
        void bind_mysql(command_processor* cp);
        
        void get_socket();
        
        void bind_socket(command_processor* cp);
    private:
        // MEMBER FIELDS
        
        file_system_loader* _file_system_loader = NULL;
        mysql_loader*       _mysql_loader = NULL;
        system_loader*      _system_loader = NULL;
        socket_loader*      _socket_loader = NULL;
        bool               flag;
        ::mutex            mutex;
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
