//
//  socket_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "socket_loader.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // CONSTRUCTORS

    socket_loader::socket_loader(const bool flag) {
        if (this->value.size())
            return;
        
        this->flag = flag;
        
        this->value.push_back(new ss::function("closeTCP", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(integration::socket_close_tcp(get_int(argv[0])));
        }));
        
        this->value.push_back(new ss::function("closeUDP", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(integration::socket_close_udp(get_int(argv[0])));
        }));
        
        this->value.push_back(new ss::function("poll", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            vector<int>   cons = integration::socket_poll(get_int(argv[0]));
            ostringstream ss;
            
            if (cons.size()) {
                size_t i;
                for (i = 0; i < cons.size() - 1; ++i)
                    ss << cons[i] << get_sep();
                
                ss << cons[i];
            }
            
            return ss.str();
        }));
        
        this->value.push_back(new ss::function("recv", [](const size_t argc, const string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc > 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            string res = integration::socket_recv(get_int(argv[0]), argc == 2 ? get_int(argv[1]) : 0);
            
            return res.empty() ? null() : encode(res);
        }));
        
        this->value.push_back(new ss::function("recvFrom", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc > 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            string res = integration::socket_recvfrom(get_int(argv[0]), argc == 2 ? get_int(argv[1]) : 0);
            
            return res.empty() ? null() : encode(res);
        }));
        
        this->value.push_back(new ss::function("send", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_send(get_int(argv[0]), decode_raw(get_string(argv[1]))));
        }));
        
        this->value.push_back(new ss::function("sendTo", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_sendto(get_int(argv[0]), decode_raw(get_string(argv[1]))));
        }));
        
        this->value.push_back(new ss::function("TCPClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_client_tcp(decode_raw(get_string(argv[0])), get_int(argv[1])));
        }));
        
        this->value.push_back(new ss::function("TCPServer", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int port_backlog[2];
            
            for (size_t i = 0; i < 2; ++i)
                port_backlog[i] = get_int(argv[i]);
            
            return std::to_string(integration::socket_server_tcp(port_backlog[0], port_backlog[1]));
        }));
        
        this->value.push_back(new ss::function("UDPClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_client_udp(decode_raw(get_string(argv[0])), get_int(argv[1])));
        }));
        
        this->value.push_back(new ss::function("UDPServer", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
                
            return std::to_string(integration::socket_server_udp(get_int(argv[0])));
        }));
        
        this->value.shrink_to_fit();
    }

    socket_loader::~socket_loader() {
        if (this->flag)
            integration::socket_close();
        
        for (size_t i = 0; i < this->value.size(); ++i)
            this->value[i]->close();
    }

    // MEMBER FUNCTIONS

    void socket_loader::bind(command_processor* cp) {
        string key = "localhost";
        
        cp->set_string(key, encode("127.0.0.1"));
        cp->set_read_only(key, true);
        cp->consume(key);
        
        for (size_t i = 0; i < this->value.size(); ++i)
            cp->set_function(this->value[i]);
    }

    // End Enhancement 1-1
}
