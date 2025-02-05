//
//  mysql_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "mysql_loader.h"

namespace ss {
    // NON-MEMBER FIELDS

    void load_mysql(command_processor* cp) {
        cp->set_function(new ss::function("closeConnection", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return std::to_string(::integration::mysql_close(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("closePool", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
        
            return std::to_string(::integration::mysql_close_pool(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("connect", [](const size_t argc, const string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (argc >= 4)
                expect_error("3 argument(s), got " + std::to_string(argc));
            
            string host_user[2];
            
            for (size_t i = 0; i < 2; ++i)
                host_user[i] = decode_raw(get_string(argv[i]));
            
            map<string, string> options;
            
            if (argc == 3) {
                string destination[argv[2].length() + 1];
                size_t len = get_dictionary(destination, argv[2]);
                
                for (size_t j = 0; j < len / 2; ++j)
                    options[decode_raw(destination[j * 2])] = decode_raw(destination[j * 2 + 1]);
            }
            
            return std::to_string(::integration::mysql_connect(host_user[0], host_user[1], options));
        }));
        
        cp->set_function(new ss::function("createPool", [](const size_t argc, const string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (argc >= 4)
                expect_error("3 argument(s), got " + std::to_string(argc));
            
            string host_user[2];
            
            for (size_t i = 0; i < 2; ++i)
                host_user[i] = decode_raw(get_string(argv[i]));
            
            map<string, string> options;
            
            if (argc == 3) {
                string destination[argv[2].length() + 1];
                size_t len = get_dictionary(destination, argv[2]);
                
                for (size_t i = 0; i < len / 2; ++i)
                    options[decode_raw(destination[i * 2])] = decode_raw(destination[i * 2 + 1]);
            }
                
            return std::to_string(::integration::mysql_create_pool(host_user[0], host_user[1], options));
        }));
        
        cp->set_function(new ss::function("getConnection", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return std::to_string(::integration::mysql_get_connection(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("preparedQuery", [](size_t argc, const string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int    connection = get_int(argv[0]);
            string sql = decode_raw(get_string(argv[1]));
            size_t valuec = argc - 2;
            string valuev[valuec];
            
            for (size_t i = 0, j = 2; i < valuec; ++i, ++j) {
                if (ss::is_array(argv[j]))
                    type_error(array_t, item_t);
                
                if (argv[j].empty())
                    null_error();
                    
                valuev[i] = is_string(argv[j]) ? decode_raw(argv[j]) :
                    encode(stod(argv[j]));
            }
            
            sql::ResultSet*   res = ::integration::mysql_prepared_query(connection, sql, argc, argv);
            
            if (res == NULL)
                return null();
            
            size_t            cols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(cols + 1);
            
            arr.push(std::to_string(cols));
            
            for (int i = 0; i < cols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < cols; ++i) {
                    string item = res->getString(i + 1);
                    
                    try {
                        arr.push(encode(stod(item)));
                    } catch (std::exception& e) {
                        arr.push(encode(item));
                    }
                }
            }
              
            delete res;
            return stringify(arr);
        }));
        
        cp->set_function(new ss::function("preparedUpdate", [](size_t argc, const string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int    connection = get_int(argv[0]);
            string sql = decode_raw(get_string(argv[1]));
            size_t valuec = argc - 2;
            string valuev[valuec];
            
            for (size_t i = 0, j = 2; i < valuec; ++i, ++j) {
                if (ss::is_array(argv[j]))
                    type_error(array_t, item_t);
                
                if (argv[j].empty())
                    null_error();
                    
                valuev[i] = is_string(argv[j]) ? decode_raw(argv[j]) :
                    encode(stod(argv[j]));
            }
            
            return std::to_string(::integration::mysql_prepared_update(connection, sql, valuec, valuev));
        }));
        
        cp->set_function(new ss::function("query", [](const size_t argc, const string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int               connection = get_int(argv[0]);
            string            sql = decode_raw(get_string(argv[1]));
            sql::ResultSet    *res = ::integration::mysql_query(connection, sql);
            
            if (res == NULL)
                return null();
            
            int               cols = res->getMetaData()->getColumnCount();
            ss::array<string> arr = ss::array<string>(cols + 1);
            
            arr.push(std::to_string(cols));
            
            for (int i = 0; i < cols; ++i)
                arr.push(encode(res->getMetaData()-> getColumnName(i + 1)));
            
            while (res->next()) {
                for (int i = 0; i < cols; ++i) {
                    string item = res->getString(i + 1);
                    
                    try {
                        arr.push(encode(stod(item)));
                    } catch (std::exception& e) {
                        arr.push(encode(item));
                    }
                }
            }
              
            delete res;
            return stringify(arr);
        }));
        
        cp->set_function(new ss::function("release", [](const size_t argc, const string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            return std::to_string(::integration::mysql_release(get_int(argv[0])));
        }));
        
        cp->set_function(new ss::function("setSchema", [](const size_t argc, const string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int    connection = get_int(argv[0]);
            string schema = decode_raw(get_string(argv[1]));
            
            return std::to_string(::integration::mysql_set_schema(connection, schema));
        }));
        
        cp->set_function(new ss::function("update", [](const size_t argc, const string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            int    connection = get_int(argv[0]);
            string sql = decode_raw(get_string(argv[1]));
            
            return std::to_string(::integration::mysql_update(connection, sql));
        }));
    }

    void unload_mysql() {
        ::integration::mysql_close();
    }
}
