//
//  mysql_initializer.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "mysql_initializer.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    vector<function_t*> mysqlv;

    void init_mysql() {
        if (mysqlv.size())
            return;
        
        mysqlv.push_back(new ss::function("closeConnection", [](const size_t argc, string* argv) {
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
            
            try {
                middleware::mysql_close((int)num);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return encode(to_string(undefined_t));
        }));
        
        mysqlv.push_back(new ss::function("connect", [](const size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (argc >= 4)
                expect_error("3 argument(s), got " + std::to_string(argc));
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, string_t);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                if (!is_string(argv[i]))
                    type_error(double_t, string_t);
                    //  double != string
                
                argv[i] = decode_raw(argv[i]);
            }
            
            for (size_t i = 0; i < 2; ++i)
                if (argv[i].empty())
                    undefined_error(encode(null()));
            
            int res;
            
            try {
                res = middleware::mysql_connect(argv[0], argv[1], argc == 3 ? argv[2] : "");
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return std::to_string(res);
        }));
        
        mysqlv.push_back(new ss::function("preparedQuery", [](size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(double_t, int_t);
                //  double != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, double_t);
                //  string != double
            
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
            
            string str = decode_raw(argv[1]);
            
            for (size_t i = 0; i < 2; ++i) {
                for (size_t j = 0; j < argc - 1; ++j)
                    swap(argv[j], argv[j + 1]);
                
                --argc;
            }
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, string_t);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                argv[i] = is_string(argv[i]) ? decode_raw(argv[i]) : encode(stod(argv[i]));
            }
            
            sql::ResultSet* res = NULL;
            
            try {
                res = middleware::mysql_prepared_query((int)num, str, argc, argv);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            if (res == NULL)
                return encode(to_string(undefined_t));
                
            size_t ncols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(ncols + 1);
            
            arr.push(std::to_string(ncols));
            
            for (int i = 0; i < ncols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < ncols; ++i) {
                    string value = res->getString(i + 1);
                    arr.push(is_number(value) ? encode(stod(value)) : encode(value));
                }
            }
              
            delete res;
            
            return stringify(arr);
        }));
        
        mysqlv.push_back(new ss::function("preparedUpdate", [](size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, double_t);
                //  string != double
            
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
            
            string str = decode_raw(argv[1]);
            
            for (size_t i = 0; i < 2; ++i) {
                for (size_t j = 0; j < argc - 1; ++j)
                    swap(argv[j], argv[j + 1]);
                
                --argc;
            }
            
            for (size_t i = 0; i < argc; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, string_t);
                    //  array != string
                
                if (argv[i].empty())
                    null_error();
                    
                argv[i] = is_string(argv[i]) ? decode_raw(argv[i]) : encode(stod(argv[i]));
            }
            
            int res;
            
            try {
                res = middleware::mysql_prepared_update((int)num, str, argc, argv);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return std::to_string(res);
        }));
        
        mysqlv.push_back(new ss::function("query", [](const size_t argc, string* argv) {
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
            
            string str = decode_raw(argv[1]);
            
            sql::ResultSet *res = NULL;
            
            try {
                res = middleware::mysql_query((int)num, str);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            if (res == NULL)
                return encode(to_string(undefined_t));
                
            size_t ncols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(ncols + 1);
            
            arr.push(std::to_string(ncols));
            
            for (int i = 0; i < ncols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < ncols; ++i) {
                    string value = res->getString(i + 1);
                    
                    arr.push(is_number(value) ? encode(stod(value)) : encode(value));
                }
            }
              
            delete res;
            
            return stringify(arr);
        }));
        
        mysqlv.push_back(new ss::function("setSchema", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(array_t, int_t);
            
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
            
            int res;
            
            try {
                res = middleware::mysql_set_schema((int)num, argv[1]);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return encode(std::to_string(res));
        }));
        
        mysqlv.push_back(new ss::function("update", [](const size_t argc, string* argv) {
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
            
            int res;
            
            try {
                res = middleware::mysql_update((int)num, argv[1]);
                
            } catch (sql::SQLException& e) {
                throw exception(e.what());
            }
            
            return std::to_string(res);
        }));
        
        mysqlv.shrink_to_fit();
    }

    void deinit_mysql() {
        middleware::mysql_close();
        
        for (size_t i = 0; i < mysqlv.size(); ++i)
            mysqlv[i]->close();
    }

    void set_mysql(command_processor* cp) {
        for (size_t i = 0; i < mysqlv.size(); ++i)
            cp->set_function(mysqlv[i]);
    }
}
