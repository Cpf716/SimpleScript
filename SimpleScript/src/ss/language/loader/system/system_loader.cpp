//
//  system_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "system_loader.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // CONSTRUCTORS

    system_loader::system_loader(command_processor* cp) {
        /*
        cp->set_function(new ss::function("cmd", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
                
            string cmd = decode_raw(get_string(argv[0]));
            string file_path = "/tmp/" + uuid();
            
            cmd += " > " + file_path;
            
            system(cmd.c_str());
            
            string res = encode(read(file_path));
        
            remove(file_path.c_str());
            
            return res;
        }));
        
        cp->set_function(new ss::function("gmt", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            time_t now = time(0);
            tm     *gmtm = gmtime(&now);
            char*  dt = asctime(gmtm);
            
            return encode(string(dt));
        }));
        
        cp->set_function(new ss::function("input", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            string item;
            getline(cin, item);
            
            return raw(item);
        }));
        
        cp->set_function(new ss::function("lap", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            int pos = this->find_stopwatch(get_int(argv[0]));
            
            if (pos == -1)
                return std::to_string(-1);
            
            double res = duration<double>(steady_clock::now() - this->stopwatchv[pos].second).count();
            
            return encode(res);
        }));
        
        cp->set_function(new ss::function("local", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            time_t now = time(0);
            char*  dt = ctime(&now);
            
            return encode(string(dt));
        }));
        
        cp->set_function(new ss::function("rand", [](const size_t argc, string* argv) {
            if (argc)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            random_device              rd;
            mt19937_64                 gen(rd());
            uniform_int_distribution<> uid;
            
            return std::to_string(uid(gen));
        }));
        
        cp->set_function(new ss::function("start", [this](const size_t argc, string* argv) {
            if (argc != 0)
                expect_error("0 argument(s), got " + std::to_string(argc));
            
            int pos = this->stopwatchc++;
            
            this->stopwatchv.push_back({ pos, steady_clock::now() });
            
            return std::to_string(pos);
        }));
        
        cp->set_function(new ss::function("stop", [this](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            int pos = this->find_stopwatch(get_int(argv[0]));
            
            if (pos == -1)
                return std::to_string(-1);
            
            double res = duration<double>(steady_clock::now() - this->stopwatchv[pos].second).count();
            
            this->stopwatchv.erase(this->stopwatchv.begin() + pos);
            
            return encode(res);
        }));
         */
    }

    // MEMBER FUNCTIONS

    int system_loader::find_stopwatch(const int key) {
        return this->find_stopwatch(key, 0, this->stopwatchv.size());
    }

    int system_loader::find_stopwatch(const int key, const size_t begin, const size_t end) {
        if (begin == end)
            return -1;
        
        size_t len = floor((end - begin) / 2);
        
        if (stopwatchv[begin + len].first == key)
            return (int)(begin + len);
        
        if (stopwatchv[begin + len].first > key)
            return find_stopwatch(key, begin, begin + len);
        
        return find_stopwatch(key, begin + len + 1, end);
    }
    // End Enhancement 1-1
}
