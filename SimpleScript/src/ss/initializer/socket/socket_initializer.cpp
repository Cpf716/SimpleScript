//
//  socket_initializer.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "socket_initializer.h"

namespace ss {
    vector<function_t*> socketv;
    
    void init_socket() {
        if (socketv.size())
            return;
        
        socketv.push_back(new ss::function("closeTcp", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            return encode(middleware::socket_tcp_close((int)num));
        }));
        
        socketv.push_back(new ss::function("closeUdp", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            return encode(middleware::socket_udp_close((int)num));
        }));
        
        socketv.push_back(new ss::function("listen", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (ss::is_array(argv[1]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[1].empty() || is_string(argv[1]))
                type_error(string_t, int_t);
                //  string != int
            
            num = stod(argv[1]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            return std::to_string(middleware::socket_listen(stoi(argv[0]), stoi(argv[1])));
        }));
        
        socketv.push_back(new ss::function("poll", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            vector<int> val = middleware::socket_poll((int)num);
            ostringstream ss;
            
            if (val.size()) {
                for (size_t i = 0; i < val.size() - 1; ++i)
                    ss << val[i] << get_sep();
                
                ss << val[val.size() - 1];
            }
            
            return ss.str();
        }));
        
        socketv.push_back(new ss::function("recv", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            double numv[2];
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(string_t, int_t);
                    //  string != int
                
                numv[i] = stod(argv[i]);
                
                if (!is_int(numv[i]))
                    type_error(double_t, int_t);
                    //  double != int
            }
            
            if (argc == 1)
                numv[1] = 0;
            
            string str = middleware::socket_recv((int)numv[0], (int)numv[1]);
            
            return str.empty() ? encode(to_string(undefined_t)) : encode(str);
        }));
        
        socketv.push_back(new ss::function("recvFrom", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            double numv[2];
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(string_t, int_t);
                    //  string != int
                
                numv[i] = stod(argv[i]);
                
                if (!is_int(numv[i]))
                    type_error(double_t, int_t);
                    //  double != int
            }
            
            if (argc == 1)
                numv[1] = 0;
            
            string str = middleware::socket_recvfrom((int)numv[0], (int)numv[1]);
            
            return str.empty() ? encode(to_string(undefined_t)) : encode(str);
        }));
        
        socketv.push_back(new ss::function("send", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (ss::is_array(argv[1]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(double_t, string_t);
                //  double != string
            
            argv[1] = decode_raw(argv[1]);
            
            int res = middleware::socket_send((int)num, argv[1]);
            
            return std::to_string(res);
        }));
        
        socketv.push_back(new ss::function("sendTo", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (ss::is_array(argv[1]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[1].empty())
                null_error();
            
            if (!is_string(argv[1]))
                type_error(double_t, string_t);
                //  double != string
            
            argv[1] = decode_raw(argv[1]);
            
            int res = middleware::socket_sendto((int)num, argv[1]);
            
            return std::to_string(res);
        }));
        
        socketv.push_back(new ss::function("tcpClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            string str = decode_raw(argv[0]);
            
            if (str.empty())
                undefined_error(encode(null()));
            
            if (ss::is_array(argv[1]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[1].empty() || is_string(argv[1]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[1]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            int fildes = middleware::socket_tcp_client(str, (int)num);
            
            return std::to_string(fildes);
        }));
        
        socketv.push_back(new ss::function("tcpServer", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            for (size_t i = 0; i < 2; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, int_t);
                    //  array != int
                
                if (argv[i].empty() || is_string(argv[i]))
                    type_error(string_t, int_t);
                    //  string != int
                
                double num = stod(argv[i]);
                
                if (!is_int(num))
                    type_error(double_t, int_t);
                    //  double != int
                
                if (num < 0)
                    range_error(std::to_string((int)num));
            }
            
            int fildes = middleware::socket_tcp_server(stoi(argv[0]), stoi(argv[1]));
            
            return std::to_string(fildes);
        }));
        
        socketv.push_back(new ss::function("udpClient", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            string str = decode_raw(argv[0]);
            
            if (str.empty())
                undefined_error(encode(null()));
            
            if (ss::is_array(argv[1]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[1].empty() || is_string(argv[1]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[1]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            int fildes = middleware::socket_udp_client(str, (int)num);
            
            return std::to_string(fildes);
        }));
        
        socketv.push_back(new ss::function("udpServer", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            int fildes = middleware::socket_udp_server((int)num);
            
            return std::to_string(fildes);
        }));
        
        socketv.shrink_to_fit();
    }

    void deinit_socket() {
        middleware::socket_close();
        
        for (size_t i = 0; i < socketv.size(); ++i)
            socketv[i]->close();
    }

    void set_socket(command_processor* cp) {
        for (size_t i = 0; i < socketv.size(); ++i)
            cp->set_function(socketv[i]);
    }
}
