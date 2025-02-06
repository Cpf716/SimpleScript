//
//  loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#include "loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
    mutex          context_mutex;
    mutex          timeout_mutex;
    // End Enhancement 1-1
    size_t         timeoutc = 0;
    vector<size_t> timeoutv;
    

    // NON-MEMBER FUNCTIONS
    // PRIVATE

    int find_timeout(const size_t key, const size_t begin = 0, const size_t end = timeoutv.size()) {
        if (begin == end)
            return -1;
        
        size_t len = floor((end - begin) / 2);
        
        if (timeoutv[begin + len] == key)
            return (int)(begin + len);
        
        if (timeoutv[begin + len] > key)
            return find_timeout(key, begin, begin + len);
        
        return find_timeout(key, begin + len + 1, end);
    }

    void clear_timeout(size_t timeout) {
        timeout_mutex.lock();
        
        int pos = find_timeout(timeout);
        
        if (pos != -1)
            timeoutv.erase(timeoutv.begin() + pos);
        
        timeout_mutex.unlock();
    }

    // PUBLIC


    bool call(command_processor* cp, const string key, size_t argc, string* argv) {
        context_mutex.lock();
        
        // Begin Enhancement 1-1 - Thread Safety - 2025-02-01
        bool flag = cp->is_defined(key);
        
        if (flag) {
            cp->set_paused(true, [key, argc, argv](command_processor* cp) {
                cp->call(key, argc, argv);
            });
        }
        // End Enhancement 1-1
        
        context_mutex.unlock();
        
        return flag;
    }

    void load(command_processor* cp) {
        load_system(cp);
        load_file_system(cp);
        load_mysql(cp);
        load_socket(cp);
        
        try {
            string filev[1024];
            size_t filec = read(filev, environment() + "/environment", separator());
            
            cp->set_array("env", [filec, &filev](variable<ss::array<string>>* value) {
                value->set_value(ss::array(filec, filev));
                // value->readonly() = true;
            });
        } catch (file_system_exception& e) { }
        
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
            
            clear_timeout(get_int(argv[0]));
            
            return null();
        }));
        
        cp->set_function(new ss::function("clearTimeout", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            clear_timeout(get_int(argv[0]));
            
            return null();
        }));
        
        cp->set_function(new ss::function("ncols", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            string destination[argv[0].length() + 1];
            
            get_table(destination, argv[0]);
            
            return destination[0];
        }));
        
        cp->set_function(new ss::function("nrows", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            string destination[argv[0].length() + 1];
            
            return std::to_string((get_table(destination, argv[0]) - 1) / stoi(destination[0]));
        }));
        
        cp->set_function(new ss::function("error", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(strerror(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("setInterval", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            // Begin Enhancement 1-1 - Thread Safety - 2025-02-01
            timeout_mutex.lock();
            
            size_t interval = timeoutc++;
            
            timeoutv.push_back(interval);
            
            timeout_mutex.unlock();
            
            thread([interval, cp](const int ms) {
                while (true) {
                    this_thread::sleep_for(milliseconds(ms));
                    
                    timeout_mutex.lock();
                    
                    int pos = find_timeout(interval);
                    
                    timeout_mutex.unlock();
                    
                    if (pos == -1)
                        return;
                    
                    call(cp, "onInterval");
                }
            }, get_int(argv[0])).detach();
            
            return std::to_string(interval);
            // End Enhancement 1-1
        }));
        
        cp->set_function(new ss::function("setTimeout", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            // Begin Enhancement 1-1 - Thread Safety - 2025-02-01
            timeout_mutex.lock();
            
            size_t timeout = timeoutc++;
            
            timeoutv.push_back(timeout);
            
            timeout_mutex.unlock();
            
            thread([timeout, cp](int ms) {
                this_thread::sleep_for(milliseconds(ms));
                
                timeout_mutex.lock();
                
                int pos = find_timeout(timeout);
                
                if (pos == -1) {
                    timeout_mutex.unlock();
                    
                    return;
                }
                
                timeoutv.erase(timeoutv.begin() + pos);
                
                timeout_mutex.unlock();
                
                call(cp, "onTimeout");
            }, get_int(argv[0])).detach();
            
            return std::to_string(timeout);
            // End Enhancement 1-1
        }));
        
        cp->set_function(new ss::function("sleep", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
            atomic<bool> flag = false;
            
            size_t number = subscribe(message_default, [&flag](const size_t valuec, const string* valuev) {
                flag.store(true);
            });
            
            thread([&flag](long ms) {
                this_thread::sleep_for(milliseconds(ms));
                
                flag.store(true);
            }, (long)get_int(argv[0])).detach();
            
            while (!flag.load())
                continue;
            
            unsubscribe(number);
            // End Enhancement 1-1
            
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
                    // Begin Enhancement 1-1 - Thread Safety - 2025-02-04
                    _cp->stack_push(target);
                    // End Enhancement 1-1
                    
                    result = target->call(_argc, _argv);
                    
                } catch (error& e) {
                    flag = false;
                    
//                    cout << e.what() << endl;
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

    void unload() {
        unload_socket();
        unload_mysql();
        unload_file_system();

        logger_close();
    }
}
