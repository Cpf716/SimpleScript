//
//  socket.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/21/23.
//

#include "socket.h"

namespace ss {
    namespace middleware {
    
        //  CONSTRUCTORS

        socket_tcp::socket_tcp(const int fildes) {
            this->get_val().push_back(fildes);
            this->get_flag().push_back(true);
        }
    
        socket_udp::socket_udp(const int fildes, struct sockaddr_in* add) {
            this->val = fildes;
            this->add = add;
        }

        //  MEMBER FUNCTIONS
    
        std::vector<struct sockaddr_in>& socket_tcp::get_add() {
            return this->add;
        }
        
        int& socket_tcp::get_addlen() {
            return this->addlen;
        }
        
        std::vector<int>& socket_tcp::get_parval() {
            return this->parval;
        }
        
        std::vector<bool>& socket_tcp::get_flag() {
            return this->flag;
        }
        
        std::vector<std::thread>& socket_tcp::get_thr() {
            return this->thr;
        }
        
        std::vector<int>& socket_tcp::get_val() {
            return this->val;
        }
    
        bool socket_tcp::is_client() const {
            return (!this->add.size() && !this->parval.size()) || (this->add.size() == 1 && this->parval.size());
        }

        bool socket_tcp::is_parallel() const {
            return !!this->parval.size();
        }

        bool socket_tcp::is_server() const {
            return (this->add.size() == 1 && !this->parval.size()) || (this->add.size() > 1 && this->parval.size());
        }
    
        void socket_tcp::set_addlen(const int addlen) {
            this->addlen = addlen;
        }

        void socket_tcp::set_address(const struct sockaddr_in add, const int addlen) {
            this->get_add().push_back(add);
            this->set_addlen(addlen);
            this->get_flag()[0] = false;
        }
    
        struct sockaddr_in* socket_udp::get_add() const {
            return this->add;
        }
    
        int socket_udp::get_val() const {
            return this->val;
        }

        //  NON-MEMBER FIELDS
    
        std::vector<struct socket_tcp*> sockv_tcp;
    
        std::vector<struct socket_udp*> sockv_udp;
    
        std::vector<struct socket_tcp*> thr;
    
        std::vector<std::pair<int, int>> timeoutv_tcp;
    
        std::vector<std::pair<int, int>> timeoutv_udp;

        //  NON-MEMBER FUNCTIONS

        void handler_parallel_accept() {
            struct socket_tcp* sock = thr[thr.size() - 1];
            
            while (1) {
                if (sock->get_flag()[sock->get_flag().size() - 1])
                    break;
                
                //  returns nonnegative file descriptor or -1 for error
                int fildes = accept(sock->get_parval()[1], (struct sockaddr *)&sock->get_add()[sock->get_add().size() - 1], (socklen_t *)&sock->get_addlen());
                
                if (fildes == -1)
                    continue;
                
                sock->get_parval().push_back(fildes);
            }
            
//            std::cout << "parallel accept joining...\n";
        }

        void handler_read() {
            struct socket_tcp* sock = thr[thr.size() - 1];
                
            while (1) {
                if (sock->get_flag()[sock->get_flag().size() - 1])
                    break;
                
                char valread[2048] = {0};
                
                //  returns message length (bytes), 0 for closed connection, or -1 for error
                if (read(sock->get_parval()[0], valread, 2048) <= 0)
                    continue;
                
                std::string val(valread);
                
                val += "\n\r";
                
                while (1) {
                    if (sock->get_flag()[sock->get_flag().size() - 1])
                        break;
                    
                    size_t i = 2;
                    while (i < sock->get_parval().size()) {
                        if (::send(sock->get_parval()[i], val.c_str(), val.length(), MSG_NOSIGNAL) <= 0) {
                            if (close(sock->get_parval()[i]))
                                throw socket_exception(std::to_string(errno));
                            
                            sock->get_parval().erase(sock->get_parval().begin() + i);
                        } else
                            ++i;
                    }
                    
                    if (i != 2)
                        break;
                }
            }
            
            //  std::cout << "read joining...\n";
        }

        void handler_accept() {
            struct socket_tcp* sock = thr[thr.size() - 1];
            
            while (1) {
                if (sock->get_flag()[0])
                    break;
                
                //  returns nonnegative file descriptor or -1 for error
                int fildes = accept(sock->get_val()[0], (struct sockaddr *)&sock->get_add()[0], (socklen_t *)&sock->get_addlen());
                
                if (fildes == -1)
                    continue;
                
                sock->get_val().push_back(fildes);
            }
            
//            std::cout << "accept joining...\n";
        }
    
        void set_timeout_tcp() {
            size_t timeoutc = timeoutv_tcp.size() - 1;
            
            std::this_thread::sleep_for(std::chrono::seconds(timeoutv_tcp[timeoutc].second));
            
            if (timeoutc == timeoutv_tcp.size() - 1 && timeoutv_tcp[timeoutc].second)
                socket_tcp_close(timeoutv_tcp[timeoutc].first);
        }
    
        void set_timeout_udp() {
            size_t timeoutc = timeoutv_udp.size() - 1;
            
            std::this_thread::sleep_for(std::chrono::seconds(timeoutv_udp[timeoutc].second));
            
            if (timeoutc == timeoutv_udp.size() - 1 && timeoutv_udp[timeoutc].second)
                socket_udp_close(timeoutv_udp[timeoutc].first);
        }

        int socket_close() {
            size_t i = 0;
            while (sockv_tcp.size() - i) {
                try {
                    socket_tcp_close(sockv_tcp[i]->get_val()[0]);
                    
                } catch (exception& err) {
//                    std::cout << err.what() << std::endl;
                    ++i;
                }
            }
            
            i = 0;
            while (sockv_udp.size() - i) {
                try {
                    socket_udp_close(sockv_udp[i]->get_val());
                    
                } catch (exception& err) {
//                    std::cout << err.what() << std::endl;
                    ++i;
                }
            }
            
            return 0;
        }

        int socket_tcp_close(const int fildes) {
            for (size_t i = 0; i < sockv_tcp.size(); ++i) {
                struct socket_tcp* sock = sockv_tcp[i];
                
                if (sock->get_val()[0] == fildes) {
                    //  close client
                    if (sock->is_client()) {
                        if (close(sock->get_val()[0]))
                            throw socket_exception(std::to_string(errno));
                    } else {
                        //  close server
                        sock->get_flag()[0] = true;
                        
                        if (shutdown(sock->get_val()[0], SHUT_RDWR))
                            throw socket_exception(std::to_string(errno));
                        
                        for (size_t j = 0; j < sock->get_val().size(); ++j)
                            if (close(sock->get_val()[j]))
                                throw socket_exception(std::to_string(errno));
                        
                        if (sock->get_thr()[0].joinable())
                            sock->get_thr()[0].join();
                    }
                    
                    //  close parallel clients
                    if (sock->is_parallel()) {
                        sock->get_flag()[sock->get_flag().size() - 1] = true;
                        
                        if (shutdown(sock->get_parval()[1], SHUT_RDWR))
                            throw socket_exception(std::to_string(errno));
                        
                        for (size_t j = 1; j < sock->get_parval().size(); ++j)
                            if (close(sock->get_parval()[j]))
                                throw socket_exception(std::to_string(errno));
                        
                        for (size_t j = sock->is_server(); j < sock->get_thr().size(); ++j)
                            if (sock->get_thr()[j].joinable())
                                sock->get_thr()[j].join();
                    }
                    
                    delete sock;
                    
                    sockv_tcp.erase(sockv_tcp.begin() + i);
                    
                    return 0;
                }
                
                //  close server client
                if (sock->is_server()) {
                    size_t j = 1;
                    while (j < sock->get_val().size() && sock->get_val()[j] != fildes)
                        ++j;
                    
                    if (j != sock->get_val().size()) {
                        if (close(sock->get_val()[j]))
                            throw socket_exception(std::to_string(errno));
                        
                        sock->get_val().erase(sock->get_val().begin() + j);
                            
                        //  close parallel client
                        if (sock->get_parval().size() && sock->get_parval()[0] == fildes) {
                            sock->get_flag()[sock->get_flag().size() - 1] = true;
                            
                            if (shutdown(sock->get_parval()[1], SHUT_RDWR))
                                throw socket_exception(std::to_string(errno));
                            
                            for (size_t k = 1; k < sock->get_parval().size(); ++k)
                                if (close(sock->get_parval()[k]))
                                    throw socket_exception(std::to_string(errno));
                            
                            for (size_t k = 1; k < sock->get_thr().size(); ++k)
                                if (sock->get_thr()[k].joinable())
                                    sock->get_thr()[k].join();
                            
                            sock->get_add().pop_back();
                            sock->get_flag().pop_back();
                            sock->get_parval().clear();
                            
                            while (sock->get_thr().size() > 1)
                                sock->get_thr().pop_back();
                        }
                        
                        return 0;
                    }
                }
            }
            
            return -1;
        }
    
        int socket_udp_close(const int fildes) {
            size_t i = 0;
            while (i < sockv_udp.size() && sockv_udp[i]->get_val() != fildes)
                ++i;
            
            if (i == sockv_udp.size())
                return -1;
            
            struct socket_udp* sock = sockv_udp[i];
            
            if (close(fildes))
                throw socket_exception(std::to_string(errno));
            
            delete sock->get_add();
            delete sock;
            
            sockv_udp.erase(sockv_udp.begin() + i);
            
            return 0;
        }

        int socket_listen(const int fildes, const int port) {
#if DEBUG_LEVEL
            assert(port >= 0);
#endif
            size_t i;
            for (i = 0; i < sockv_tcp.size(); ++i) {
                if (sockv_tcp[i]->get_val()[0] == fildes) {
                    if (sockv_tcp[i]->is_server() || sockv_tcp[i]->is_parallel())
                        return -1;
                    
                    break;
                }
                
                if (sockv_tcp[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_val().size() && sockv_tcp[i]->get_val()[j] != fildes)
                        ++j;
                    
                    if (j != sockv_tcp[i]->get_val().size()) {
                        int _fildes = ::socket(AF_INET, SOCK_STREAM, 0);
                        
                        if (_fildes == -1)
                            throw socket_exception(std::to_string(errno));
                        
                        int opt = 1;
                        
                        if (setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
                            close(_fildes);
                            throw socket_exception(std::to_string(errno));
                        }
                        
                        struct sockaddr_in add;
                            
                        add.sin_addr.s_addr = INADDR_ANY;
                        add.sin_family = AF_INET;
                        add.sin_port = htons(port);
                        
                        struct socket_tcp* sock = sockv_tcp[i];
                        
                        if (bind(_fildes, (struct sockaddr *)&add, sock->get_addlen())) {
                            close(_fildes);
                            throw socket_exception(std::to_string(errno));
                        }
                        
                        if (listen(_fildes, 1)) {
                            close(_fildes);
                            throw socket_exception(std::to_string(errno));
                        }
                        
                        sock->get_add().push_back(add);
                        sock->get_flag().push_back(false);
                        sock->get_parval().push_back(fildes);
                        sock->get_parval().push_back(_fildes);
                        
                        thr.push_back(sock);
                        
                        sock->get_thr().push_back(std::thread(handler_parallel_accept));
                        sock->get_thr().push_back(std::thread(handler_read));
                        
                        return 0;
                    }
                }
                
                size_t j = 1;
                while (j < sockv_tcp[i]->get_parval().size() && sockv_tcp[i]->get_parval()[j] != fildes)
                    ++j;
                
                if (j != sockv_tcp[i]->get_parval().size())
                    return -1;
            }
            
            //  socket is undefined
            if (i == sockv_tcp.size())
                return -1;
            
            struct socket_tcp* sock = sockv_tcp[i];
            
            int _fildes = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (_fildes == -1)
                throw socket_exception(std::to_string(errno));
            
            int opt = 1;
            
            if (setsockopt(_fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
                close(_fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            struct sockaddr_in add;
                
            add.sin_addr.s_addr = INADDR_ANY;
            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            int addlen = sizeof(add);
            
            if (bind(_fildes, (struct sockaddr *)&add, addlen)) {
                close(_fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            if (listen(_fildes, 1)) {
                close(_fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            sock->get_parval().push_back(fildes);
            sock->get_parval().push_back(_fildes);
            
            sock->set_address(add, addlen);
            
            thr.push_back(sock);
            
            sock->get_thr().push_back(std::thread(handler_parallel_accept));
            sock->get_thr().push_back(std::thread(handler_read));
            
            return 0;
        }
    
        std::vector<int> socket_poll(const int fildes) {
            size_t i;
            for (i = 0; i < sockv_tcp.size(); ++i) {
                if (sockv_tcp[i]->get_val()[0] == fildes) {
                    if (sockv_tcp[i]->is_client())
                        return std::vector<int>();
                    
                    break;
                }
                
                if (sockv_tcp[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_val().size() && sockv_tcp[i]->get_val()[j] != fildes)
                        ++j;
                    
                    if (j != sockv_tcp[i]->get_val().size())
                        return std::vector<int>();
                }
                
                if (sockv_tcp[i]->is_parallel()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_parval().size() && sockv_tcp[i]->get_parval()[j] != fildes)
                        ++j;
                    
                    if (j != sockv_tcp[i]->get_parval().size())
                        return std::vector<int>();
                }
            }
            
            if (i == sockv_tcp.size())
                return std::vector<int>();
            
            struct socket_tcp* sock = sockv_tcp[i];
            
            i = 1;
            while (i < sock->get_val().size()) {
                if (::send(sock->get_val()[i], std::string("\n\r").c_str(), 2, MSG_NOSIGNAL) <= 0) {
                    if (close(sock->get_val()[i]))
                        throw socket_exception(std::to_string(errno));
                    
                    sock->get_val().erase(sock->get_val().begin() + i);
                } else
                    ++i;
            }
            
            return std::vector<int>(sock->get_val().begin() + 1, sock->get_val().end());
        }

        std::string socket_recv(const int fildes, int timeout) {
            size_t i;
            for (i = 0; i < sockv_tcp.size(); ++i) {
                if (sockv_tcp[i]->get_val()[0] == fildes)
                    break;
                
                if (sockv_tcp[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_val().size() && sockv_tcp[i]->get_val()[j] != fildes)
                        ++j;
                    
                    if (j != sockv_tcp[i]->get_val().size())
                        break;
                }
                
                size_t j = 1;
                while (j < sockv_tcp[i]->get_parval().size() && sockv_tcp[i]->get_parval()[j] != fildes)
                    ++j;
                
                //  cannot read from socket
                if (j != sockv_tcp[i]->get_parval().size())
                    return std::string();
            }
            
            //  socket is undefined
            if (i == sockv_tcp.size())
                return std::string();
            
            int timeoutc = -1;
            
            if (timeout > 0) {
                timeoutc = (int)timeoutv_tcp.size();
                
                timeoutv_tcp.push_back(std::pair<int, int>(fildes, timeout));
                
                std::thread(set_timeout_tcp).detach();
            }
            
            while (1) {
                char valread[2048] = {0};
                
                ssize_t res = read(fildes, valread, 2048);
                
                if (res == -1)
                    throw socket_exception(std::to_string(errno));
                
                if (!res) {
                    if (timeoutc != -1)
                        timeoutv_tcp[timeoutc].second = 0;
                    
                    return std::string();
                }
                
                //  leading \n\r is caused by poll()
                
                size_t valc = 0;
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
                    
                    if (beg == end) {
                        for (size_t j = i; j < valc - 1; ++j)
                            swap(valv[j], valv[j + 1]);
                        
                        --valc;
                    } else
                        ++i;
                }
                
                if (!valc)
                    continue;
                
                if (timeoutc != -1)
                    timeoutv_tcp[timeoutc].second = 0;
                
                ss.str(std::string());
                ss.clear();
                            
                for (i = 0; i < valc - 1; ++i)
                    ss << valv[i] << "\n";
                
                ss << valv[valc - 1];
                
                return ss.str();
            }
        }
    
        std::string socket_recvfrom(const int fildes, int timeout) {
            size_t i = 0;
            while (i < sockv_udp.size() && sockv_udp[i]->get_val() != fildes)
                ++i;
            
            if (i == sockv_udp.size())
                return std::string();
            
            struct socket_udp* sock = sockv_udp[i];
            
            ssize_t res;
            char buffer[2048];
            socklen_t len;
            
            len = sizeof(* sock->get_add());
            
            int timeoutc = -1;
            
            if (timeout > 0) {
                timeoutc = (int)timeoutv_udp.size();
                
                timeoutv_udp.push_back(std::pair<int, int>(fildes, timeout));
                
                std::thread(set_timeout_udp).detach();
            }
            
            res = recvfrom(sock->get_val(), (char *)buffer, 2048, MSG_WAITALL, (struct sockaddr *)sock->get_add(), &len);
            
            if (res == -1)
                throw socket_exception(std::to_string(errno));
            
            if (timeoutc != -1)
                timeoutv_udp[timeoutc].second = 0;
            
            buffer[res] = '\0';
            
            return std::string(buffer);
        }

        int socket_send(const int fildes, const std::string msg) {
            size_t i;
            for (i = 0; i < sockv_tcp.size(); ++i) {
                if (sockv_tcp[i]->get_val()[0] == fildes)
                    break;
                
                if (sockv_tcp[i]->is_server()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_val().size() && sockv_tcp[i]->get_val()[j] != fildes)
                        ++j;
                    
                    if (j != sockv_tcp[i]->get_val().size())
                        break;
                }
                
                if (sockv_tcp[i]->is_parallel()) {
                    size_t j = 1;
                    while (j < sockv_tcp[i]->get_parval().size() && sockv_tcp[i]->get_parval()[j] != fildes)
                        ++j;
                    
                    //  cannot write to socket
                    if (j != sockv_tcp[i]->get_parval().size())
                        return -1;
                }
            }
            
            //  socket is undefined
            if (i == sockv_tcp.size())
                return -1;
            
            ssize_t res = ::send(fildes, (msg + "\n\r").c_str(), msg.length() + 2, MSG_NOSIGNAL);
            
            if (res == -1)
                throw socket_exception(std::to_string(errno));
            
            return (int)res;
        }
    
        int socket_sendto(const int fildes, const std::string mes) {
            size_t i = 0;
            while (i < sockv_udp.size() && sockv_udp[i]->get_val() != fildes)
                ++i;
            
            if (i == sockv_udp.size())
                return -1;
            
            struct socket_udp* sock = sockv_udp[i];
            
            ssize_t res;
            socklen_t len;
            
            len = sizeof(* sock->get_add());
            res = ::sendto(sock->get_val(), (const char *)mes.c_str(), mes.length(), 0, (const struct sockaddr *)sock->get_add(), len);
            
            if (res == -1)
                throw socket_exception(std::to_string(res));
            
            return (int)res;
        }
    
        int socket_tcp_client(const std::string src, const int port) {
#if DEBUG_LEVEL
            assert(port >= 0);
#endif
            int fildes = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (fildes == -1)
                throw socket_exception(std::to_string(errno));
            
            struct sockaddr_in add;

            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            if (inet_pton(AF_INET, src.c_str(), &add.sin_addr) == -1)
                throw socket_exception(std::to_string(errno));
            
            //  returns 0 for success, -1 otherwise
            if (connect(fildes, (struct sockaddr *)&add, sizeof(add))) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            sockv_tcp.push_back(new struct socket_tcp(fildes));
            
            return fildes;
        }

        int socket_tcp_server(const int port, const int backlog) {
#if DEBUG_LEVEL
            assert(port >= 0);
            assert(backlog >= 1);
#endif
            int fildes = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (fildes == -1)
                throw socket_exception(std::to_string(errno));
                    
            int opt = 1;
            
            if (setsockopt(fildes, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            struct sockaddr_in add;
                
            add.sin_addr.s_addr = INADDR_ANY;
            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            int addlen = sizeof(add);
            
            if (bind(fildes, (struct sockaddr *)&add, addlen)) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            if (listen(fildes, backlog)) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            struct socket_tcp* sock = new struct socket_tcp(fildes);
            
            sock->set_address(add, addlen);
            
            thr.push_back(sock);
            
            sock->get_thr().push_back(std::thread(handler_accept));
            
            sockv_tcp.push_back(sock);
            
            return fildes;
        }
    
        int socket_udp_client(const std::string src, const int port) {
#if DEBUG_LEVEL
            assert(src.length());
            assert(port >= 0);
#endif
            int fildes = ::socket(AF_INET, SOCK_DGRAM, 0);
            
            if (fildes == -1)
                throw socket_exception(std::to_string(errno));
            
            struct sockaddr_in* add = new struct sockaddr_in();
            
            memset(add, 0, sizeof(* add));
            
            add->sin_family = AF_INET;
            add->sin_port = htons(port);
            
            if (inet_pton(AF_INET, src.c_str(), &add->sin_addr) == -1) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            struct socket_udp* sock = new struct socket_udp(fildes, add);
            
            sockv_udp.push_back(sock);
            
            return fildes;
        }
    
        int socket_udp_server(const int port) {
#if DEBUG_LEVEL
            assert(port >= 0);
#endif
            int fildes = ::socket(AF_INET, SOCK_DGRAM, 0);
            
            if (fildes == -1)
                throw socket_exception(std::to_string(errno));
            
            struct sockaddr_in add;
            
            memset(&add, 0, sizeof(add));
            
            add.sin_addr.s_addr = INADDR_ANY;
            add.sin_family = AF_INET;
            add.sin_port = htons(port);
            
            if (bind(fildes, (const struct sockaddr *)&add, sizeof(add))) {
                close(fildes);
                throw socket_exception(std::to_string(errno));
            }
            
            struct sockaddr_in* _add = new struct sockaddr_in();
            
            memset(_add, 0, sizeof(* _add));
            
            struct socket_udp* sock = new struct socket_udp(fildes, _add);
            
            sockv_udp.push_back(sock);
            
            return fildes;
        }
    }
}
