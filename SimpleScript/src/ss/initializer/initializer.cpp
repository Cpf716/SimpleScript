//
//  initializer.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#include "initializer.h"

namespace ss {
    //  NON-MEMBER FIELDS
    
    size_t workerc = 0;

    atomic<bool> _is_locked = false;

    //  NON-MEMBER FUNCTIONS

    string call(command_processor* cp, const string symbol, size_t argc, string* argv) {
        while (is_locked());
        
        lock();
        
        string result = std::to_string(1);
        
        if (cp->is_defined(symbol)) {
            cp->set_pause(true);
            
            this_thread::sleep_for(milliseconds(10));
        
            result = cp->call(symbol, argc, argv);
            
            cp->set_pause(false);
        }
        
        unlock();
        
        return result;
    }

    void deinitialize() {
        deinit_socket();
        deinit_mysql();
        deinit_file_system();
        
        logger_close();
    }

    void initialize(command_processor* cp) {
        init_system(cp);
        
        cp->set_function(new ss::function("worker", [cp](size_t argc, string* argv) {
            int id = (int)workerc++;
            
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            string filename = decode_raw(argv[0]);
            
            if (filename.empty())
                undefined_error(encode(empty()));
            
            for (size_t i = 0; i < argc - 1; ++i)
                swap(argv[i], argv[i + 1]);
            
            --argc;
            
            file* target = NULL;
            node<string>* root = NULL;
            
            command_processor* _cp = new command_processor();
            
            initialize(_cp);
            
            try {
                root = new node<string>(empty(), NULL);
                target = new file(filename, root, _cp);
                
            } catch (error& e) {
                root->close();
                
                throw e;
            }
            
            root->close();
            
            string* _argv = new string[argc];
            
            for (size_t i = 0; i < argc; ++i)
                _argv[i] = argv[i];
            
            thread([_cp, argc, _argv, cp, id, target]() {
                string result;
                bool success = true;
                
                try {
                    result = target->call(argc, _argv);
                    
                } catch (error& e) {
                    success = false;
                    
                    cout << e.what() << endl;
                }
                
                delete[] _argv;
                delete _cp;
                
                target->close();
                
                if (!success || !is_running())
                    return;
                
                string argv[2];
                
                argv[0] = std::to_string(id);
                argv[1] = result;
                
                call(cp, "onMessage", 2, argv);
            }).detach();
            
            return std::to_string(id);
        }));
        
        cp->save_state();
    }

    bool is_locked() {
        return _is_locked.load();
    }

    void lock() {
        _is_locked.store(true);
    }

    void unlock() {
        _is_locked.store(false);
    }
}
