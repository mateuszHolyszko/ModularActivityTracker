#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <sqlite3.h>
#include <vector>
#include <string>
#include <functional>

struct UserMeasurements {
    double arms = 0.0;
    double calves = 0.0;
    double neck = 0.0;
    double thighs = 0.0;
    double chest = 0.0;
    double waist = 0.0;
    double hips = 0.0;
    double forearms = 0.0;
    double weight = 0.0;
    std::string date;
};

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
    
    // Parameterized query method
    template<typename T>
    std::vector<T> executeQuery(const std::string& sql, 
                               std::function<T(sqlite3_stmt*)> rowMapper,
                               int parameter);
    
    bool executeNonQuery(const std::string& sql);

    // Specific queries
    std::vector<std::string> getAllUserNames();
    std::vector<std::string> getProgramNamesForUser(int userId);
    std::vector<int> getUserIds();
    bool createUser(const std::string& name, double weight, double height);
    
    // New methods for user measurements
    UserMeasurements getLatestUserMeasurements(int userId);
    bool insertUserMeasurements(const std::string& userName, const UserMeasurements& measurements);
    int getUserIdByName(const std::string& userName);
    
    // History query method
    bool getUserHistory(int userId, const std::string& measurement, int weeksToQuery, 
                       std::vector<std::string>& dates, std::vector<float>& values);

private:
    sqlite3* db;
    bool createTables();
    
    // Helper method to map measurement names to column names
    std::string getColumnNameForMeasurement(const std::string& measurement);
};

#endif