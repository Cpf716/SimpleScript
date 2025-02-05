//
//  mysql.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#ifndef mysql_h
#define mysql_h

#include "mysql_connection.h"
#include "mysql_exception.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <map>
#include <mutex>
#include <thread>

using namespace std;

namespace integration {
    class mysql_pool_connection {
        // MEMBER FIELDS
        
        int  _connection;
        bool _is_released = false;
        int  _timeouts = 0;
    public:
        // CONSTRUCTORS
        
        mysql_pool_connection(const int connection);
        
        // MEMBER FUNCTIONS
        
        int connection() const;
        
        int& timeouts();
        
        bool& is_released();
    };

    class mysql_pool {
        // MEMBER FIELDS
        
        vector<mysql_pool_connection*> _connections;
        string                         _host;
        map<string, string>            _options;
        int                            _pool;
        string                         _user;
    public:
        // CONSTRUCTORS
        
        mysql_pool(const int pool, const string host, const string user, map<string, string> options = {{}});
        
        ~mysql_pool();
        
        // MEMBER FUNCTIONS
        
        vector<mysql_pool_connection*>& connections();
        
        string host() const;
        
        string get_option(const string key);
        
        int pool() const;
        
        string user() const;
    };

    // NON-MEMBER FUNCTIONS

    int mysql_close();

    int mysql_close(const int connection);

    int mysql_close_pool(const int pool);

    int mysql_connect(const string host, const string user, map<string, string> options = {{}});

    int mysql_create_pool(const string host, const string user, map<string, string> options = {{}});

    int mysql_get_connection(const int pool);

    sql::ResultSet* mysql_prepared_query(const int connection, const string sql, const size_t valuec, const string* valuev);

    int mysql_prepared_update(const int connection, const string sql, const size_t valuec, const string* valuev);

    sql::ResultSet* mysql_query(const int connection, const string sql);

    int mysql_release(const int connection);

    int mysql_set_schema(const int connection, const string schema);

    int mysql_update(const int connection, const string sql);
}
#endif /* mysql_h */
