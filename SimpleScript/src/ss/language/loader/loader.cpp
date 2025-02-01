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
    atomic<size_t>                                                     timeoutc = 0;

    //  NON-MEMBER FUNCTIONS

    void send_message(const event_t type, const string* value) {
        loader_mutex.lock();
        
        for (size_t i = 0; i < callbackv.size(); ++i)
            if (std::get<1>(callbackv[i]) == type)
                std::get<2>(callbackv[i])(value);
        
        loader_mutex.unlock();
    }

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback) {
        loader_mutex.lock();
        
        callbackv.push_back({ callbackc, event, callback });
        
        size_t value = callbackc++;
        
        loader_mutex.unlock();
        
        return value;
    }

    void unsubscribe(const size_t subscription) {
        loader_mutex.lock();
        
        size_t i = 0;
        while (i < callbackv.size() && std::get<0>(callbackv[i]) != subscription)
            ++i;
        
        if (i != callbackv.size())
            callbackv.erase(callbackv.begin() + i);
        
        loader_mutex.unlock();
    }

    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // CONSTRUCTORS

    loader::loader(command_processor* cp, const bool flag) {
       this->_system_loader = new system_loader(cp);
        this->flag = flag;
        
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
            
            send_message(set_interval_t, (string[]){ std::to_string(timeoutc.load()), std::to_string(get_int(argv[0])) });
            
            size_t value = timeoutc.load();
            
            timeoutc.store(value + 1);
            
            return std::to_string(value);
        }));
        
        cp->set_function(new ss::function("setTimeout", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            send_message(set_timeout_t, (string[]){ std::to_string(timeoutc.load()), std::to_string(get_int(argv[0])) });
            
            size_t value = timeoutc.load();
            
            timeoutc.store(value + 1);
            
            return std::to_string(value);
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
        
        cp->set_function(new ss::function("worker", [&](const size_t argc, const string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            node<string>*      parent = new node<string>(null(), NULL);;
            file*              target = NULL;
            command_processor* cp = new command_processor();
            struct loader*     loader = new struct loader(cp);
            
            try {
                string filename = decode_raw(argv[0]);
                
                target = new file(filename, parent, cp, loader);
                
            } catch (error& e) {
                parent->close();
                
                delete loader;
                delete cp;
                throw e;
            }
            
            parent->close();
            
            size_t _argc = argc - 1;
            string* _argv = new string[_argc];
            
            for (size_t i = 0; i < _argc; ++i)
                _argv[i] = argv[i + 1];
            
            thread([&]() {
                string result;
                bool   flag = true;
                
                try {
                    // Segfault here
                    result = target->call(_argc, _argv);
                    
                } catch (error& e) {
                    flag = false;
                    
                    cout << e.what() << endl;
                }
                
                delete[] _argv;
                delete cp;
                
                target->close();
                
                if (flag) {
                    loader->call(cp, "onMessage", 1, (string[]){ result });
                }
                
                delete loader;
            }).detach();
            
            return null();
        }));
        
        cp->save_state();
    }

    loader::~loader() {
       // Begin Enhancement 1-1 - Thread safety - 2025-01-23
        
       if (this->_system_loader != NULL)
           delete this->_system_loader;
        
       if (this->_socket_loader != NULL)
           delete this->_socket_loader;
        
       if (this->_mysql_loader != NULL)
           delete this->_mysql_loader;
        
       if (this->_file_system_loader != NULL)
           delete this->_file_system_loader;
       // End Enhancement 1-1
        
       logger_close();
    }

    // MEMNBER FUNCTIONS

    bool loader::call(command_processor* cp, const string key, size_t argc, string* argv) {
        this->mutex.lock();
        
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
        
        this->mutex.unlock();
        
        return flag;
    }

    void loader::get_file_system() {
        this->_file_system_loader = new file_system_loader(this->flag);
    }

    void loader::bind_file_system(command_processor* cp) {
        this->_file_system_loader->bind(cp);
    }

    void loader::get_mysql() {
        this->_mysql_loader = new mysql_loader(this->flag);
    }

    void loader::bind_mysql(command_processor* cp) {
        this->_mysql_loader->bind(cp);
    }

    void loader::get_socket() {
        this->_socket_loader = new socket_loader(this->flag);
    }

    void loader::bind_socket(command_processor* cp) {
        this->_socket_loader->bind(cp);
    }
    // End Enhancement 1-1
}
