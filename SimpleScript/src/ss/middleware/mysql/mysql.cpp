//
//  mysql.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#include "mysql.h"

namespace ss {
    namespace middleware {
        int conc = 0;
        vector<pair<int, sql::Connection*>> conv;
    
        int _find_con(const int key, const size_t beg, const size_t end) {
            if (beg == end)
                return -1;
            
            size_t len = floor((end - beg) / 2);
            
            if (conv[beg + len].first == key)
                return (int)(beg + len);
            
            if (conv[beg + len].first > key)
                return _find_con(key, beg, beg + len);
            
            return _find_con(key, beg + len + 1, end);
        }
    
        int _find_con(const int key) {
            return _find_con(key, 0, conv.size());
        }

        int mysql_connect(const string host, const string usr, const string pwd) {
            sql::Driver *driver = NULL;
            sql::Connection *con = NULL;
            
            try {
                driver = get_driver_instance();
                
                con = driver->connect(host, usr, pwd);
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            conv.push_back(pair<int, sql::Connection*>(conc, con));
            
            return conc++;
        }

        int mysql_close(const int con) {
            int i = _find_con(con);
            
            if (i == -1)
                return -1;
            
            try {
                conv[i].second->close();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete conv[i].second;
            
            conv.erase(conv.begin() + i);
            
            return 0;
        }

        int mysql_close() {
            for (size_t i = 0; i < conv.size(); ++i) {
                try {
                    conv[i].second->close();
                    
                    delete conv[i].second;
                    
                } catch (sql::SQLException &e) {
                    throw e;
                }
                
            }
            
            return 0;
        }

        int mysql_set_schema(const int con, const std::string sch) {
            int i = _find_con(con);
            
            if (i == -1)
                return -1;
            
            try {
                conv[i].second->setSchema(sch);
                
            } catch (sql::SQLException& e) {
                throw e;
            }
            
            return 0;
        }

        sql::ResultSet* mysql_prepared_query(const int con, const string sql, const size_t argc, string* argv) {
            int i = _find_con(con);
            
            if (i == -1)
                return NULL;
            
            sql::PreparedStatement* prep_stmt = NULL;
            sql::ResultSet* res = NULL;
            
            try {
                prep_stmt = conv[i].second->prepareStatement(sql);
                
                for (int j = 0; j < argc; ++j) {
                    if (is_number(argv[j]))
                        prep_stmt->setDouble(j + 1, stod(argv[j]));
                    else
                        prep_stmt->setString(j + 1, argv[j]);
                }
                
                res = prep_stmt->executeQuery();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete prep_stmt;
            return res;
        }

        int mysql_prepared_update(const int con, const string sql, const size_t argc, string* argv) {
            int i = _find_con(con);
            
            if (i == -1)
                return -1;
            
            sql::PreparedStatement* prep_stmt = NULL;

            int res;
            
            try {
                prep_stmt = conv[i].second->prepareStatement(sql);
                
                for (int j = 0; j < argc; ++j) {
                    if (is_number(argv[j]))
                        prep_stmt->setDouble(j + 1, stod(argv[j]));
                    else
                        prep_stmt->setString(j + 1, argv[j]);
                }
                
                res = prep_stmt->executeUpdate();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete prep_stmt;
            return res;
        }

        int mysql_update(const int con, const string sql) {
            int i = _find_con(con);
            
            if (i == -1)
                return -1;
            
            sql::Statement *stmt = NULL;
            
            int res;
            
            try {
                stmt = conv[i].second->createStatement();
                
                res = stmt->executeUpdate(sql);
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete stmt;
            return res;
        }

        sql::ResultSet* mysql_query(const int con, const string sql) {
            int i = _find_con(con);
            
            if (i == -1)
                return NULL;
            
            sql::Statement *stmt = NULL;
            sql::ResultSet *res = NULL;
            
            try {
                stmt = conv[i].second->createStatement();
                
                res = stmt->executeQuery(sql);
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete stmt;
            return res;
        }
    }
}
