//
//  loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#include "loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    size_t                                                             callbackc = 0;
    vector<tuple<size_t, event_t, std::function<void(const string*)>>> callbackv;
    mutex                                                              loader_mutex;
    size_t                                                             timeoutc = 0;
    size_t                                                             workerc = 0;

    //  NON-MEMBER FUNCTIONS

    bool call(command_processor* cp, const string key, size_t argc, string* argv) {
        loader_mutex.lock();
        
        bool flag = false;
        
        if (cp->is_defined(key)) {
            flag = true;
            
            cp->set_paused(true);
            
            // Begin Enhancement 1 - Thread safety - 2025-01-22
            while (!cp->is_paused());
            // End Enhancement 1
        
            cp->call(key, argc, argv);
            cp->set_paused(false);
        }
        
        loader_mutex.unlock();
        
        return flag;
    }

    void load(command_processor* cp) {
        load_system(cp);
        
        // array
        cp->set_function(new ss::function(to_string(array_t), [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc == 1) {
                int len = get_int(argv[0]);
                
                if (len < 1)
                    range_error(std::to_string(len));
                
                string buff[(size_t)len];
                
                return stringify(sizeof(buff) / sizeof(buff)[0], buff);
            }
            
            for (size_t i = 0; i < argc; ++i)
                if (ss::is_array(argv[i]))
                    type_error(array_t, item_t);
            
            return stringify(argc, argv);
        }));
        
        
        cp->set_function(new ss::function("clearInterval", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            send_message(clear_timeout_t, (string[]){ std::to_string(get_int(argv[0])) });
            
            return null();
        }));
        
        cp->set_function(new ss::function("clearTimeout", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            send_message(clear_timeout_t, (string[]){ std::to_string(get_int(argv[0])) });
            
            return null();
        }));
        
        cp->set_function(new ss::function("nrows", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            string destination[argv[0].length() + 1];
            
            return std::to_string((get_table(destination, argv[0]) - 1) / stoi(destination[0]));
        }));
        
        cp->set_function(new ss::function("errorMessage", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(strerror(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("setInterval", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            send_message(set_interval_t, (string[]){ std::to_string(timeoutc), std::to_string(get_int(argv[0])) });
            
            return std::to_string(timeoutc++);
        }));
        
        cp->set_function(new ss::function("setTimeout", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            send_message(set_timeout_t, (string[]){ std::to_string(timeoutc), std::to_string(get_int(argv[0])) });
            
            return std::to_string(timeoutc++);
        }));
        
        cp->set_function(new ss::function("sleep", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            this_thread::sleep_for(milliseconds((long)get_int(argv[0])));
            
            return null();
        }));
        
        cp->set_function(new ss::function("string", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                throw error("Unexpected token: " + argv[0]);
            
            if (argv[0].empty())
                null_error();
            
            if (is_string(argv[0]))
                return argv[0];
            
            return encode(argv[0]);
        }));
        
        cp->set_function(new ss::function("worker", [cp](const size_t argc, const string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            node<string>*      parent = new node<string>(null(), NULL);;
            file*              target = NULL;
            command_processor* _cp = new command_processor();
            
            load(_cp);
            
            try {
                string filename = decode_raw(argv[0]);
                
                target = new file(filename, parent, _cp);
                
            } catch (error& e) {
                parent->close();
                
                delete _cp;
                throw e;
            }
            
            parent->close();
            
            size_t _argc = argc - 1;
            string* _argv = new string[_argc];
            
            for (size_t i = 0; i < _argc; ++i)
                _argv[i] = argv[i + 1];
            
            thread([_cp, _argc, _argv, cp, target]() {
                string result;
                bool   flag = true;
                
                try {
                    result = target->call(_argc, _argv);
                    
                } catch (error& e) {
                    flag = false;
                    
                    cout << e.what() << endl;
                }
                
                delete[] _argv;
                delete _cp;
                
                target->close();
                
                if (flag)
                    call(cp, "onMessage", 1, (string[]){ result });
            }).detach();
            
            return null();
        }));
        
        cp->save_state();
    }

    void send_message(const event_t type, const string* value) {
        for (size_t i = 0; i < callbackv.size(); ++i)
            if (std::get<1>(callbackv[i]) == type)
                std::get<2>(callbackv[i])(value);
    }

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback) {
        callbackv.push_back({ callbackc, event, callback });
        
        return callbackc++;
    }

    void unload() {
        unload_socket();
        unload_mysql();
        unload_file_system();
        
        logger_close();
    }


    void unsubscribe(const size_t subscription) {
        size_t i = 0;
        while (i < callbackv.size() && std::get<0>(callbackv[i]) != subscription)
            ++i;
        
        if (i != callbackv.size())
            callbackv.erase(callbackv.begin() + i);
    }
}
