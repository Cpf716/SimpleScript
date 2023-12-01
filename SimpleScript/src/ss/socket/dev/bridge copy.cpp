//
//  bridge.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/17/23.
//

#include <arpa/inet.h>      //  inet_ptons
#include <csignal>          //  signal
#include <iostream>         //  cin, cout
#include <netinet/in.h>     //  sockaddr_in
#include <sstream>          //  ostringstream
#include <sys/socket.h>     //  socket
#include <thread>
#include <unistd.h>         //  close, read

using namespace std;

//  TYPEDEF

struct socket {
    //  MEMBER FIELDS
    
    vector<struct sockaddr_in> add;

    int addlen;
    
    vector<int> parval;
    
    int fildes;
    
    vector<bool> fla;
        
    vector<thread> thr;
    
    vector<int> val;
    
    //  CONSTRUCTORS
    
    socket(const int fildes) {
        val.push_back(fildes);
        fla.push_back(true);
    }
    
    //  MEMBER FUNCTIONS
    
    bool is_client() {
        return (!add.size() && !parval.size()) || (add.size() == 1 && parval.size());
    }
    
    bool is_parallel() { return !!parval.size(); }
    
    bool is_server() {
        return (add.size() == 1 && !parval.size()) || (add.size() > 1 && parval.size());
    }
    
    void set_address(const struct sockaddr_in add, const int addlen) {
        this->add.push_back(add);
        this->addlen = addlen;
        this->fla[0] = false;
    }
};

//  NON-MEMBER FIELDS

vector<struct socket*> thr;

vector<struct socket*> val;

//  NON-MEMBER FUNCTIONS

vector<int> accept(const int fildes) {
    size_t i;
    for (i = 0; i < val.size(); ++i) {
        if (val[i]->val[0] == fildes) {
            if (val[i]->is_client())
                return vector<int>();
            
            break;
        }
        
        if (val[i]->is_server()) {
            size_t j = 1;
            while (j < val[i]->val.size() && val[i]->val[j] != fildes)
                ++j;
            
            if (j != val[i]->val.size())
                return vector<int>();
        }
        
        size_t j = 0;
        while (j < val[i]->parval.size() && val[i]->parval[j] != fildes)
            ++j;
        
        if (j != val[i]->parval.size())
            return vector<int>();
    }
    
    if (i == val.size())
        return vector<int>();
    
    struct socket* sock = val[i];
    
    i = 1;
    while (i < sock->val.size()) {
        if (send(sock->val[i], string("\n").c_str(), 1, MSG_NOSIGNAL) <= 0)
            sock->val.erase(sock->val.begin() + i);
        else
            ++i;
    }
    
    return vector<int>(sock->val.begin() + 1, sock->val.end());
}

void accept_handler_parallel() {
    size_t thrnum = thr.size() - 1;
    
    while (1) {
        struct socket* sock = thr[thrnum];
        
        if (sock->fla[sock->fla.size() - 1])
            break;
        
        //  returns nonnegative file descriptor or -1 for error
        int fildes = accept(sock->parval[0], (struct sockaddr *)&sock->add[sock->add.size() - 1], (socklen_t *)&sock->addlen);
        
        if (fildes == -1)
            continue;
        
        sock->parval.push_back(fildes);
    }
    
    //  cout << "parallel accept joining...\n";
}

void accept_handler_server() {
    size_t thrnum = thr.size() - 1;
    
    while (1) {
        struct socket* sock = thr[thrnum];
        
        if (sock->fla[0])
            break;
        
        //  returns nonnegative file descriptor or -1 for error
        int fildes = accept(sock->val[0], (struct sockaddr *)&sock->add[0], (socklen_t *)&sock->addlen);
        
        if (fildes == -1)
            continue;
        
        sock->val.push_back(fildes);
    }
    
    //  cout << "server accept joining...\n";
}

int client(const char* src, const int port) {
    int fildes;
    
    while (1) {
        fildes = socket(AF_INET, SOCK_STREAM, 0);
        
        struct sockaddr_in add;

        add.sin_family = AF_INET;
        add.sin_port = htons(port);
        
        //  localhost
        inet_pton(AF_INET, src, &add.sin_addr);
        
        //  returns 0 for success, -1 otherwise
        if (!connect(fildes, (struct sockaddr *)&add, sizeof(add)))
            break;
            
        close(fildes);
        
        this_thread::sleep_for(chrono::seconds(1));
    }
    
    val.push_back(new struct socket(fildes));
    
    return fildes;
}

int _close(const int fildes) {
    for (size_t i = 0; i < val.size(); ++i) {
        struct socket* sock = val[i];
        
        if (sock->val[0] == fildes) {
            if (sock->is_parallel()) {
                sock->fla[sock->fla.size() - 1] = true;
                
                shutdown(sock->parval[0], SHUT_RDWR);
                
                for (size_t j = 0; j < sock->parval.size(); ++j)
                    close(sock->parval[j]);
                
                for (size_t j = sock->is_server() ? 1 : 0; j < sock->thr.size(); ++j)
                    if (sock->thr[j].joinable())
                        sock->thr[j].join();
            }
            
            if (sock->is_client())
                close(sock->val[0]);
            else {
                sock->fla[0] = true;
                
                shutdown(sock->val[0], SHUT_RDWR);
                
                for (size_t j = 0; j < sock->val.size(); ++j)
                    close(sock->val[j]);
                
                if (sock->thr[0].joinable())
                    sock->thr[0].join();
            }
            
            delete sock;
            
            val.erase(val.begin() + i);
            
            return 0;
        }
        
        if (sock->is_server()) {
            size_t j = 1;
            while (j < sock->val.size() && sock->val[j] != fildes)
                ++j;
            
            if (j != sock->val.size()) {
                close(sock->val[j]);
                
                sock->val.erase(sock->val.begin() + j);
                
                return 0;
            }
        }
        
        size_t j = 0;
        while (j < sock->parval.size() && sock->parval[j] != fildes)
            ++j;
        
        if (j != sock->parval.size()) {
            if (j == 0) {
                sock->fla[sock->fla.size() - 1] = true;
                
                shutdown(sock->parval[0], SHUT_RDWR);
                
                for (size_t j = 0; j < sock->parval.size(); ++j)
                    close(sock->parval[j]);
                
                for (size_t j = sock->is_server() ? 1 : 0; j < sock->thr.size(); ++j)
                    if (sock->thr[j].joinable())
                        sock->thr[j].join();
                
                sock->parval.clear();
                sock->add.erase(sock->add.end() - 1);
                
                return 0;
            }
            
            return -1;
        }
    }
    
    return -1;
}

int input() {
    while (1) {
        string str;
        
        getline(cin, str);
        
        try {
            return stoi(str);
            
        } catch (invalid_argument& e) { }
    }
}

size_t split(string* dst, const string src, const string pattern) {
    size_t s = 0, n = 0;
    for (int e = 0; e <= (int)src.length() - (int)pattern.length(); ++e) {
        size_t i = 0;
        while (i < pattern.length() && src[e + i] == pattern[i])
            ++i;
        
        if (i == pattern.length()) {
            dst[n++] = src.substr(s, e - s);
            
            s = e + i;
        }
    }
    
    dst[n++] = src.substr(s);

    return n;
}

void read_handler() {
    size_t thrnum = thr.size() - 1;
        
    while (1) {
        struct socket* sock = thr[thrnum];
        
        if (sock->fla[sock->fla.size() - 1])
            break;
        
        char val[1024] = {0};
        
        //  returns message length (bytes), 0 for closed connection, or -1 for error
        if (read(sock->fildes, val, 1024) <= 0)
            continue;
        
        string valv[strlen(val) + 1];
        size_t valc = split(valv, string(val), "\n");
        
        size_t i = 0;
        
        while (i < valc) {
            if (valv[i].empty()) {
                for (size_t j = i; j < valc - 1; ++j)
                    swap(valv[j], valv[j + 1]);
                
                --valc;
            } else
                ++i;
        }
        
        stringstream ss;
        
        for (i = 0; i < valc; ++i)
            ss << valv[i] << "\n";
        
        string _val = ss.str();
        
        if (_val.empty())
            continue;
                
        i = 1;
        while (i < sock->parval.size()) {
            if (send(sock->parval[i], _val.c_str(), _val.length(), MSG_NOSIGNAL) <= 0) {
                sock->parval.erase(sock->parval.begin() + i);
            } else
                ++i;
        }
    }
    
    //  cout << "read joining...\n";
}

int _listen(const int fildes, const int port) {
    size_t i;
    for (i = 0; i < val.size(); ++i) {
        if (val[i]->val[0] == fildes) {
            if (val[i]->is_server() || val[i]->is_parallel())
                return -1;
            
            break;
        }
        
        if (val[i]->is_server()) {
            size_t j = 1;
            while (j < val[i]->val.size() && val[i]->val[j] != fildes)
                ++j;
            
            if (j != val[i]->val.size()) {
                int _fildes = socket(AF_INET, SOCK_STREAM, 0);
                
                int opt = 1;
                
                setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                
                struct sockaddr_in add;
                    
                add.sin_addr.s_addr = INADDR_ANY;
                add.sin_family = AF_INET;
                add.sin_port = htons(port);
                
                struct socket* sock = val[i];
                
                bind(_fildes, (struct sockaddr *)&add, sock->addlen);
                listen(_fildes, 1);
                
                sock->parval.push_back(_fildes);
                sock->add.push_back(add);
                sock->fla.push_back(false);
                
                sock->fildes = fildes;
                
                thr.push_back(sock);
                
                sock->thr.push_back(thread(accept_handler_parallel));
                sock->thr.push_back(thread(read_handler));
                
                return _fildes;
            }
        }
        
        size_t j = 0;
        while (j < val[i]->parval.size() && val[i]->parval[j] != fildes)
            ++j;
        
        if (j != val[i]->parval.size())
            return -1;
    }
    
    //  socket is undefined
    if (i == val.size())
        return -1;
    
    struct socket* sock = val[i];
    
    int _fildes = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    
    setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in add;
        
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_family = AF_INET;
    add.sin_port = htons(port);
    
    int addlen = sizeof(add);
    
    bind(_fildes, (struct sockaddr *)&add, addlen);
    listen(_fildes, 1);
    
    sock->parval.push_back(_fildes);
    
    sock->set_address(add, addlen);
    
    sock->fildes = fildes;
    
    thr.push_back(sock);
    
    sock->thr.push_back(thread(accept_handler_parallel));
    sock->thr.push_back(thread(read_handler));
    
    return _fildes;
}

string recv(const int fildes) {
    size_t i;
    for (i = 0; i < val.size(); ++i) {
        if (val[i]->val[0] == fildes)
            break;
        
        if (val[i]->is_server()) {
            size_t j = 1;
            while (j < val[i]->val.size() && val[i]->val[j] != fildes)
                ++j;
            
            if (j != val[i]->val.size())
                break;
        }
        
        size_t j = 0;
        while (j < val[i]->parval.size() && val[i]->parval[j] != fildes)
            ++j;
        
        //  cannot read from socket
        if (j != val[i]->parval.size())
            return string();
    }
    
    //  socket is undefined
    if (i == val.size())
        return string();
    
    char valread[1024] = {0};
    
    return read(fildes, valread, 1024) <= 0 ? string() : string(valread);
}

int send(const int fildes, const string mes) {
    size_t i;
    for (i = 0; i < val.size(); ++i) {
        if (val[i]->val[0] == fildes)
            break;
        
        if (val[i]->is_server()) {
            size_t j = 1;
            while (j < val[i]->val.size() && val[i]->val[j] != fildes)
                ++j;
            
            if (j != val[i]->val.size())
                break;
        }
        
        size_t j = 0;
        while (j < val[i]->parval.size() && val[i]->parval[j] != fildes)
            ++j;
        
        //  cannot write to socket
        if (j != val[i]->parval.size())
            return -1;
    }
    
    //  socket is undefined
    if (i == val.size())
        return -1;
    
    //  check only that fildes does not belong to a listener
    
    return (int)send(fildes, mes.c_str(), mes.length(), MSG_NOSIGNAL);
}

int server(const int port, const int backlog) {
    //  check ports in use
    
    int fildes = socket(AF_INET, SOCK_STREAM, 0);
            
    int opt = 1;
    
    setsockopt(fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in add;
        
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_family = AF_INET;
    add.sin_port = htons(port);
    
    int addlen = sizeof(add);
    
    bind(fildes, (struct sockaddr *)&add, addlen);
    listen(fildes, backlog);
    
    struct socket* sock = new struct socket(fildes);
    
    sock->set_address(add, addlen);
    
    thr.push_back(sock);
    
    sock->thr.push_back(thread(accept_handler_server));
    
    val.push_back(sock);
    
    return fildes;
}

void signal_handler(int signum) {
    while (val.size())
        _close(val[0]->val[0]);
    
    exit(signum);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    
    cout << "-- Time Server ----------------\n";
    cout << "1\t-\tServer\n";
    cout << "2\t-\tParallel Client\n";
    cout << "3\t-\tClient\n";
    cout << "-------------------------------\n";
    cout << ">\t";
    
    switch (input()) {
        case 1: {
            int fildes = server(8080, 3);
            
            /*
            while (!accept(fildes).size());
            
            int _fildes = accept(fildes)[0];
            int __fildes = _listen(_fildes, 8081);
            
            _close(__fildes);
            _listen(_fildes, 8081);
             */
            
            cout << "Server listening on port 8080...\n";
                
            while (1) {
                //  /*
                vector<int> fildess = accept(fildes);
                
                for (size_t i = 0; i < fildess.size(); ++i) {
                    time_t now = time(0);
                    
                    tm *gmtm = gmtime(&now);
                    char* dt = asctime(gmtm);
                    
                    string valsend = string(dt);
                    
                    send(fildess[i], valsend);
                }
                
                this_thread::sleep_for(chrono::seconds(1));
                 // */
            }
            
            break;
        } case 2: {
            int fildes = client("127.0.0.1", 8080);
            
            /*
            while (1) {
                time_t now = time(0);
                
                tm *gmtm = gmtime(&now);
                char* dt = asctime(gmtm);
                
                string valsend = string(dt);
                
                send(fildes, valsend);
                
                this_thread::sleep_for(chrono::seconds(1));
            }
             */
            
            //  /*
            int _fildes = _listen(fildes, 8081);
                        
            _close(_fildes);
            _listen(fildes, 8081);
            
            cout << "Bridge listening on port 8081...\n";
            
            while (1);
            
            break;
             // */
        } case 3: {
            int fildes = client("127.0.0.1", 8081);
            
            while (1) {
                string valread = recv(fildes);
                
                if (valread.empty())
                    return 0;
                
                string valv[valread.length() + 1];
                size_t valc = split(valv, valread, "\n");
                
                size_t i = 0;
                while (i < valc) {
                    if (valv[i].empty()) {
                        for (size_t j = i; j < valc - 1; ++j)
                            swap(valv[j], valv[j + 1]);
                        
                        --valc;
                    } else
                        ++i;
                }
                
                ostringstream ss;
                
                for (i = 0; i < valc; ++i)
                    ss << valv[i] << "\n";
                
                valread = ss.str();
                
                if (valread.empty())
                    continue;
                
                cout << valread;
            }
            
            break;
        } default: {
            exit(0);
        }
    }
}
