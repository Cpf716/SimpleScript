//
//  loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/2/24.
//

#include "loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    atomic<bool> _is_locked = false;

    size_t callbackc = 0;
    vector<tuple<size_t, event_t, std::function<void(const string*)>>> callbackv;

    size_t timeoutc = 0;
    
    size_t workerc = 0;

    //  NON-MEMBER FUNCTIONS

    bool call(command_processor* cp, const string key, size_t argc, string* argv) {
        while (is_locked());
        
        lock();
        
        bool flag = false;
        
        if (cp->is_defined(key)) {
            flag = true;
            
            cp->set_pause(true);
            
            this_thread::sleep_for(milliseconds(10));
        
            cp->call(key, argc, argv);
            cp->set_pause(false);
        }
        
        unlock();
        
        return flag;
    }

    void load(command_processor* cp) {
        load_system(cp);
        
        // array
        cp->set_function(new ss::function(to_string(array_t), [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc == 1) {
                if (ss::is_array(argv[0]))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (argv[0].empty() || is_string(argv[0]))
                    type_error(string_t, int_t);
                    //  string != int
                
                double num = stod(argv[0]);
                
                if (!is_int(num))
                    type_error(double_t, int_t);
                    //  double != int
                
                if (num < 1)
                    range_error(std::to_string((int)num));
                
                string arr[(size_t)num];
                
                return stringify(sizeof(arr) / sizeof(arr)[0], arr);
            }
            
            for (size_t i = 0; i < argc; ++i)
                if (ss::is_array(argv[i]))
                    type_error(array_t, string_t);
            
            return stringify(argc, argv);
        }));
        
        
        cp->set_function(new ss::function("cancelInterval", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            string id[] { std::to_string(num) };
            
            broadcast(offinterval, id);
            
            return encode(to_string(undefined_t));
        }));
        
        cp->set_function(new ss::function("cancelTimeout", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            string id[] { std::to_string(num) };
            
            broadcast(offtimeout, id);
            
            return encode(to_string(undefined_t));
        }));
        
        cp->set_function(new ss::function("setInterval", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            size_t id = timeoutc++;
            
            string interval[] { std::to_string(id), std::to_string(num) };
            
            broadcast(oninterval, interval);
            
            return std::to_string(id);
        }));
        
        cp->set_function(new ss::function("setTimeout", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            size_t id = timeoutc++;
            
            string timeout[] { std::to_string(id), std::to_string(num) };
            
            broadcast(ontimeout, timeout);
            
            return std::to_string(id);
        }));
        
        cp->set_function(new ss::function("setTimeoutSync", [cp](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            this_thread::sleep_for(milliseconds((long)num));
            
            return encode(to_string(undefined_t));
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
        
        cp->set_function(new ss::function("worker", [cp](size_t argc, string* argv) {
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
                undefined_error(encode(null()));
            
            for (size_t i = 0; i < argc - 1; ++i)
                swap(argv[i], argv[i + 1]);
            
            --argc;
            
            file* target = NULL;
            node<string>* root = NULL;
            
            command_processor* _cp = new command_processor();
            
            load(_cp);
            
            try {
                root = new node<string>(null(), NULL);
                target = new file(filename, root, _cp);
                
            } catch (error& e) {
                root->close();
                
                throw e;
            }
            
            root->close();
            
            string* _argv = new string[argc];
            
            for (size_t i = 0; i < argc; ++i)
                _argv[i] = argv[i];
            
            thread([_cp, argc, _argv, cp, target]() {
                string value;
                bool flag = true;
                
                try {
                    value = target->call(argc, _argv);
                    
                } catch (error& e) {
                    flag = false;
                    
                    cout << e.what() << endl;
                }
                
                delete[] _argv;
                delete _cp;
                
                target->close();
                
                if (!flag || !is_running())
                    return;
                
                string message[] { value };
                
                call(cp, "onMessage", 1, message);
            }).detach();
            
            return encode(to_string(undefined_t));
        }));
        
        cp->save_state();
    }

    bool is_locked() {
        return _is_locked.load();
    }

    void lock() {
        _is_locked.store(true);
    }

    size_t subscribe(const event_t event, const std::function<void(const string*)> callback) {
        callbackv.push_back(tuple<size_t, event_t, std::function<void(const string*)>>(callbackc, event, callback));
        
        return callbackc++;
    }

    void broadcast(const event_t event, const string* value) {
        for (size_t i = 0; i < callbackv.size(); ++i)
            if (std::get<1>(callbackv[i]) == event)
                std::get<2>(callbackv[i])(value);
    }

    void unlock() {
        _is_locked.store(false);
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
