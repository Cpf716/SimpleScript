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

vector<size_t> timeoutv;

int find_timeout(const size_t key, const size_t beg = 0, const size_t end = timeoutv.size()) {
    if (beg == end)
        return -1;
    
    size_t len = floor((end - beg) / 2);
    
    if (timeoutv[beg + len] == key)
        return (int)(beg + len);
    
    if (timeoutv[beg + len] > key)
        return find_timeout(key, beg, beg + len);
    
    return find_timeout(key, beg + len + 1, end);
}

//  NON-MEMBER FUNCTIONS

void set_interval(int interval) {
    size_t id = timeoutv[timeoutv.size() - 1];
    
    while (true) {
        this_thread::sleep_for(milliseconds(interval));
        
        if (find_timeout(id) == -1)
            return;
        
        call(&cp, "onInterval");
    }
}

void set_timeout(int timeout) {
    size_t id = timeoutv[timeoutv.size() - 1];
    
    this_thread::sleep_for(milliseconds(timeout));
    
    int pos = find_timeout(id);
    if (pos == -1)
        return;
    
    timeoutv.erase(timeoutv.begin() + pos);
    
    call(&cp, "onTimeout");
}

void signal_handler(int signum) {
    thread([signum]() {
        if (!call(&cp, "onExit", 1, (string[]) { to_string(signum) })) {
            middleware::socket_close();
            
            set_is_running(false);
            
            cp.kill();
            
            logger_write("^C\n");
        }
    }).detach();
}

int main(int argc, char* argv[]) {
    //  register signal callbacks
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    load_preferences();
    load(&cp);
    
    subscribe(offinterval, [&](const string* value) {
        int pos = find_timeout(stoi(value[0]));
        if (pos != -1)
            timeoutv.erase(timeoutv.begin() + pos);
    });
    
    subscribe(offtimeout, [&](const string* value) {
        int pos = find_timeout(stoi(value[0]));
        if (pos != -1)
            timeoutv.erase(timeoutv.begin() + pos);
    });
    
    subscribe(oninterval, [&](const string* value) {
        timeoutv.push_back(stoi(value[0]));
        
        thread(set_interval, stoi(value[1])).detach();
    });
    
    subscribe(ontimeout, [&](const string* value) {
        timeoutv.push_back(stoi(value[0]));
        
        thread(set_timeout, stoi(value[1])).detach();
    });
    
    //  build
    
    string filename = argc == 1 ? get_base_dir() + get_main() : decode_raw(raw(argv[1]));
    file* target = NULL;
    
    logger_write("Building...\n");
    
    time_point<steady_clock> beg = steady_clock::now();;
    
    node<string>* root = new node<string>(null(), NULL);
    target = new file(filename, root, &cp);
     
    root->close();
    
    double seconds = duration<double>(steady_clock::now() - beg).count();
    
    logger_write("Done in " + to_string(seconds) + " s.\n");
    
    //  run
    
    logger_write("Running...\n");
    
    beg = steady_clock::now();
    
    try {
        try {
            cp.stack_push(target);
            
            string _argv[max(0, argc - 2)];
            
            for (size_t i = 0; i < sizeof(_argv) / sizeof(_argv[0]); i += 1)
                _argv[i] = raw(argv[i + 2]);
            
            target->call(sizeof(_argv) / sizeof(_argv[0]), _argv);
            
        } catch (ss::exception& e) {
            ostringstream ss;
            
            ss << "EXCEPTION: " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    } catch (error& e) {
        if (is_running()) {
            ostringstream ss;
            
            ss << "ERROR: " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    }
    
    seconds = duration<double>(steady_clock::now() - beg).count();
  
    logger_write(to_string(seconds) + " s.\n");
    
    //  deinitialize
    
    target->close();
    
    unload();
}
