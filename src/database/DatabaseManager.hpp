#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <sqlite3.h>
#include <vector>
#include <string>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    bool initialize();
    bool close();

    // Queries
    std::vector<std::string> getAllUserNames();

private:
    sqlite3* db;
    bool createTables();  // Add this method
};

#endif