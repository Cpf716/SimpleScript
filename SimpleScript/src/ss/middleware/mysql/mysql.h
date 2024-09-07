//
//  mysql.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#ifndef mysql_h
#define mysql_h

#include "common.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

namespace ss {
    namespace middleware {
        int mysql_connect(const string host, const string usr, string pwd = "");

        int mysql_close();

        int mysql_close(const int con);

        sql::ResultSet* mysql_prepared_query(const int con, const string sql, const size_t argc, string* argv);

        int mysql_prepared_update(const int con, const string sql, const size_t argc, string* argv);
    
        sql::ResultSet* mysql_query(const int con, const string sql);
    
        int mysql_set_schema(const int con, const std::string sch);
    
        int mysql_update(const int con, const string sql);
    }
}
#endif /* mysql_h */
