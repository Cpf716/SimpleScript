//
//  socket.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/21/23.
//

#include "socket.h"

namespace ss {
    namespace api {
        //  CONSTRUCTORS

        socket::socket(const int fildes) {
            this->val.push_back(fildes);
            this->flg.push_back(true);
        }

        //  MEMBER FUNCTIONS

        bool socket::is_client() const {
            return (!this->add.size() && !this->parval.size()) || (this->add.size() == 1 && this->parval.size());
        }

        bool socket::is_parallel() const { return !!this->parval.size(); }

        bool socket::is_server() const {
            return (this->add.size() == 1 && !this->parval.size()) || (this->add.size() > 1 && this->parval.size());
        }

        void socket::set_address(const struct sockaddr_in add, const int addlen) {
            this->add.push_back(add);
            this->addlen = addlen;
            this->flg[0] = false;
        }

        //  NON-MEMBER FIELDS

        std::vector<struct socket*> thr;

        std::vector<struct socket*> sockv;

        //  NON-MEMBER FUNCTIONS

        void handler_parallel_accept() {
            struct socket* sock = thr[thr.size() - 1];
            
            while (1) {
                if (sock->flg[sock->flg.size() - 1])
                    break;
                
                //  returns nonnegative file descriptor or -1 for error
                int fildes = accept(sock->parval[1], (struct sockaddr *)&sock->add[sock->add.size() - 1], (socklen_t *)&sock->addlen);
                
                if (fildes == -1)
                    continue;
                
                sock->parval.push_back(fildes);
            }
            
            //  std::cout << "parallel accept joining...\n";
        }

        void handler_read() {
            struct socket* sock = thr[thr.size() - 1];
                
            while (1) {
                if (sock->flg[sock->flg.size() - 1])
                    break;
                
                char valread[1024] = {0};
                
                //  returns message length (bytes), 0 for closed connection, or -1 for error
                if (read(sock->parval[0], valread, 1024) <= 0)
                    continue;
                
                std::string val(valread);
                
                val += "\n\r";
                        
                size_t i = 2;
                while (i < sock->parval.size()) {
                    if (::send(sock->parval[i], val.c_str(), val.length(), MSG_NOSIGNAL) <= 0)
                        sock->parval.erase(sock->parval.begin() + i);
                    else
                        ++i;
                }
            }
            
            //  std::cout << "read joining...\n";
        }

        void handler_server_accept() {
            struct socket* sock = thr[thr.size() - 1];
            
            while (1) {
                if (sock->flg[0])
                    break;
                
                //  returns nonnegative file descriptor or -1 for error
                int fildes = accept(sock->val[0], (struct sockaddr *)&sock->add[0], (socklen_t *)&sock->addlen);
                
                if (fildes == -1)
                    continue;
                
                sock->val.push_back(fildes);
            }
            
            //  std::cout << "server accept joining...\n";
        }
    
        std::vector<int> socket_accept(const int fildes) {
            size_t i;
            for (i = 0; i < sockv.size(); ++i) {
                if (sockv[i]->val[0] == fildes) {
                    if (sockv[i]->is_client())
                        return std::vector<int>();
                    
                    break;
                }
                
                if (sockv[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv[i]->val.size() && sockv[i]->val[j] != fildes)
                        ++j;
                    
                    if (j != sockv[i]->val.size())
                        return std::vector<int>();
                }
                
                size_t j = 1;
                while (j < sockv[i]->parval.size() && sockv[i]->parval[j] != fildes)
                    ++j;
                
                if (j != sockv[i]->parval.size())
                    return std::vector<int>();
            }
            
            if (i == sockv.size())
                return std::vector<int>();
            
            struct socket* sock = sockv[i];
            
            i = 1;
            while (i < sock->val.size()) {
                if (::send(sock->val[i], std::string("\n\r").c_str(), 2, MSG_NOSIGNAL) <= 0)
                    sock->val.erase(sock->val.begin() + i);
                else
                    ++i;
            }
            
            return std::vector<int>(sock->val.begin() + 1, sock->val.end());
        }

        int socket_client(const std::string src, const int port) {
            int fildes;
            
            while (1) {
                fildes = ::socket(AF_INET, SOCK_STREAM, 0);
                
                if (fildes == -1)
                    throw api::socket_exception(std::to_string(errno));
                
                struct sockaddr_in add;

                add.sin_family = AF_INET;
                add.sin_port = htons(port);
                
                //  localhost
                if (inet_pton(AF_INET, src.c_str(), &add.sin_addr) == -1)
                    throw api::socket_exception(std::to_string(errno));
                
                if (!connect(fildes, (struct sockaddr *)&add, sizeof(add)))
                    break;
                
                //  returns 0 for success, -1 otherwise
                if (close(fildes))
                    throw api::socket_exception(std::to_string(errno));
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            sockv.push_back(new struct socket(fildes));
            
            return fildes;
        }

        int socket_close() {
            while (sockv.size())
                socket_close(sockv[0]->val[0]);
            
            return 0;
        }

        int socket_close(const int fildes) {
            for (size_t i = 0; i < sockv.size(); ++i) {
                struct socket* sock = sockv[i];
                
                if (sock->val[0] == fildes) {
                    if (sock->is_client()) {
                        if (close(sock->val[0]))
                            throw api::socket_exception(std::to_string(errno));
                    } else {
                        sock->flg[0] = true;
                        
                        shutdown(sock->val[0], SHUT_RDWR);
                        
                        for (size_t j = 0; j < sock->val.size(); ++j)
                            if (close(sock->val[j]))
                                throw api::socket_exception(std::to_string(errno));
                        
                        if (sock->thr[0].joinable())
                            sock->thr[0].join();
                    }
                    
                    if (sock->is_parallel()) {
                        sock->flg[sock->flg.size() - 1] = true;
                        
                        shutdown(sock->parval[1], SHUT_RDWR);
                        
                        for (size_t j = 1; j < sock->parval.size(); ++j)
                            if (close(sock->parval[j]))
                                throw api::socket_exception(std::to_string(errno));
                        
                        for (size_t j = sock->is_server(); j < sock->thr.size(); ++j)
                            if (sock->thr[j].joinable())
                                sock->thr[j].join();
                    }
                    
                    delete sock;
                    
                    sockv.erase(sockv.begin() + i);
                    
                    return 0;
                }
                
                if (sock->is_server()) {
                    size_t j = 1;
                    while (j < sock->val.size() && sock->val[j] != fildes)
                        ++j;
                    
                    if (j != sock->val.size()) {
                        if (close(sock->val[j]))
                            throw api::socket_exception(std::to_string(errno));
                        
                        sock->val.erase(sock->val.begin() + j);
                        
                        return 0;
                    }
                }
            }
            
            return -1;
        }

        void socket_listen(const int fildes, const int port) {
            size_t i;
            for (i = 0; i < sockv.size(); ++i) {
                if (sockv[i]->val[0] == fildes) {
                    if (sockv[i]->is_server() || sockv[i]->is_parallel())
                        return;
                    
                    break;
                }
                
                if (sockv[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv[i]->val.size() && sockv[i]->val[j] != fildes)
                        ++j;
                    
                    if (j != sockv[i]->val.size()) {
                        int _fildes = ::socket(AF_INET, SOCK_STREAM, 0);
                        
                        if (_fildes == -1)
                            throw api::socket_exception(std::to_string(errno));
                        
                        int opt = 1;
                        
                        if (setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
                            throw api::socket_exception(std::to_string(errno));
                        
                        struct sockaddr_in add;
                            
                        add.sin_addr.s_addr = INADDR_ANY;
                        add.sin_family = AF_INET;
                        add.sin_port = htons(port);
                        
                        struct socket* sock = sockv[i];
                        
                        if (bind(_fildes, (struct sockaddr *)&add, sock->addlen))
                            throw api::socket_exception(std::to_string(errno));
                        
                        if (listen(_fildes, 1))
                            throw api::socket_exception(std::to_string(errno));
                        
                        sock->parval.push_back(fildes);
                        sock->parval.push_back(_fildes);
                        sock->add.push_back(add);
                        sock->flg.push_back(false);
                        
                        thr.push_back(sock);
                        
                        sock->thr.push_back(std::thread(handler_parallel_accept));
                        sock->thr.push_back(std::thread(handler_read));
                        
                        return;
                    }
                }
                
                size_t j = 1;
                while (j < sockv[i]->parval.size() && sockv[i]->parval[j] != fildes)
                    ++j;
                
                if (j != sockv[i]->parval.size())
                    return;
            }
            
            //  socket is undefined
            if (i == sockv.size())
                return;
            
            struct socket* sock = sockv[i];
            
            int _fildes = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (_fildes == -1)
                throw api::socket_exception(std::to_string(errno));
            
            int opt = 1;
            
            if (setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
                throw api::socket_exception(std::to_string(errno));
            
            struct sockaddr_in add;
                
            add.sin_addr.s_addr = INADDR_ANY;
            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            int addlen = sizeof(add);
            
            if (bind(_fildes, (struct sockaddr *)&add, addlen))
                throw api::socket_exception(std::to_string(errno));
            
            if (listen(_fildes, 1))
                throw api::socket_exception(std::to_string(errno));
            
            sock->parval.push_back(fildes);
            sock->parval.push_back(_fildes);
            
            sock->set_address(add, addlen);
            
            thr.push_back(sock);
            
            sock->thr.push_back(std::thread(handler_parallel_accept));
            sock->thr.push_back(std::thread(handler_read));
        }

        std::string socket_recv(const int fildes) {
            size_t i;
            for (i = 0; i < sockv.size(); ++i) {
                if (sockv[i]->val[0] == fildes)
                    break;
                
                if (sockv[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv[i]->val.size() && sockv[i]->val[j] != fildes)
                        ++j;
                    
                    if (j != sockv[i]->val.size())
                        break;
                }
                
                size_t j = 1;
                while (j < sockv[i]->parval.size() && sockv[i]->parval[j] != fildes)
                    ++j;
                
                //  cannot read from socket
                if (j != sockv[i]->parval.size())
                    return std::string();
            }
            
            //  socket is undefined
            if (i == sockv.size())
                return std::string();
            
            while (1) {
                char valread[1024] = {0};
                
                if (read(fildes, valread, 1024) <= 0)
                    return std::string();
                
                std::size_t valc = 0;
                std::string valv[strlen(valread) + 1];
                
                std::stringstream ss(valread);
                std::string str;
                
                while (getline(ss, str))
                    valv[valc++] = str;
                
                i = 0;
                
                while (i < valc) {
                    //  trim
                    size_t beg = 0;
                    while (beg < valv[i].length() && isspace(valv[i][beg]))
                        ++i;
                    
                    size_t end = valv[i].length();
                    while (end > 0 && isspace(valv[i][end - 1]))
                        --end;
                    
                    if (valv[i].substr(beg, end - beg).empty()) {
                        for (size_t j = i; j < valc - 1; ++j)
                            swap(valv[j], valv[j + 1]);
                        
                        --valc;
                    } else
                        ++i;
                }
                
                if (!valc)
                    continue;
                
                ss.str("");
                ss.clear();
                            
                for (i = 0; i < valc - 1; ++i)
                    ss << valv[i] << std::endl;
                
                ss << valv[valc - 1];
                
                return ss.str();
            }
        }

        int socket_send(const int fildes, const std::string msg) {
            size_t i;
            for (i = 0; i < sockv.size(); ++i) {
                if (sockv[i]->val[0] == fildes)
                    break;
                
                if (sockv[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv[i]->val.size() && sockv[i]->val[j] != fildes)
                        ++j;
                    
                    if (j != sockv[i]->val.size())
                        break;
                }
                
                size_t j = 1;
                while (j < sockv[i]->parval.size() && sockv[i]->parval[j] != fildes)
                    ++j;
                
                //  cannot write to socket
                if (j != sockv[i]->parval.size())
                    return -1;
            }
            
            //  socket is undefined
            if (i == sockv.size())
                return -1;
            
            //  check only that fildes does not belong to a listener
            
            return (int)::send(fildes, (msg + "\n\r").c_str(), msg.length() + 2, MSG_NOSIGNAL);
        }

        int socket_server(const int port, const int backlog) {
            //  check ports in use
            
            int fildes = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (fildes == -1)
                throw api::socket_exception(std::to_string(errno));
                    
            int opt = 1;
            
            if (setsockopt(fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
                throw api::socket_exception(std::to_string(errno));
            
            struct sockaddr_in add;
                
            add.sin_addr.s_addr = htonl(INADDR_ANY);
            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            int addlen = sizeof(add);
            
            if (bind(fildes, (struct sockaddr *)&add, addlen))
                throw api::socket_exception(std::to_string(errno));
            
            if (listen(fildes, backlog))
                throw api::socket_exception(std::to_string(errno));
            
            struct socket* sock = new struct socket(fildes);
            
            sock->set_address(add, addlen);
            
            thr.push_back(sock);
            
            sock->thr.push_back(std::thread(handler_server_accept));
            
            sockv.push_back(sock);
            
            return fildes;
        }
    }
}
