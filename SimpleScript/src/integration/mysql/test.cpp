//
//  test.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 12/16/24.
//

/*
 To Do:
 > query timeouts?
 */

#include "mysql.h"

using namespace integration;

void handle_signal(int signum) {
    mysql_close();
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    int pool = mysql_create_pool("localhost", "root", {{ "password", "root" }, { "port", to_string(3306) }, { "schema", "musicLibrary" }, { "timeout", to_string(1) }}),
        conn;
    
    conn = mysql_get_connection(pool);
    
    mysql_release(conn);

//    conn = mysql_get_connection(pool);
    
    for (size_t i = 0; i < 10; ++i)
        conn = mysql_get_connection(pool);
    
    this_thread::sleep_for(chrono::seconds(1));
    
    mysql_set_schema(conn, "musicLibrary");
    
    sql::ResultSet* result = NULL;
    
    try {
        result = mysql_query(conn, "select * from ongs");
    } catch (mysql_exception& e) {
        cout << e.what() << endl;
    }
//
    delete result;
    
//    while (true);
    
    mysql_close_pool(pool);
}
