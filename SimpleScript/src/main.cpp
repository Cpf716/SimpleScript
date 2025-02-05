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
//bool              return_flag = false;

//  NON-MEMBER FUNCTIONS

void handle_signal(int signum) {
    // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
    broadcast(message_default);
    // End Enhancement 1-1
    
    thread([signum]() {
        if (call(&cp, "onExit", 1, (string[]) { to_string(signum) }))
            return;
        
        ss::integration::socket_close();
        
//        return_flag = true;
        
        cp.kill();
        
        logger_write("^C\n");
    }).detach();
}

int main(int argc, char* argv[]) {
    //  register signal callbacks
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    logger_open(logs());
    load(&cp);
    
    //  build
    
    logger_write("Building...\n");
    
    time_point<steady_clock> beg = steady_clock::now();;
    node<string>*            parent = new node<string>();
    file*                    target = NULL;
    
    try {
        string filename = argc == 1 ? path(library(), main()) : decode_raw(raw(argv[1]));
        
        target = new file(filename, parent, &cp);
    } catch (error& e) {
        parent->close();
        
        throw e;
    }
    
    parent->close();
    target->subscribe(event_kill, []() {
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
            
            ss << "EXCEPTION - " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    } catch (error& e) {
//        if (!return_flag) {
            ostringstream ss;
            
            ss << "ERROR - " << e.what() << endl;
            ss << cp.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
//        }
    }
    
    res = duration<double>(steady_clock::now() - beg).count();
  
    logger_write("Done in " + to_string(res) + " s.\n");
    
    target->close();
    
    unload();
}
