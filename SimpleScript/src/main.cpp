//
//  main.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 4/18/23.
//

#include "file.h"

using namespace ss;

//  NON-MEMBER FIELDS

bool alive = true, ready = false;

size_t listenerc = 0;

interpreter ssu;

//  NON-MEMBER FUNCTIONS

string format(const string value) {
    string valuev[value.length() + 1];
    size_t valuec = parse(valuev, value);
    
    ostringstream ss;
    
    for (size_t i = 0; i < valuec - 1; ++i)
        ss << (valuev[i].empty() ? "null" : decode(valuev[i])) << "\t";
    
    ss << (valuev[valuec - 1].empty() ? "null" : decode(valuev[valuec - 1]));
    
    return ss.str();
}

void set_timeout() {
    size_t lis = listenerc++;
    
    double num = ssu.get_number("timeout");
    
    if (!num)
        return;
    
    this_thread::sleep_for(seconds((int)num));
    
    if (lis == listenerc - 1) {
        alive = false;
        
        ssu.kill();
        
        while (!ready);
        
        string timemes = decode(ssu.get_string("timeoutMessage"));
        
        if (!timemes.empty())
            cout << timemes << endl;
    }
}

void signal_handler(int signum) {
    if (ssu.get_number("askBeforeExit")) {
        cout << "\nAre you sure? (Y/n)\t";
        
        string str;
        getline(cin, str);
        
        if (toupper(trim(str)) != "Y")
            return;
    }
    
    api::mysql_close();
    api::socket_close();
    
    logger_write("^C\n");
    logger_close();
    
    exit(signum);
}

void write_out(const string value) {
    if (!value.empty())
        cout << "  " << value << endl;
}

int main(int argc, char* argv[]) {
    //  register signal callbacks
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    ssu.set_listener("timeout", [&](const string val) {
        thread(set_timeout).detach();
    });
    
    string filename;
    
    if (argc == 1)
        filename = base_dir() + "main.txt";
    
    else if (tolower(string(argv[1])) == "cli") {
        cout << "Building...\n";
        
        logger_write("Building...\n");
        
        time_point<steady_clock> beg = steady_clock::now();;
        
        ifstream file(base_dir() + "lib.txt");
        
        while (getline(file, filename)) {
            node<string>* root = new node<string>(empty(), NULL);
            class file* target = new class file(filename, root, &ssu);
            
            root->close();
            
            ssu.set_function(target);
        }
        
        file.close();
        
        double dif = duration<double>(steady_clock::now() - beg).count();
        
        cout << "Done in " << dif << " s.\n";
        
        logger_write("Done in " + to_string(dif) + " s.\n");
        
        cout << "Running...\n";
        
        logger_write("Running...\n");
        
        bool flag = false;
        string str;
        
        while (1) {
            if (!alive)
                break;
            
            if (!flag)
                cout << "> ";
            
            getline(cin, str);
            
            str = trim(str);
            
            if ((flag = str.empty()))
                continue;
            
            logger_write("> " + str + "\n");
            
            try {
                try {
                    str = format(ssu.evaluate(str));
                    
                    if (alive) {
                        write_out(str);
                        logger_write("  " + str + "\n");
                    }
                } catch (ss::exception& e) {
                    throw error(e.what());
                }
            } catch (error& e) {
                if (alive) {
                    cout << "  ERROR: " << e.what() << endl;
                    
                    logger_write("  ERROR: " + string(e.what()) + "\n");
                }
            }
        }
        
        ready = true;
        
        logger_close();
        
        return 0;
    } else
        filename = decode(raw(argv[1]));
    
    logger_write("Building...\n");
    
    time_point<steady_clock> beg = steady_clock::now();;
    
    //  initialize filepath tree
    //  prevents recursive file inclusion
    node<string>* root = new node<string>(empty(), NULL);
    file* target = new file(filename, root, &ssu);
     
    root->close();
    
    ssu.set_function(target);
    
    time_point<steady_clock> end = steady_clock::now();
        
    logger_write("Done in " + to_string(duration<double>(end - beg).count()) + " s.\n");
    logger_write("Running...\n");
    
    string _argv[max(0, argc - 2)];
    
    for (size_t i = 0; i < sizeof(_argv) / sizeof(_argv[0]); i += 1)
        _argv[i] = raw(argv[i + 2]);
    
    beg = steady_clock::now();
    
    try {
        try {
            ssu.stack_push(target);
            
            string result = format(target->call(sizeof(_argv) / sizeof(_argv[0]), _argv));
            
            if (alive)
                write_out(result);
            
        } catch (ss::exception& e) {
            ostringstream ss;
            
            ss << "EXCEPTION : " << e.what() << endl;
            ss << ssu.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    } catch (error& e) {
        if (alive) {
            ostringstream ss;
            
            ss << "ERROR : " << e.what() << endl;
            ss << ssu.stack_trace();
            
            cout << ss.str();
            
            logger_write(ss.str());
        }
    }
    
    ready = true;
    
    end = steady_clock::now();
    
    logger_write("Done in " + to_string(duration<double>(end - beg).count()) + " s.\n");
    logger_close();
}
