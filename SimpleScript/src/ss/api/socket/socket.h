//
//  socket.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/21/23.
//

#ifndef socket_h
#define socket_h

#include <arpa/inet.h>          //  inet_ptons
#include <csignal>              //  signal
#include <iostream>             //  cout
#include <netinet/in.h>         //  sockaddr_in
#include "socket_exception.h"
#include <sstream>              //  stringstream
#include <sys/socket.h>         //  socket
#include <thread>
#include <unistd.h>             //  close, read

namespace ss {
    namespace api {
        //  TYPEDEF

        struct socket {
            //  MEMBER FIELDS
            
            //  addresses
            std::vector<struct sockaddr_in> add;

            //  address length
            int addlen;
            
            //  parallel socket file descriptors
            std::vector<int> parval;
                
            //  flags
            std::vector<bool> flg;
                
            //  threads
            std::vector<std::thread> thr;
            
            //  file descriptors
            std::vector<int> val;
            
            //  CONSTRUCTORS
            
            socket(const int fildes);
            
            //  MEMBER FUNCTIONS
            
            bool is_client() const;
            
            bool is_parallel() const;
            
            bool is_server() const;
            
            void set_address(const struct sockaddr_in add, const int addlen);
        };

        //  NON-MEMBER FUNCTIONS

        void handler_parallel_accept();

        void handler_read();

        void handler_server_accept();

        std::vector<int> socket_accept(const int fildes);

        int socket_client(const std::string src, const int port);

        int socket_close();

        int socket_close(const int fildes);

        void socket_listen(const int fildes, const int port);

        std::string socket_recv(const int fildes);

        int socket_send(const int fildes, const std::string mes);

        int socket_server(const int port, const int backlog);
    }
}

#endif /* socket_h */
