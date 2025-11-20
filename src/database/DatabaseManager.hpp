#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <sqlite3.h>
#include <vector>
#include <string>
#include <functional>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    bool initialize();
    bool close();

    // Query methods
    template<typename T>
    std::vector<T> executeQuery(const std::string& sql, 
                               std::function<T(sqlite3_stmt*)> rowMapper);
    
    bool executeNonQuery(const std::string& sql);

    // Specific queries
    std::vector<std::string> getAllUserNames();
    std::vector<std::string> getProgramNamesForUser(int userId);
    std::vector<int> getUserIds();
    bool createUser(const std::string& name, double weight, double height);

private:
    sqlite3* db;
    bool createTables();
};

#endif