//
//  message.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 2/4/25.
//

#include "message.h"

namespace ss {
    // NON-MEMBER FIELDS
    
    mutex           message_mutex;
    size_t          subscriptionc = 0;
    vector<message> subscriptionv;

    // NON-MEMBER FUNCTIONS
    // PRIVATE
    
    int find_subscription(const size_t number, const size_t start = 0, const size_t end = subscriptionv.size()) {
        if (start == end)
            return -1;
        
        size_t len = floor((end - start) / 2);
        
        if (subscriptionv[start + len].number() == number)
            return (int)(start + len);
        
        if (subscriptionv[start + len].number() > number)
            return find_subscription(number, start, start + len);
        
        return find_subscription(number, start + len + 1, end);
    }

    // PUBLIC
    
    size_t subscribe(const message_type key, std::function<void(const size_t, const string*)> value) {
        message_mutex.lock();
        
        subscriptionv.push_back(message(subscriptionc, key, value));
        
        size_t result = subscriptionc++;
        
        message_mutex.unlock();
        
        return result;
    }
    
    void unsubscribe(const size_t key) {
        message_mutex.lock();
        
        int pos = find_subscription(key);
        
        if (pos == -1) {
            message_mutex.unlock();
            return;
        }
        
        subscriptionv.erase(subscriptionv.begin() + pos);
        
        message_mutex.unlock();
    }
    
    void broadcast(const message_type key, const size_t valuec, const string* valuev) {
        message_mutex.lock();
        
        for (message value: subscriptionv)
            if (value.key() == key)
                value.value()(valuec, valuev);
        
        message_mutex.unlock();
    }

    // CONSTRUCTORS
    
    message::message(const size_t number, const message_type key, function<void(const size_t, const string*)> value) {
        this->_number = number;
        this->_key = key;
        this->_value = value;
    }

    // MEMBER FIELDS
    
    size_t message::number() const {
        return this->_number;
    }
    
    message_type message::key() const {
        return this->_key;
    }
    
    function<void(const size_t, const string*)> message::value() {
        return this->_value;
    }
}
