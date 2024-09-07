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

size_t intervalc = 0, timeoutc = 0;

//  NON-MEMBER FUNCTIONS

void set_interval() {
    double num = cp.get_number("interval");
    
    size_t lis = intervalc++;
    
    if (!num)
        return;
    
    while (true) {
        this_thread::sleep_for(seconds((int)num));
        
        if (lis != intervalc - 1)
            break;
        
        call(&cp, "onInterval");
    }
}

void set_timeout() {
    double num = cp.get_number("timeout");
    
    size_t lis = timeoutc++;
    
    if (!num)
        return;
    
    this_thread::sleep_for(seconds((int)num));
    
    if (lis == timeoutc - 1 && evaluate(call(&cp, "onTimeout"))) {
        middleware::socket_close();
        
        set_is_running(false);
        
        cp.kill();
    }
}

void signal_handler(int signum) {
    thread([signum]() {
        string argv[1];
        
        argv[0] = to_string(signum);
        
        if (evaluate(call(&cp, "onExit", 1, argv))) {
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
    
    add_listener(oninterval_t, [&]() {
        thread(set_interval).detach();
    });
    
    add_listener(ontimeout_t, [&]() {
        thread(set_timeout).detach();
    });
    
    //  build
    
    string filename = argc == 1 ? get_base_dir() + get_main() : decode_raw(raw(argv[1]));
    file* target = NULL;
    
    logger_write("Building...\n");
    
    time_point<steady_clock> beg = steady_clock::now();;
    
    node<string>* root = new node<string>(empty(), NULL);
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
            
            if (is_running()) {
                string valuev[result.length() + 1];
                size_t valuec = parse(valuev, result);
                
                ostringstream ss;
                
                for (size_t i = 0; i < valuec - 1; ++i)
                    ss << (valuev[i].empty() ? "null" : decode_raw(valuev[i])) << "\t";
                
                ss << (valuev[valuec - 1].empty() ? "null" : decode_raw(valuev[valuec - 1]));
                
                if (!ss.str().empty())
                    cout << "  " << ss.str() << endl;
            }
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
