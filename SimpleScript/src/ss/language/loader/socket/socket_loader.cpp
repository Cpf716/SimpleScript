//
//  socket_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "socket_loader.h"

namespace ss {
    vector<function_t*> socketv;
    
    void load_socket() {
        if (socketv.size())
            return;
        
        socketv.push_back(new ss::function("closeTCP", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(integration::socket_close_tcp(get_int(argv[0])));
        }));
        
        socketv.push_back(new ss::function("closeUDP", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return encode(integration::socket_close_udp(get_int(argv[0])));
        }));
        
        socketv.push_back(new ss::function("poll", [](const size_t argc, string* argv) {
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
        
        socketv.push_back(new ss::function("recv", [](const size_t argc, const string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc > 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            string res = integration::socket_recv(get_int(argv[0]), argc == 2 ? get_int(argv[1]) : 0);
            
            return res.empty() ? null() : encode(res);
        }));
        
        socketv.push_back(new ss::function("recvFrom", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc > 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            string res = integration::socket_recvfrom(get_int(argv[0]), argc == 2 ? get_int(argv[1]) : 0);
            
            return res.empty() ? null() : encode(res);
        }));
        
        socketv.push_back(new ss::function("send", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_send(get_int(argv[0]), decode_raw(get_string(argv[1]))));
        }));
        
        socketv.push_back(new ss::function("sendTo", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_sendto(get_int(argv[0]), decode_raw(get_string(argv[1]))));
        }));
        
        socketv.push_back(new ss::function("TCPClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_client_tcp(decode_raw(get_string(argv[0])), get_int(argv[1])));
        }));
        
        socketv.push_back(new ss::function("TCPServer", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int port_backlog[2];
            
            for (size_t i = 0; i < 2; ++i)
                port_backlog[i] = get_int(argv[i]);
            
            return std::to_string(integration::socket_server_tcp(port_backlog[0], port_backlog[1]));
        }));
        
        socketv.push_back(new ss::function("UDPClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            return std::to_string(integration::socket_client_udp(decode_raw(get_string(argv[0])), get_int(argv[1])));
        }));
        
        socketv.push_back(new ss::function("UDPServer", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
                
            return std::to_string(integration::socket_server_udp(get_int(argv[0])));
        }));
        
        socketv.shrink_to_fit();
    }

    void set_socket(command_processor* cp) {
        string localhost = "localhost";
        
        cp->set_string(localhost, encode("127.0.0.1"));
        cp->set_read_only(localhost, true);
        cp->consume(localhost);
        
        for (size_t i = 0; i < socketv.size(); ++i)
            cp->set_function(socketv[i]);
    }

    void unload_socket() {
        integration::socket_close();
        
        for (size_t i = 0; i < socketv.size(); ++i)
            socketv[i]->close();
    }
}
