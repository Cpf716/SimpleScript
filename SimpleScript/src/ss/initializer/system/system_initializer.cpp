//
//  system_initializer.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "system_initializer.h"

namespace ss {
    //  NON-MEMBER FIELDS

    size_t callbackc = 0;
    vector<tuple<size_t, event_t, std::function<void(void)>>> callbackv;

    int stopwatc = 0;
    vector<pair<int, time_point<steady_clock>>> stopwatv;

    int _find_stopwat(const int key, const size_t beg, const size_t end) {
        if (beg == end)
            return -1;
        
        size_t len = floor((end - beg) / 2);
        
        if (stopwatv[beg + len].first == key)
            return (int)(beg + len);
        
        if (stopwatv[beg + len].first > key)
            return _find_stopwat(key, beg, beg + len);
        
        return _find_stopwat(key, beg + len + 1, end);
    }

    int _find_stopwat(const int key) {
        return _find_stopwat(key, 0, stopwatv.size());
    }

    //  NON-MEMBER FUNCTIONS

    size_t add_listener(const event_t event, const std::function<void(void)> cb) {
        callbackv.push_back(tuple<size_t, event_t, std::function<void(void)>>(callbackc, event, cb));
        
        return callbackc++;
    }

    void dispatch(const event_t event) {
        for (size_t i = 0; i < callbackv.size(); ++i)
            if (std::get<1>(callbackv[i]) == event)
                std::get<2>(callbackv[i])();
    }

    void init_system(command_processor* cp) {
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
        
        cp->set_function(new ss::function("cmd", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
                
            string cmd = decode_raw(argv[0]);
            string abspath = "/tmp/" + uuid();
            
            cmd = cmd + " > " + abspath;
            
            system(cmd.c_str());
            
            string result = encode(read(abspath));
        
            remove(abspath.c_str());
            
            return result;
        }));
        
        cp->set_function(new ss::function("gmt", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            time_t now = time(0);
            
            tm *gmtm = gmtime(&now);
            char* dt = asctime(gmtm);
            
            return encode(string(dt));
        }));
        
        cp->set_function(new ss::function("input", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            string str;
            getline(cin, str);
            
            return raw(str);
        }));
        
        cp->set_function(new ss::function("lap", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            int i = _find_stopwat((int)num);
            
            if (i == -1)
                return std::to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatv[i].second).count();
            
            return trim_end(num);
        }));
        
        cp->set_function(new ss::function("local", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            time_t now = time(0);
            char* dt = ctime(&now);
            
            return encode(string(dt));
        }));
        
        cp->set_function(new ss::function("rand", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            random_device rd;
            mt19937_64 gen(rd());
            uniform_int_distribution<> uid;
            
            return std::to_string(uid(gen));
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
            
            cp->set_read_only("interval", false);
            cp->set_number("interval", num);
            cp->set_read_only("interval", true);
            
            dispatch(oninterval_t);
            
            return encode(to_string(undefined_t));
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
            
            cp->set_read_only("timeout", false);
            cp->set_number("timeout", num);
            cp->set_read_only("timeout", true);
            
            dispatch(ontimeout_t);
            
            return encode(to_string(undefined_t));
        }));
        
        cp->set_function(new ss::function("start", [](const size_t argc, string* argv) {
            if (argc != 0)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            int num = stopwatc++;
            
            stopwatv.push_back(pair<int, time_point<steady_clock>>(num, steady_clock::now()));
            
            return std::to_string(num);
        }));
        
        cp->set_function(new ss::function("stop", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            int i = _find_stopwat((int)num);
            
            if (i == -1)
                return std::to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatv[i].second).count();
            
            stopwatv.erase(stopwatv.begin() + i);
            
            return trim_end(num);
        }));
    }

    void remove_listener(const size_t listener) {
        size_t i = 0;
        while (i < callbackv.size() && std::get<0>(callbackv[i]) != listener)
            ++i;
        
        if (i != callbackv.size())
            callbackv.erase(callbackv.begin() + i);
    }
}
