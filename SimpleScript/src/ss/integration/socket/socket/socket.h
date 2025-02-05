//
// socket.h
// SimpleScript
//
// Created by Corey Ferguson on 9/21/23.
//

#define DEBUG_LEVEL 0

#ifndef socket_h
#define socket_h

#include "socket_exception.h"
#include <arpa/inet.h>          // inet_ptons
#include <cassert>
#include <csignal>              // signal
#include <mutex>
#include <netinet/in.h>         // sockaddr_in
#include <sstream>              // stringstream
#include <sys/socket.h>         // socket
#include <thread>
#include <unistd.h>             // close, read

namespace ss {
    namespace integration {
        // TYPEDEF

        struct socket_tcp {
            // CONSTRUCTORS
            
            socket_tcp(const int file_descriptor);
            
            // MEMBER FIELDS
            
            int                             address_length;
            std::vector<struct sockaddr_in> addresses;
            std::vector<int>                file_descriptors;
            std::atomic<bool>               flag;
            std::vector<std::thread>        threads;
            
            // MEMBER FUNCTIONS
            
            bool is_client() const;
            
            bool is_server() const;
            
            void set_address(const struct sockaddr_in address, const int address_length);
        };
    
        class socket_udp {
            // MEMBER FIELDS
            
            struct sockaddr_in* _address = NULL;
            int                 _file_descriptor;
        public:
            // CONSTRUCTORS
            
            socket_udp(const int file_descriptor, struct sockaddr_in* address);
            
            // MEMBER FUNCTIONS
            
            struct sockaddr_in* address() const;
            
            int file_descriptor() const;
        };

        // NON-MEMBER FUNCTIONS
    
        int socket_close();

        int socket_close_tcp(const int file_descriptor);
    
        int socket_close_udp(const int file_descriptor);
    
        std::vector<int> socket_poll(const int file_descriptor);

        std::string socket_recv(const int file_descriptor, const int timeout = 0);
    
        std::string socket_recvfrom(const int file_descriptor, const int timeout = 0);

        int socket_send(const int file_descriptor, const std::string value);
    
        int socket_sendto(const int file_descriptor, const std::string value);
    
        int socket_client_tcp(const std::string host, const int port);

        int socket_server_tcp(const int port, const int backlog);
    
        int socket_client_udp(const std::string host, const int port);
    
        int socket_server_udp(const int port);
    }
}

#endif /* socket_h */
