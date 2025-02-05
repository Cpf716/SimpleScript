//
//  message.h
//  SimpleScript
//
//  Created by Corey Ferguson on 2/4/25.
//

#include <iostream>

using namespace std;

namespace ss {
    // TYPEDEF

    enum message_type { message_default };

    struct message {
        // CONSTRUCTORS
        
        message(const size_t number, const message_type key, std::function<void(const size_t, const string*)> value);
        
        // MEMBER FUNCTIONS
        
        message_type                                     key() const;
        size_t                                           number() const;
        std::function<void(const size_t, const string*)> value();
    private:
        // MEMBER FIELDS

        message_type                                     _key;
        size_t                                           _number;
        std::function<void(const size_t, const string*)> _value;
    };

    // NON-MEMBER FUNCTIONS

    size_t subscribe(const message_type key, std::function<void(const size_t, const string*)> value);

    void   unsubscribe(const size_t number);

    void   broadcast(const message_type key, const size_t valuec = 0, const string* valuev = NULL);
}
