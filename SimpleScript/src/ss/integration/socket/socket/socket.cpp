//
// socket.cpp
// SimpleScript
//
// Created by Corey Ferguson on 9/21/23.
//

#include "socket.h"

namespace ss {
    namespace integration {
        // NON-MEMBER FIELDS
    
        std::mutex               socket_mutex;
        std::vector<socket_tcp*> socketv_tcp;
        std::vector<socket_udp*> socketv_udp;
        int                      timeoutc = 0;
        std::vector<int>         timeoutv;
    
        // NON-MEMBER FUNCTIONS
        // private
        
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
    
        void clear_timeout(int timeout) {
            if (timeout == -1)
                return;
            
            socket_mutex.lock();
            
            int pos = find_timeout(timeout);
            
            if (pos != -1)
                timeoutv.erase(timeoutv.begin() + pos);
                
            socket_mutex.unlock();
        }

        int find_socket(const int key, const size_t begin = 0, const size_t end = socketv_udp.size()) {
            if (begin == end)
                return -1;
            
            size_t len = floor((end - begin) / 2);
            
            if (socketv_udp[begin + len]->file_descriptor() == key)
                return (int)(begin + len);
            
            if (socketv_udp[begin + len]->file_descriptor() > key)
                return find_socket(key, begin, begin + len);
            
            return find_socket(key, begin + len + 1, end);
        }
    

        void handle_accept(socket_tcp* socket) {
            while (true) {
                if (socket->flag.load())
                    break;
                
                // returns nonnegative file descriptor or -1 for error
                int fd = accept(socket->file_descriptors[0], (struct sockaddr *)&socket->addresses[0], (socklen_t *)&socket->address_length);
                
                if (fd == -1)
                    continue;
                
                socket_mutex.lock();
                
                socket->file_descriptors.push_back(fd);
                
                socket_mutex.unlock();
            }
        }
    
        void set_timeout(std::tuple<int, int, int> timeout, std::function<void(int)> cb) {
            std::this_thread::sleep_for(std::chrono::seconds(std::get<2>(timeout)));
            
            socket_mutex.lock();
            
            int pos = find_timeout(std::get<0>(timeout));
            
            if (pos == -1) {
                socket_mutex.unlock();
                return;
            }
            
            timeoutv.erase(timeoutv.begin() + pos);
                
            socket_mutex.unlock();
            
            cb(std::get<1>(timeout));
        }

        // public

        int socket_close() {
            size_t pos = 0;
            
            while (socketv_tcp.size() - pos) {
                try {
                    socket_close_tcp(socketv_tcp[pos]->file_descriptors[0]);
                    
                } catch (exception& err) {
                    ++pos;
                }
            }
            
            pos = 0;
            
            while (socketv_udp.size() - pos) {
                try {
                    socket_close_udp(socketv_udp[pos]->file_descriptor());
                    
                } catch (exception& err) {
                    ++pos;
                }
            }
            
            return 0;
        }

        int socket_close_tcp(const int file_descriptor) {
            socket_mutex.lock();
            
            size_t i;
            for (i = 0; i < socketv_tcp.size(); ++i) {
                socket_tcp* sock = socketv_tcp[i];
                
                if (sock->file_descriptors[0] == file_descriptor) {
                    // server
                    if (sock->addresses.size()) {
                        sock->flag.store(true);

                        if (shutdown(sock->file_descriptors[0], SHUT_RDWR)) {
                            socket_mutex.unlock();

                            throw socket_exception(std::to_string(errno));
                        }

                        // To Do:
                        // * If client errs on close, then what?
                        for (size_t j = 0; j < sock->file_descriptors.size(); ++j) {
                            if (close(sock->file_descriptors[j])) {
                                socket_mutex.unlock();

                                throw socket_exception(std::to_string(errno));
                            }
                        }
                        
                        if (sock->threads[0].joinable())
                            sock->threads[0].join();
                        
                        // client
                    } else if (close(sock->file_descriptors[0])) {
                        socket_mutex.unlock();
                        
                        throw socket_exception(std::to_string(errno));
                    }
                    
                    delete sock;
                    
                    socketv_tcp.erase(socketv_tcp.begin() + i);
                    socket_mutex.unlock();
                    
                    return 0;
                }
                
                // server
                if (sock->addresses.size()) {
                    size_t j = 1;
                    while (j < sock->file_descriptors.size() && sock->file_descriptors[j] != file_descriptor)
                        ++j;
                    
                    if (j == sock->file_descriptors.size())
                        continue;
                    
                    if (close(sock->file_descriptors[j])) {
                        socket_mutex.unlock();
                        
                        throw socket_exception(std::to_string(errno));
                    }
                    
                    sock->file_descriptors.erase(sock->file_descriptors.begin() + j);
                    
                    socket_mutex.unlock();
                    
                    return 0;
                }
            }
            
            return -1;
        }
    
        int socket_close_udp(const int file_descriptor) {
            socket_mutex.lock();
            
            int pos = find_socket(file_descriptor);
            
            if (pos == -1) {
                socket_mutex.unlock();
                
                return -1;
            }
            
            if (close(file_descriptor)) {
                socket_mutex.unlock();
                
                throw socket_exception(std::to_string(errno));
            }
            
            delete socketv_udp[pos]->address();
            delete socketv_udp[pos];
            
            socketv_udp.erase(socketv_udp.begin() + pos);
            socket_mutex.unlock();
            
            return 0;
        }
    
        std::vector<int> socket_poll(const int file_descriptor) {
            socket_mutex.lock();
            
            size_t i;
            for (i = 0; i < socketv_tcp.size(); ++i) {
                socket_tcp* sock = socketv_tcp[i];
                
                if (sock->file_descriptors[0] == file_descriptor) {
                    // client
                    if (!sock->addresses.size()) {
                        socket_mutex.unlock();
                        
                        return std::vector<int>();
                    }
                    break;
                }
                
                // server
                if (sock->addresses.size()) {
                    size_t j = 1;
                    while (j < sock->file_descriptors.size() && sock->file_descriptors[j] != file_descriptor)
                        ++j;
                    
                    if (j == sock->file_descriptors.size())
                        continue;
                    
                    socket_mutex.unlock();
                    
                    return std::vector<int>();
                }
            }
            
            if (i == socketv_tcp.size()) {
                socket_mutex.unlock();
                
                return std::vector<int>();
            }
            
            socket_tcp* sock = socketv_tcp[i];
            
            size_t j = 1;
            while (j < sock->file_descriptors.size()) {
                if (::send(sock->file_descriptors[j], std::string("\n\r").c_str(), 2, MSG_NOSIGNAL) <= 0) {
                    if (close(sock->file_descriptors[j])) {
                        socket_mutex.unlock();
                        
                        throw socket_exception(std::to_string(errno));
                    }
                    
                    sock->file_descriptors.erase(sock->file_descriptors.begin() + j);
                } else
                    ++j;
            }
            
            std::vector<int> res = std::vector<int>(sock->file_descriptors.begin() + 1, sock->file_descriptors.end());
            
            socket_mutex.unlock();
            
            return res;
        }

        std::string socket_recv(const int file_descriptor, const int timeout) {
            socket_mutex.lock();
            
            size_t i;
            for (i = 0; i < socketv_tcp.size(); ++i) {
                socket_tcp* sock = socketv_tcp[i];
                
                if (sock->file_descriptors[0] == file_descriptor)
                    break;
                
                // server
                if (sock->addresses.size()) {
                    size_t j = 1;
                    while (j < sock->file_descriptors.size() && sock->file_descriptors[j] != file_descriptor)
                        ++j;
                    
                    if (j != sock->file_descriptors.size())
                        break;
                }
            }
            
            if (i == socketv_tcp.size()) {
                socket_mutex.unlock();
                
                return std::string();
            }
            
            socket_mutex.unlock();
            
            while (true) {
                int pos = -1;
                
                if (timeout > 0) {
                    socket_mutex.lock();
                    
                    pos = timeoutc++;
                    
                    timeoutv.push_back(pos);
                    socket_mutex.unlock();
                    
                    std::thread(set_timeout, std::tuple<int, int, int>(pos, file_descriptor, timeout), [](const int  file_descriptor) {
                        socket_close_tcp(file_descriptor);
                    }).detach();
                }
                
                char buff[2048] = {0};
                
                ssize_t len = recv(file_descriptor, buff, 2048, 0);
                
                clear_timeout(pos);
                
                if (len == -1)
                    throw socket_exception(std::to_string(errno));
                
                if (!len)
                    return std::string();
                
                // leading \n\r is caused by poll()
                
                std::stringstream ss(buff);
                std::string       str;
                std::string       valv[strlen(buff) + 1];
                size_t            valc = 0;
                
                while (getline(ss, str))
                    valv[valc++] = str;
                
                i = 0;
                while (i < valc) {
                    // trim
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
                
                ss.str(std::string());
                ss.clear();
                            
                for (i = 0; i < valc - 1; ++i)
                    ss << valv[i] << "\n";
                
                ss << valv[valc - 1];
                
                return ss.str();
            }
        }
    
        std::string socket_recvfrom(const int file_descriptor, const int timeout) {
            socket_mutex.lock();
            
            int pos = find_socket(file_descriptor);
            
            if (pos == -1) {
                socket_mutex.unlock();
                
                return std::string();
            }
            
            // thread-safe?
            socket_udp* sock = socketv_udp[pos];
            
            socket_mutex.unlock();
            
            int _pos = -1;
            
            if (timeout > 0) {
                socket_mutex.lock();
                
                _pos = timeoutc++;
                
                timeoutv.push_back(pos);
                socket_mutex.unlock();
                
                std::thread(set_timeout, std::tuple<int, int, int>(_pos, file_descriptor, timeout), [](const int  file_descriptor) {
                    socket_close_udp(file_descriptor);
                }).detach();
            }
            
            char      buff[2048];
            socklen_t addrlen = sizeof(* sock->address());
            ssize_t   len = recvfrom(sock->file_descriptor(), (char *)buff, 2048, MSG_WAITALL, (struct sockaddr *)sock->address(), &addrlen);
            
            clear_timeout(_pos);
            
            if (len == -1)
                throw socket_exception(std::to_string(errno));
            
            buff[len] = '\0';
            
            return std::string(buff);
        }

        int socket_send(const int file_descriptor, const std::string value) {
            socket_mutex.lock();
            
            size_t i;
            for (i = 0; i < socketv_tcp.size(); ++i) {
                socket_tcp* sock = socketv_tcp[i];
                
                if (sock->file_descriptors[0] == file_descriptor)
                    break;
                
                // server
                if (sock->addresses.size()) {
                    size_t j = 1;
                    while (j < sock->file_descriptors.size() && sock->file_descriptors[j] != file_descriptor)
                        ++j;
                    
                    if (j != sock->file_descriptors.size())
                        break;
                }
            }
            
            if (i == socketv_tcp.size()) {
                socket_mutex.unlock();
                
                return -1;
            }
            
            socket_mutex.unlock();
            
            ssize_t len = ::send(file_descriptor, (value + "\n\r").c_str(), value.length() + 2, MSG_NOSIGNAL);
            
            if (len == -1)
                throw socket_exception(std::to_string(errno));
            
            return (int)len;
        }
    
        int socket_sendto(const int file_descriptor, const std::string value) {
            socket_mutex.lock();
            
            int pos = find_socket(file_descriptor);
            
            if (pos == -1) {
                socket_mutex.unlock();
                
                return -1;
            }
            
            ssize_t     len;
            socket_udp* sock = socketv_udp[pos];
            
            len = ::sendto(sock->file_descriptor(), (const char *)value.c_str(), value.length(), 0, (const struct sockaddr *)sock->address(), sizeof(* sock->address()));
            
            socket_mutex.unlock();
            
            if (len == -1)
                throw socket_exception(std::to_string(len));
            
            return (int)len;
        }
    
        int socket_client_tcp(const std::string host, const int port) {
#if DEBUG_LEVEL
            assert(port >= 0);
#endif
            int file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (file_descriptor == -1)
                throw socket_exception(std::to_string(errno));
            
            struct sockaddr_in addr;

            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            
            if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) == -1)
                throw socket_exception(std::to_string(errno));
            
            // returns 0 for success, -1 otherwise
            if (connect(file_descriptor, (struct sockaddr *)&addr, sizeof(addr))) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            socket_mutex.lock();
            
            socketv_tcp.push_back(new socket_tcp(file_descriptor));
            socket_mutex.unlock();
            
            return file_descriptor;
        }

        int socket_server_tcp(const int port, const int backlog) {
#if DEBUG_LEVEL
            assert(port >= 0);
            assert(backlog >= 1);
#endif
            int file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
            
            if (file_descriptor == -1)
                throw socket_exception(std::to_string(errno));
                    
            int opt = 1;
            
            if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            struct sockaddr_in address;
                
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_family = AF_INET;
            address.sin_port = htons(port);
            
            int address_length = sizeof(address);
            
            if (bind(file_descriptor, (struct sockaddr *)&address, address_length)) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            if (listen(file_descriptor, backlog)) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            socket_tcp* socket = new socket_tcp(file_descriptor);
            
            socket->set_address(address, address_length);
            socket->threads.push_back(std::thread(handle_accept, socket));
            
            socket_mutex.lock();
            
            socketv_tcp.push_back(socket);
            socket_mutex.unlock();
            
            return file_descriptor;
        }
    
        int socket_client_udp(const std::string host, const int port) {
#if DEBUG_LEVEL
            assert(src.length());
            assert(port >= 0);
#endif
            int file_descriptor = ::socket(AF_INET, SOCK_DGRAM, 0);
            
            if (file_descriptor == -1)
                throw socket_exception(std::to_string(errno));
            
            struct sockaddr_in* address = new struct sockaddr_in();
            
            memset(address, 0, sizeof(* address));
            
            address->sin_family = AF_INET;
            address->sin_port = htons(port);
            
            if (inet_pton(AF_INET, host.c_str(), &address->sin_addr) == -1) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            socket_mutex.lock();
            
            socketv_udp.push_back(new socket_udp(file_descriptor, address));
            socket_mutex.unlock();
            
            return file_descriptor;
        }
    
        int socket_server_udp(const int port) {
#if DEBUG_LEVEL
            assert(port >= 0);
#endif
            int file_descriptor = ::socket(AF_INET, SOCK_DGRAM, 0);
            
            if (file_descriptor == -1)
                throw socket_exception(std::to_string(errno));
            
            // server
            struct sockaddr_in addr;
            
            memset(&addr, 0, sizeof(addr));
            
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            
            if (bind(file_descriptor, (const struct sockaddr *)&addr, sizeof(addr))) {
                close(file_descriptor);
                
                throw socket_exception(std::to_string(errno));
            }
            
            // client
            struct sockaddr_in* address = new struct sockaddr_in();
            
            memset(address, 0, sizeof(* address));
            
            socket_mutex.lock();
            
            socketv_udp.push_back(new socket_udp(file_descriptor, address));
            socket_mutex.unlock();
            
            return file_descriptor;
        }

    
        // CONSTRUCTORS

        socket_tcp::socket_tcp(const int file_descriptor) {
            this->file_descriptors.push_back(file_descriptor);
        }
    
        socket_udp::socket_udp(const int file_descriptor, struct sockaddr_in* address) {
            this->_file_descriptor = file_descriptor;
            this->_address = address;
        }

        // MEMBER FUNCTIONS

        void socket_tcp::set_address(const struct sockaddr_in address, const int address_length) {
            this->addresses.push_back(address);
            this->address_length = address_length;
        }
    
        struct sockaddr_in* socket_udp::address() const {
            return this->_address;
        }
    
        int socket_udp::file_descriptor() const {
            return this->_file_descriptor;
        }
    }
}
