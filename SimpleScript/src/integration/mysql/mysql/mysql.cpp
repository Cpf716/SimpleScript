//
//  mysql.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 9/22/23.
//

#include "mysql.h"

namespace integration {
    // NON-MEMBER FIELDS

    const int                           MAX_SIZE = 100;
    const int                           TIMEOUT = 10;

    int                                 connectionc = 0;
    vector<pair<int, sql::Connection*>> connectionv;

    mutex                               mysql_mutex;

    int                                 poolc = 0;
    vector<mysql_pool*>                 poolv;

    // NON-MEMBER FUNCTIONS
    // private:

    int find_connection(const int key, const size_t begin = 0, const size_t end = connectionv.size()) {
        if (begin == end)
            return -1;
        
        size_t len = floor((end - begin) / 2);
        
        if (connectionv[begin + len].first == key)
            return (int)(begin + len);
        
        if (connectionv[begin + len].first > key)
            return find_connection(key, begin, begin + len);
        
        return find_connection(key, begin + len + 1, end);
    }

    int find_pool(const int key, const size_t begin = 0, const size_t end = poolv.size()) {
        if (begin == end)
            return -1;
        
        size_t len = floor((end - begin) / 2);
        
        if (poolv[begin + len]->pool() == key)
            return (int)(begin + len);
        
        if (poolv[begin + len]->pool() > key)
            return find_pool(key, begin, begin + len);
        
        return find_pool(key, begin + len + 1, end);
    }

    pair<int, int> find_pool_connection(int connection) {
        int i, j = 0;
        for (i = 0; i < poolv.size(); ++i) {
            j = 0;
            while (j < poolv[i]->connections().size() && poolv[i]->connections()[j]->connection() != connection)
                ++j;
            
            if (j != poolv[i]->connections().size())
                break;
        }
        
        return { i == poolv.size() ? -1 : i, j };
    }

    void clear_timeout(int connection) {
        auto [i, j] = find_pool_connection(connection);
        
        if (i != -1)
            poolv[i]->connections()[j]->timeouts()++;
    }

    void set_timeout(pair<int, int> position) {
        if (position.first == -1) {
            mysql_mutex.unlock();
            return;
        }
        
        int            connection = poolv[position.first]->connections()[position.second]->connection();
        pair<int, int> timeout = { poolv[position.first]->connections()[position.second]->timeouts()++, stoi(poolv[position.first]->get_option("timeout")) };
        
        mysql_mutex.unlock();
        
        this_thread::sleep_for(chrono::seconds(timeout.second));
        
        mysql_mutex.lock();
        
        position = find_pool_connection(connection);
        
        if (position.first == -1 || timeout.first != poolv[position.first]->connections()[position.second]->timeouts() - 1) {
            mysql_mutex.unlock();
            return;
        }
        
        // close connection
        int pos = find_connection(connection);
        
        try {
            connectionv[pos].second->close();
        } catch (sql::SQLException &e) {
            mysql_mutex.unlock();
            
            throw mysql_exception(e.what());
        }
        
        delete connectionv[pos].second;
        
        connectionv.erase(connectionv.begin() + pos);
        
        delete poolv[position.first]->connections()[position.second];
        
        poolv[position.first]->connections().erase(poolv[position.first]->connections().begin() + position.second);
        
        mysql_mutex.unlock();
        
//        cout << "Pooled connection closed." << endl;
    }

    // public:

    int mysql_close() {
        for (size_t i = 0; i < poolv.size(); ++i)
            mysql_close_pool(poolv[i]->pool());
        
        for (size_t i = 0; i < connectionv.size(); ++i) {
            try {
                connectionv[i].second->close();
                
                delete connectionv[i].second;
            } catch (sql::SQLException &e) {
                throw mysql_exception(e.what());
            }
            
        }
        
        return 0;
    }

    int mysql_close(const int connection) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        try {
            connectionv[pos].second->close();
        } catch (sql::SQLException &e) {
            mysql_mutex.unlock();
            
            throw mysql_exception(e.what());
        }
        
        delete connectionv[pos].second;
        
        connectionv.erase(connectionv.begin() + pos);
        mysql_mutex.unlock();
        
        return 0;
    }

    int mysql_close_pool(const int pool) {
        mysql_mutex.lock();
        
        int pos = find_pool(pool);

        if (pos == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        delete poolv[pos];
        
        poolv.erase(poolv.begin() + pos);
        mysql_mutex.unlock();
        
        return 0;
    }

    int mysql_connect(const string host, const string user, map<string, string> options) {
        mysql_mutex.lock();
        
        sql::Driver     *driver = NULL;
        sql::Connection *con = NULL;
        sql::ConnectOptionsMap connection_properties;
        
        connection_properties["hostName"] = "tcp://" + host;
        connection_properties["userName"] = user;
        
        for (string key: (string[]){ "OPT_READ_TIMEOUT", "OPT_WRITE_TIMEOUT", "port" })
            if (!options[key].empty())
                connection_properties[key] = stoi(options[key]);
        
        for (string key: (string[]){ "password", "schema" })
            if (!options[key].empty())
                connection_properties[key] = options[key];

        try {
            driver = get_driver_instance();

            con = driver->connect(connection_properties);
        } catch (sql::SQLException &e) {
            mysql_mutex.unlock();
            
            throw mysql_exception(e.what());
        }
        
        connectionv.push_back({ connectionc, con });
        
        int connection = connectionc++;
        
        mysql_mutex.unlock();
        
        return connection;
    }

    int mysql_create_pool(const string host, const string user, map<string, string> options) {
        mysql_mutex.lock();
        poolv.push_back(new mysql_pool(poolc, host, user, options));
        
        int pool = poolc++;
        
        mysql_mutex.unlock();
        
        return pool;
    }

    int mysql_get_connection(const int pool) {
        mysql_mutex.lock();
        
        int i = find_pool(pool);
        
        if (i == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        int j = 0;
        while (j < poolv[i]->connections().size() && !poolv[i]->connections()[j]->is_released())
            ++j;
        
        if (j == poolv[i]->connections().size()) {
            if (poolv[i]->connections().size() == stoi(poolv[i]->get_option("max_size"))) {
                mysql_mutex.unlock();
                
                throw mysql_exception(string("pool->size() == max_size"));
            }
            
            sql::Driver            *driver = NULL;
            sql::Connection        *con = NULL;
            sql::ConnectOptionsMap connection_properties;
            
            connection_properties["hostName"] = "tcp://" + poolv[i]->host();
            connection_properties["userName"] = poolv[i]->user();
            
            for (string key: (string[]){ "OPT_READ_TIMEOUT", "OPT_WRITE_TIMEOUT", "port" })
                if (!poolv[i]->get_option(key).empty())
                    connection_properties[key] = stoi(poolv[i]->get_option(key));
            
            for (string key: (string[]){ "password", "schema" })
                if (!poolv[i]->get_option(key).empty())
                    connection_properties[key] = poolv[i]->get_option(key);

            try {
                driver = get_driver_instance();

                con = driver->connect(connection_properties);
            } catch (sql::SQLException &e) {
                mysql_mutex.unlock();
                
                throw mysql_exception(e.what());
            }

            connectionv.push_back({ connectionc, con });
            
            poolv[i]->connections().push_back(new mysql_pool_connection(connectionc));
            
            thread(set_timeout, pair<int, int>(i, poolv[i]->connections().size() - 1)).detach();

            return connectionc++;
        }
        
        poolv[i]->connections()[j]->is_released() = false;
        
        thread(set_timeout, pair<int, int>(i, j)).detach();
                    
        return poolv[i]->connections()[j]->connection();
    }

    sql::ResultSet* mysql_prepared_query(const int connection, const string sql, const size_t valuec, const string* valuev) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return NULL;
        }
        
        clear_timeout(connection);
        
        sql::PreparedStatement* prep_stmt = NULL;
        bool                    is_lock_free = false;
        sql::ResultSet*         res = NULL;
        
        try {
            prep_stmt = connectionv[pos].second->prepareStatement(sql);
            
            mysql_mutex.unlock();
            
            is_lock_free = true;
            
            for (int j = 0; j < valuec; ++j) {
                try {
                    prep_stmt->setDouble(j + 1, stod(valuev[j]));
                } catch (std::exception& e) {
                    prep_stmt->setString(j + 1, valuev[j]);
                }
            }
            
            res = prep_stmt->executeQuery();
        } catch (sql::SQLException &e) {
            if (is_lock_free)
                mysql_mutex.lock();
            
            thread(set_timeout, find_pool_connection(connection)).detach();
            
            throw mysql_exception(e.what());
        }
        
        delete prep_stmt;
        
        mysql_mutex.lock();
        
        thread(set_timeout, find_pool_connection(connection)).detach();
        
        return res;
    }

    int mysql_prepared_update(const int connection, const string sql, const size_t valuec, const string* valuev) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        clear_timeout(connection);
        
        sql::PreparedStatement* prep_stmt = NULL;
        bool                    is_lock_free = false;
        int                     res;
        
        try {
            prep_stmt = connectionv[pos].second->prepareStatement(sql);
            
            mysql_mutex.unlock();
            
            is_lock_free = true;
            
            for (int j = 0; j < valuec; ++j) {
                try {
                    prep_stmt->setDouble(j + 1, stod(valuev[j]));
                } catch (std::exception& e) {
                    prep_stmt->setString(j + 1, valuev[j]);
                }
            }
            
            res = prep_stmt->executeUpdate();
        } catch (sql::SQLException &e) {
            if (is_lock_free)
                mysql_mutex.lock();
            
            thread(set_timeout, find_pool_connection(connection)).detach();
            
            throw mysql_exception(e.what());
        }
        
        delete prep_stmt;
        
        mysql_mutex.lock();
        
        thread(set_timeout, find_pool_connection(connection)).detach();
        
        return res;
    }

    sql::ResultSet* mysql_query(const int connection, const string sql) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return NULL;
        }
        
        clear_timeout(connection);
        
        sql::Statement *stmt = NULL;
        bool           is_lock_free = false;
        sql::ResultSet *res = NULL;
        
        try {
            stmt = connectionv[pos].second->createStatement();
            
            mysql_mutex.unlock();
            
            is_lock_free = true;
            
            res = stmt->executeQuery(sql);
        } catch (sql::SQLException &e) {
            if (is_lock_free)
                mysql_mutex.lock();
            
            thread(set_timeout, find_pool_connection(connection)).detach();
            
            throw mysql_exception(e.what());
        }
        
        delete stmt;
        
        mysql_mutex.lock();
        
        thread(set_timeout, find_pool_connection(connection)).detach();
        
        return res;
    }

    int mysql_release(const int connection) {
        mysql_mutex.lock();
        
        auto [i, j] = find_pool_connection(connection);
        
        if (i == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        poolv[i]->connections()[j]->is_released() = true;
        
        mysql_mutex.unlock();
        
        return 0;
    }

    int mysql_set_schema(const int connection, const string schema) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        try {
            connectionv[pos].second->setSchema(schema);
        } catch (sql::SQLException& e) {
            mysql_mutex.unlock();
            
            throw mysql_exception(e.what());
        }
        
        mysql_mutex.unlock();
        
        return 0;
    }

    int mysql_update(const int connection, const string sql) {
        mysql_mutex.lock();
        
        int pos = find_connection(connection);
        
        if (pos == -1) {
            mysql_mutex.unlock();
            
            return -1;
        }
        
        clear_timeout(connection);
        
        sql::Statement *stmt = NULL;
        bool           is_lock_free = false;
        int            res;
        
        try {
            stmt = connectionv[pos].second->createStatement();
            
            mysql_mutex.unlock();
            
            is_lock_free = true;
            
            res = stmt->executeUpdate(sql);
        } catch (sql::SQLException &e) {
            if (is_lock_free)
                mysql_mutex.lock();
            
            thread(set_timeout, find_pool_connection(connection)).detach();
            
            throw mysql_exception(e.what());
        }
        
        delete stmt;
        
        mysql_mutex.lock();
        
        thread(set_timeout, find_pool_connection(connection)).detach();
        
        return res;
    }

    // CONSTRUCTORS

    mysql_pool::mysql_pool(const int pool, const string host, const string user, map<string, string> options) {
        this->_pool = pool;
        this->_host = host;
        this->_user = user;
        
        for (auto [key, value]: map<string, int>{{ "max_size", MAX_SIZE }, { "timeout", TIMEOUT }}) {
            if (options.find(key) == options.end())
                options[key] = to_string(value);
            else {
                try {
                    if (stoi(options[key]) < 1)
                        options[key] = to_string(value);
                } catch (const std::exception& e) {
                    options[key] = to_string(value);
                }
            }
        }
        
        this->_options = options;
    }

    mysql_pool::~mysql_pool() {
        for (size_t i = 0; i < this->_connections.size(); ++i) {
            this->_connections[i]->timeouts()++;
            
            int pos = find_connection(this->_connections[i]->connection());
            
            try {
                connectionv[pos].second->close();
            } catch (sql::SQLException &e) {
                mysql_mutex.unlock();
                
                throw mysql_exception(e.what());
            }
            
            delete connectionv[pos].second;
            
            connectionv.erase(connectionv.begin() + pos);
        }
    }

    mysql_pool_connection::mysql_pool_connection(const int connection) {
        this->_connection = connection;
    }

    // MEMBER FUNCTIONS

    vector<mysql_pool_connection*>& mysql_pool::connections() {
        return this->_connections;
    };

    string mysql_pool::host() const {
        return this->_host;
    }

    string mysql_pool::get_option(const string key) {
        return this->_options[key];
    }

    int mysql_pool::pool() const {
        return this->_pool;
    }

    string mysql_pool::user() const {
        return this->_user;
    }

    int mysql_pool_connection::connection() const {
        return this->_connection;
    }

    int& mysql_pool_connection::timeouts() {
        return this->_timeouts;
    }

    bool& mysql_pool_connection::is_released() {
        return this->_is_released;
    }
}
