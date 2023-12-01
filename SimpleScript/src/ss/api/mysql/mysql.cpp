//
//  mysql.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#include "mysql.h"

namespace ss {
    namespace api {
        bst<pair<size_t, int>>* conbst = NULL;
    
        size_t conc = 1024;
        vector<pair<size_t, sql::Connection*>> conv;

        size_t mysql_connect(const string host, const string usr, const string pwd) {
            sql::Driver *driver = NULL;
            sql::Connection *con = NULL;
            
            try {
                driver = get_driver_instance();
                
                con = driver->connect(host, usr, pwd);
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            conv.push_back(pair<int, sql::Connection*>(conc, con));
            
            if (conbst != NULL)
                conbst->close();
            
            size_t symv[conv.size()];
            
            for (size_t i = 0; i < conv.size(); ++i)
                symv[i] = conv[i].first;
            
            conbst = build(symv, 0, (int)conv.size());
            
            return conc++;
        }

        int mysql_close(const size_t con) {
            if (conbst == NULL)
                return -1;
            
            int i = index_of(conbst, con);
            
            if (i == -1)
                return -1;
            
            try {
                conv[i].second->close();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete conv[i].second;
            
            conv.erase(conv.begin() + i);
            
            conbst->close();
            
            if (conv.size()) {
                size_t symv[conv.size()];
                
                for (size_t i = 0; i < conv.size(); ++i)
                    symv[i] = conv[i].first;
                
                conbst = build(symv, 0, (int)conv.size());
            } else
                conbst = NULL;
            
            return 0;
        }

        int mysql_close() {
            if (conbst != NULL)
                conbst->close();
            
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

        bool mysql_set_schema(const size_t con, const std::string sch) {
            if (conbst == NULL)
                return false;
            
            int i = index_of(conbst, con);
            
            if (i == -1)
                return false;
            
            try {
                conv[i].second->setSchema(sch);
                
            } catch (sql::SQLException& e) {
                throw e;
            }
            
            return true;
        }

        sql::ResultSet* mysql_prepare_query(const size_t con, const string sql, const size_t argc, string* argv) {
            if (conbst == NULL)
                return NULL;
            
            int i = index_of(conbst, con);
            
            if (i == -1)
                return NULL;
            
            sql::PreparedStatement* prep_stmt = NULL;
            sql::ResultSet* res = NULL;
            
            try {
                prep_stmt = conv[i].second->prepareStatement(sql);
                
                for (int j = 0; j < argc; ++j) {
                    try {
                        double num = stod(argv[j]);
                        
                        prep_stmt->setDouble(j + 1, num);
                        
                    } catch (invalid_argument& e) {
                        prep_stmt->setString(j + 1, argv[j]);
                    }
                }
                
                res = prep_stmt->executeQuery();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete prep_stmt;
            
            return res;
        }

        int mysql_prepare_update(const size_t con, const string sql, const size_t argc, string* argv) {
            if (conbst == NULL)
                return -1;
            
            int i = index_of(conbst, con);
            
            if (i == -1)
                return -1;
            
            sql::PreparedStatement* prep_stmt = NULL;

            int res;
            
            try {
                prep_stmt = conv[i].second->prepareStatement(sql);
                
                for (int j = 0; j < argc; ++j) {
                    try {
                        double num = stod(argv[j]);
                        
                        prep_stmt->setDouble(j + 1, num);
                        
                    } catch (invalid_argument& e) {
                        prep_stmt->setString(j + 1, argv[j]);
                    }
                }
                
                res = prep_stmt->executeUpdate();
                
            } catch (sql::SQLException &e) {
                throw e;
            }
            
            delete prep_stmt;
            
            return res;
        }

        int mysql_update(const size_t con, const string sql) {
            if (conbst == NULL)
                return -1;
            
            int i = index_of(conbst, con);
            
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

        sql::ResultSet* mysql_query(const size_t con, const string sql) {
            if (conbst == NULL)
                return NULL;
            
            int i = index_of(conbst, con);
            
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
