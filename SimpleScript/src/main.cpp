//
//  main.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#include "loader.h"

using namespace ss;

//  NON-MEMBER FIELDS

command_processor cp;
// Begin Enhancement 1-1 - Thread safety - 2025-01-23
loader            _loader(&cp);
// End Enhancement 1-1
bool              is_running = true;
mutex             main_mutex;
vector<int>       timeouts;

//  NON-MEMBER FUNCTIONS

int find_timeout(const size_t key, const size_t begin = 0, const size_t end = timeouts.size()) {
    if (begin == end)
        return -1;
    
    size_t len = floor((end - begin) / 2);
    
    if (timeouts[begin + len] == key)
        return (int)(begin + len);
    
    if (timeouts[begin + len] > key)
        return find_timeout(key, begin, begin + len);
    
    return find_timeout(key, begin + len + 1, end);
}

void clear_timeout(string timeout) {
    main_mutex.lock();
    
    int pos = find_timeout(stoi(timeout));
    
    if (pos != -1)
        timeouts.erase(timeouts.begin() + pos);
    
    main_mutex.unlock();
}

void signal_handler(int signum) {
    thread([signum]() {
        // Begin Enhancement 1-1 - Thread safety - 2025-01-23
        if (_loader.call(&cp, "onExit", 1, (string[]) { to_string(signum) }))
        // End Enhancement 1-1
            return;
        
        ss::integration::socket_close();
        
        is_running = false;
        
        cp.kill();
        
        logger_write("^C\n");
    }).detach();
}

int main(int argc, char* argv[]) {
    //  register signal callbacks
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    load_preferences();
    
    subscribe(clear_interval_t, [&](const string* value) {
        clear_timeout(value[0]);
    });
    
    subscribe(clear_timeout_t, [&](const string* value) {
        clear_timeout(value[0]);
    });
    
    subscribe(set_interval_t, [&](const string* value) {
        main_mutex.lock();
        
        int interval = stoi(value[0]);
        
        timeouts.push_back(interval);
        main_mutex.unlock();
        
        thread([interval](const int duration) {
            while (true) {
                this_thread::sleep_for(milliseconds(duration));
                
                main_mutex.lock();
                
                int pos = find_timeout(interval);
                
                main_mutex.unlock();
                
                if (pos == -1)
                    return;
                
                // Begin Enhancement 1-1 - Thread safety - 2025-01-23
                _loader.call(&cp, "onInterval");
                // End Enhancement 1-1
            }
        }, stoi(value[1])).detach();
    });
    
    subscribe(set_timeout_t, [&](const string* value) {
        main_mutex.lock();
        
        int timeout = stoi(value[0]);
        
        timeouts.push_back(timeout);
        main_mutex.unlock();
        
        thread([timeout](int duration) {
            this_thread::sleep_for(milliseconds(duration));
            
            main_mutex.lock();
            
            int pos = find_timeout(timeout);
            
            if (pos == -1) {
                main_mutex.unlock();
                
                return;
            }
            
            timeouts.erase(timeouts.begin() + pos);
            main_mutex.unlock();
            
            // Begin Enhancement 1-1 - Thread safety - 2025-01-23
            _loader.call(&cp, "onTimeout");
            // End Enhancement 1-1
        }, stoi(value[1])).detach();
    });
    
    //  build
    
    logger_write("Building...\n");
    
    time_point<steady_clock> beg = steady_clock::now();;
    
    node<string>* parent = new node<string>();
    file*         target = NULL;
    
    try {
        string filename = argc == 1 ? get_base_dir() + get_main() :
            decode_raw(raw(argv[1]));
        
        // Begin Enhancement 1-1 - Thread safety - 2025-01-23
        target = new file(filename, parent, &cp, &_loader);
        // End Enhancement 1-1
    } catch (error& e) {
        parent->close();
        
        throw e;
    }
    
    parent->close();
    
    // exit
    target->subscribe([]() {
        ss::integration::socket_close();
    });
    
    double res = duration<double>(steady_clock::now() - beg).count();
    
    logger_write("Done in " + to_string(res) + " s.\nRunning...\n");
    
    beg = steady_clock::now();
    
    try {
        try {
            cp.stack_push(target);
            
            string _argv[max(0, argc - 2)];
            
            for (size_t i = 0; i < sizeof(_argv) / sizeof(_argv[0]); i += 1)
                _argv[i] = raw(argv[i + 2]);
            
            target->call(sizeof(_argv) / sizeof(_argv[0]), _argv);
            
        } catch (::exception& e) {
            ostringstream ss;
            
            ss << "EXCEPTION: " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    } catch (error& e) {
        if (is_running) {
            ostringstream ss;
            
            ss << "ERROR: " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    }
    
    res = duration<double>(steady_clock::now() - beg).count();
  
    logger_write("Done in " + to_string(res) + " s.\n");
    
    target->close();
}
