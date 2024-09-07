//
//  socket.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/21/23.
//

#define DEBUG_LEVEL 0

#ifndef socket_h
#define socket_h

#include "socket_exception.h"
#include <arpa/inet.h>          //  inet_ptons
#include <cassert>
#include <csignal>              //  signal
#include <netinet/in.h>         //  sockaddr_in
#include <sstream>              //  stringstream
#include <sys/socket.h>         //  socket
#include <thread>
#include <unistd.h>             //  close, read

namespace ss {
    namespace middleware {
        //  TYPEDEF

        class socket_tcp {
            //  MEMBER FIELDS
            
            //  addresses
            std::vector<struct sockaddr_in> add;

            //  address length
            int addlen;
            
            //  parallel socket file descriptors
            std::vector<int> parval;
                
            //  flags
            std::vector<bool> flag;
                
            //  threads
            std::vector<std::thread> thr;
            
            //  file descriptors
            std::vector<int> val;
        public:
            //  CONSTRUCTORS
            
            socket_tcp(const int fildes);
            
            //  MEMBER FUNCTIONS
            
            std::vector<struct sockaddr_in>& get_add();
            
            int& get_addlen();
            
            std::vector<int>& get_parval();
            
            std::vector<bool>& get_flag();
            
            std::vector<std::thread>& get_thr();
            
            std::vector<int>& get_val();
            
            bool is_client() const;
            
            bool is_parallel() const;
            
            bool is_server() const;
            
            void set_addlen(const int addlen);
            
            void set_address(const struct sockaddr_in add, const int addlen);
        };
    
        class socket_udp {
            //  MEMBER FIELDS
            
            struct sockaddr_in* add = NULL;
            
            int val;
        public:
            //  CONSTRUCTORS
            
            socket_udp(const int fildes, struct sockaddr_in* add);
            
            //  MEMBER FUNCTIONS
            
            struct sockaddr_in* get_add() const;
            
            int get_val() const;
        };

        //  NON-MEMBER FUNCTIONS
    
        int socket_close();

        int socket_tcp_close(const int fildes);
    
        int socket_udp_close(const int fildes);

        int socket_listen(const int fildes, const int port);
    
        std::vector<int> socket_poll(const int fildes);

        std::string socket_recv(const int fildes, int timeout = 0);
    
        std::string socket_recvfrom(const int fildes, int timeout = 0);

        int socket_send(const int fildes, const std::string mes);
    
        int socket_sendto(const int fildes, const std::string mes);
    
        int socket_tcp_client(const std::string src, const int port);

        int socket_tcp_server(const int port, const int backlog);
    
        int socket_udp_client(const std::string src, const int port);
    
        int socket_udp_server(const int port);
    }
}

#endif /* socket_h */
