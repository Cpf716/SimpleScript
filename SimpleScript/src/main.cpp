//
//  main.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#include "initializer.h"

using namespace ss;

//  NON-MEMBER FIELDS

command_processor cp;

vector<int> timeoutv;

int find_timeout(const int key, const size_t beg = 0, const size_t end = timeoutv.size()) {
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
    int id = timeoutv[timeoutv.size() - 1];
    
    while (true) {
        this_thread::sleep_for(seconds(interval));
        
        int index = find_timeout(id);
        if (index == -1)
            return;
        
        call(&cp, "onInterval");
    }
}

void set_timeout(int timeout) {
    int id = timeoutv[timeoutv.size() - 1];
    
    this_thread::sleep_for(seconds(timeout));
    
    int index = find_timeout(id);
    if (index == -1)
        return;
    
    timeoutv.erase(timeoutv.begin() + index);
    
    call(&cp, "onTimeout");
}

void signal_handler(int signum) {
    thread([signum]() {
        string _signum[] { to_string(signum) };
        
        if (evaluate(call(&cp, "onExit", 1, _signum))) {
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
    
    init_preferences();
    initialize(&cp);
    
    subscribe(offinterval, [&](const string* value) {
        int index = find_timeout(stoi(value[0]));
        if (index != -1)
            timeoutv.erase(timeoutv.begin() + index);
    });
    
    subscribe(offtimeout, [&](const string* value) {
        int index = find_timeout(stoi(value[0]));
        if (index != -1)
            timeoutv.erase(timeoutv.begin() + index);
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
            
            string result = target->call(sizeof(_argv) / sizeof(_argv[0]), _argv);
            
//            if (is_running()) {
//                string valuev[result.length() + 1];
//                size_t valuec = parse(valuev, result);
//                
//                ostringstream ss;
//                
//                for (size_t i = 0; i < valuec - 1; ++i)
//                    ss << (valuev[i].empty() ? "null" : decode_raw(valuev[i])) << "\t";
//                
//                ss << (valuev[valuec - 1].empty() ? "null" : decode_raw(valuev[valuec - 1]));
//                
//                if (!ss.str().empty())
//                    cout << "  " << ss.str() << endl;
//            }
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
    
    int hours = 0;
    while (seconds >= (hours + 1) * 3600)
        ++hours;
    
    seconds -= hours * 3600;
    
    int minutes = 0;
    while (seconds >= (minutes + 1) * 60)
        ++minutes;
    
    seconds -= minutes * 60;
    
    logger_write("Done in ");
    
    if (hours)
        logger_write(to_string(hours) + " h, ");
    
    if (minutes)
        logger_write(to_string(minutes) + " m, ");
    
    logger_write(to_string(seconds) + " s.\n");
    
    //  deinitialize
    
    target->close();
    
    deinitialize();
}
