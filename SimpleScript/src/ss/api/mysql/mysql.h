//
//  mysql.h
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#ifndef mysql_h
#define mysql_h

#include "bst.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "mysql_connection.h"

using namespace std;

namespace ss {
    namespace api {
        size_t mysql_connect(const string host, const string usr, const string pwd);

        int mysql_close();

        int mysql_close(const size_t con);

        bool mysql_set_schema(const size_t con, const std::string sch);

        sql::ResultSet* mysql_prepare_query(const size_t con, const string sql, const size_t argc, string* argv);

        int mysql_prepare_update(const size_t con, const string sql, const size_t argc, string* argv);

        int mysql_update(const size_t con, const string sql);

        sql::ResultSet* mysql_query(const size_t con, const string sql);
    }
}
#endif /* mysql_h */
