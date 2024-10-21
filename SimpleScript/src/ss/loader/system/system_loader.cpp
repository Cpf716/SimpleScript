//
//  system_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "system_loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    int stopwatchc = 0;
    vector<pair<int, time_point<steady_clock>>> stopwatchv;

    int find_stopwatch(const int key, const size_t beg = 0, const size_t end = stopwatchv.size()) {
        if (beg == end)
            return -1;
        
        size_t len = floor((end - beg) / 2);
        
        if (stopwatchv[beg + len].first == key)
            return (int)(beg + len);
        
        if (stopwatchv[beg + len].first > key)
            return find_stopwatch(key, beg, beg + len);
        
        return find_stopwatch(key, beg + len + 1, end);
    }

    //  NON-MEMBER FUNCTIONS

    void load_system(command_processor* cp) {
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
            
            int i = find_stopwatch((int)num);
            
            if (i == -1)
                return std::to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatchv[i].second).count();
            
            return encode(num);
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
        
        cp->set_function(new ss::function("start", [](const size_t argc, string* argv) {
            if (argc != 0)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            int num = stopwatchc++;
            
            stopwatchv.push_back(pair<int, time_point<steady_clock>>(num, steady_clock::now()));
            
            return std::to_string(num);
        }));
        
        cp->set_function(new ss::function("stop", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            int i = find_stopwatch((int)num);
            
            if (i == -1)
                return std::to_string(-1);
            
            num = duration<double>(steady_clock::now() - stopwatchv[i].second).count();
            
            stopwatchv.erase(stopwatchv.begin() + i);
            
            return encode(num);
        }));
    }
}
