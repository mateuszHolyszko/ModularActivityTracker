#include "DatabaseManager.hpp"
#include <iostream>
#include <ctime>

DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open("activity_tracker.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Enable foreign keys and create tables
    return createTables();
}

bool DatabaseManager::createTables() {
    // Enable foreign keys
    const char* enableFK = "PRAGMA foreign_keys = ON;";
    int rc = sqlite3_exec(db, enableFK, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to enable foreign keys: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    #pragma region <Schema>
    // Your schema
    const char* createTablesSQL = R"(
        CREATE TABLE IF NOT EXISTS user (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT,
          weight REAL,
          height REAL,
          gender TEXT,
          style TEXT
        );

        CREATE TABLE IF NOT EXISTS user_history (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user_id INTEGER REFERENCES user(id),
          date DATE,
          weight REAL,
          arms REAL,
          calves REAL,
          neck REAL,
          thighs REAL,
          chest REAL,
          waist REAL,
          hips REAL,
          forearms REAL
        );

        CREATE TABLE IF NOT EXISTS program (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user_id INTEGER REFERENCES user(id),
          name TEXT
        );

        CREATE TABLE IF NOT EXISTS workout (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          program_id INTEGER REFERENCES program(id),
          date DATETIME
        );

        CREATE TABLE IF NOT EXISTS exercise (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT,
          is_bodyweight INTEGER,
          max_reps INTEGER,
          min_reps INTEGER
        );

        CREATE TABLE IF NOT EXISTS program_exercise (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          program_id INTEGER REFERENCES program(id),
          exercise_id INTEGER REFERENCES exercise(id)
        );

        CREATE TABLE IF NOT EXISTS workout_exercise (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          workout_id INTEGER REFERENCES workout(id),
          exercise_id INTEGER REFERENCES exercise(id)
        );

        CREATE TABLE IF NOT EXISTS "set" (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          workout_id INTEGER REFERENCES workout(id),
          exercise_id INTEGER REFERENCES exercise(id),
          reps INTEGER,
          weight REAL,
          set_number INTEGER
        );

        CREATE TABLE IF NOT EXISTS target_muscle (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT UNIQUE
        );

        CREATE TABLE IF NOT EXISTS exercise_target_muscle (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          exercise_id INTEGER REFERENCES exercise(id),
          target_muscle_id INTEGER REFERENCES target_muscle(id)
        );
    )";
    #pragma endregion <Schema>
    char* errorMessage = nullptr;
    rc = sqlite3_exec(db, createTablesSQL, nullptr, nullptr, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }

    std::cout << "Database tables created successfully!" << std::endl;
    return true;
}

bool DatabaseManager::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    return true;
}

// Template method implementation
template<typename T>
std::vector<T> DatabaseManager::executeQuery(const std::string& sql, 
                                           std::function<T(sqlite3_stmt*)> rowMapper) {
    std::vector<T> results;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return results;
    }
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        results.push_back(rowMapper(stmt));
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return results;
}

// Parameterized query method implementation
template<typename T>
std::vector<T> DatabaseManager::executeQuery(const std::string& sql, 
                                           std::function<T(sqlite3_stmt*)> rowMapper,
                                           int parameter) {
    std::vector<T> results;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return results;
    }
    
    // Bind the parameter
    sqlite3_bind_int(stmt, 1, parameter);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        results.push_back(rowMapper(stmt));
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return results;
}

bool DatabaseManager::executeNonQuery(const std::string& sql) {
    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }
    
    return true;
}

#pragma region <Queries>
// Specific query implementations

std::vector<std::string> DatabaseManager::getAllUserNames() {
    return executeQuery<std::string>(
        "SELECT name FROM user;",
        [](sqlite3_stmt* stmt) -> std::string {
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            return name ? std::string(name) : "";
        }
    );
}

std::vector<std::string> DatabaseManager::getProgramNamesForUser(int userId) {
    std::string sql = "SELECT name FROM program WHERE user_id = " + std::to_string(userId) + ";";
    
    return executeQuery<std::string>(
        sql,
        [](sqlite3_stmt* stmt) -> std::string {
            const char* programName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            return programName ? std::string(programName) : "";
        }
    );
}

bool DatabaseManager::addProgramForUser(int userId, const std::string& programName) {
    // Validate inputs
    if (userId <= 0 || programName.empty()) {
        std::cerr << "Invalid parameters for addProgramForUser: userId=" 
                  << userId << ", programName='" << programName << "'" << std::endl;
        return false;
    }
    
    // Prepare SQL statement with parameter binding for security
    std::string sql = "INSERT INTO program (user_id, name) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Bind parameters
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, programName.c_str(), -1, SQLITE_STATIC);
    
    // Execute statement
    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    
    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

int DatabaseManager::getProgramIdByName(int userId, const std::string& programName) {
    // Validate inputs
    if (userId <= 0 || programName.empty()) {
        std::cerr << "Invalid parameters for getProgramIdByName: userId=" 
                  << userId << ", programName='" << programName << "'" << std::endl;
        return -1;
    }
    
    // Prepare SQL statement with parameter binding
    std::string sql = "SELECT id FROM program WHERE user_id = ? AND name = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, programName.c_str(), -1, SQLITE_STATIC);
    
    // Execute query
    rc = sqlite3_step(stmt);
    int programId = -1;
    
    if (rc == SQLITE_ROW) {
        programId = sqlite3_column_int(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return programId;
}

bool DatabaseManager::removeProgramById(int programId) {
    // Validate input
    if (programId <= 0) {
        std::cerr << "Invalid program ID: " << programId << std::endl;
        return false;
    }
    
    // Check if program exists
    std::string checkSql = "SELECT id FROM program WHERE id = ?;";
    sqlite3_stmt* checkStmt;
    
    int rc = sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(checkStmt, 1, programId);
    rc = sqlite3_step(checkStmt);
    
    if (rc != SQLITE_ROW) {
        std::cerr << "Program with ID " << programId << " does not exist" << std::endl;
        sqlite3_finalize(checkStmt);
        return false;
    }
    
    sqlite3_finalize(checkStmt);
    
    // Delete program_exercise entries first (due to foreign key constraints)
    std::string deleteProgramExercisesSql = "DELETE FROM program_exercise WHERE program_id = ?;";
    sqlite3_stmt* deleteProgramExercisesStmt;
    
    rc = sqlite3_prepare_v2(db, deleteProgramExercisesSql.c_str(), -1, &deleteProgramExercisesStmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for deleting program exercises: " 
                  << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(deleteProgramExercisesStmt, 1, programId);
    rc = sqlite3_step(deleteProgramExercisesStmt);
    sqlite3_finalize(deleteProgramExercisesStmt);
    
    // Delete workouts associated with this program
    std::string deleteWorkoutsSql = "DELETE FROM workout WHERE program_id = ?;";
    sqlite3_stmt* deleteWorkoutsStmt;
    
    rc = sqlite3_prepare_v2(db, deleteWorkoutsSql.c_str(), -1, &deleteWorkoutsStmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for deleting workouts: " 
                  << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(deleteWorkoutsStmt, 1, programId);
    rc = sqlite3_step(deleteWorkoutsStmt);
    sqlite3_finalize(deleteWorkoutsStmt);
    
    // Finally delete the program
    std::string deleteProgramSql = "DELETE FROM program WHERE id = ?;";
    sqlite3_stmt* deleteProgramStmt;
    
    rc = sqlite3_prepare_v2(db, deleteProgramSql.c_str(), -1, &deleteProgramStmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for deleting program: " 
                  << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(deleteProgramStmt, 1, programId);
    rc = sqlite3_step(deleteProgramStmt);
    bool success = (rc == SQLITE_DONE);
    
    if (success) {
        std::cout << "Successfully removed program with ID " << programId << std::endl;
    } else {
        std::cerr << "Failed to remove program with ID " << programId 
                  << ": " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(deleteProgramStmt);
    return success;
}

std::vector<int> DatabaseManager::getUserIds() {
    return executeQuery<int>(
        "SELECT id FROM user;",
        [](sqlite3_stmt* stmt) -> int {
            return sqlite3_column_int(stmt, 0);
        }
    );
}

bool DatabaseManager::createUser(const std::string& name, double weight, double height) {
    std::string sql = "INSERT INTO user (name, weight, height) VALUES ('" + 
                     name + "', " + std::to_string(weight) + ", " + 
                     std::to_string(height) + ");";
    return executeNonQuery(sql);
}

// New method implementations
UserMeasurements DatabaseManager::getLatestUserMeasurements(int userId) {
    std::string sql = R"(
        SELECT 
            uh.arms, uh.calves, uh.neck, uh.thighs, uh.chest, 
            uh.waist, uh.hips, uh.forearms, uh.weight, uh.date
        FROM user_history uh
        WHERE uh.user_id = ?
        ORDER BY uh.date DESC
        LIMIT 1;
    )";
    
    auto results = executeQuery<UserMeasurements>(
        sql,
        [](sqlite3_stmt* stmt) -> UserMeasurements {
            UserMeasurements measurements;
            
            // All measurements - if NULL, will remain 0.0 (default)
            measurements.arms = sqlite3_column_double(stmt, 0);
            measurements.calves = sqlite3_column_double(stmt, 1);
            measurements.neck = sqlite3_column_double(stmt, 2);
            measurements.thighs = sqlite3_column_double(stmt, 3);
            measurements.chest = sqlite3_column_double(stmt, 4);
            measurements.waist = sqlite3_column_double(stmt, 5);
            measurements.hips = sqlite3_column_double(stmt, 6);
            measurements.forearms = sqlite3_column_double(stmt, 7);
            measurements.weight = sqlite3_column_double(stmt, 8);
            
            const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            if (date) {
                measurements.date = std::string(date);
            }
            
            return measurements;
        },
        userId
    );
    
    if (results.empty()) {
        return UserMeasurements(); // Returns all zeros
    }
    
    return results[0];
}

bool DatabaseManager::insertUserMeasurements(const std::string& userName, 
                                           const UserMeasurements& measurements) {
    // First, get user ID
    int userId = getUserIdByName(userName);
    if (userId == -1) {
        std::cerr << "User not found: " << userName << std::endl;
        return false;
    }
    
    // Insert measurements
    std::string insertSql = 
        "INSERT INTO user_history (user_id, date, weight, arms, calves, neck, thighs, chest, waist, hips, forearms) "
        "VALUES (" + std::to_string(userId) + ", '" + measurements.date + "', " +
        std::to_string(measurements.weight) + ", " +
        std::to_string(measurements.arms) + ", " +
        std::to_string(measurements.calves) + ", " +
        std::to_string(measurements.neck) + ", " +
        std::to_string(measurements.thighs) + ", " +
        std::to_string(measurements.chest) + ", " +
        std::to_string(measurements.waist) + ", " +
        std::to_string(measurements.hips) + ", " +
        std::to_string(measurements.forearms) + ");";
    
    return executeNonQuery(insertSql);
}

int DatabaseManager::getUserIdByName(const std::string& userName) {
    std::string sql = "SELECT id FROM user WHERE name = '" + userName + "';";
    
    auto userIds = executeQuery<int>(
        sql,
        [](sqlite3_stmt* stmt) -> int {
            return sqlite3_column_int(stmt, 0);
        }
    );
    
    if (userIds.empty()) {
        return -1;
    }
    
    return userIds[0];
}

bool DatabaseManager::getUserHistory(int userId, const std::string& measurement, int weeksToQuery, 
                                   std::vector<std::string>& dates, std::vector<float>& values) {
    // Clear output vectors
    dates.clear();
    values.clear();
    
    // Get the column name for the measurement
    std::string columnName = getColumnNameForMeasurement(measurement);
    if (columnName.empty()) {
        std::cerr << "Invalid measurement: " << measurement << std::endl;
        return false;
    }
    
    // Calculate the date cutoff (current date minus weeksToQuery)
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    tm->tm_mday -= weeksToQuery * 7; // Subtract weeks in days
    std::mktime(tm); // Normalize the date
    
    char cutoffDate[11]; // YYYY-MM-DD + null terminator
    std::strftime(cutoffDate, sizeof(cutoffDate), "%Y-%m-%d", tm);
    
    // Build the SQL query
    std::string sql = "SELECT date, " + columnName + 
                     " FROM user_history WHERE user_id = ? AND date >= ? AND " + 
                     columnName + " IS NOT NULL ORDER BY date ASC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Bind parameters
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, cutoffDate, -1, SQLITE_STATIC);
    
    // Execute query and populate vectors
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Get date
        const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (date) {
            dates.push_back(std::string(date));
        }
        
        // Get measurement value
        float value = sqlite3_column_double(stmt, 1);
        values.push_back(value);
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

std::string DatabaseManager::getColumnNameForMeasurement(const std::string& measurement) {
    // Map measurement names to database column names
    if (measurement == "arms") return "arms";
    else if (measurement == "calves") return "calves";
    else if (measurement == "neck") return "neck";
    else if (measurement == "thighs") return "thighs";
    else if (measurement == "chest") return "chest";
    else if (measurement == "waist") return "waist";
    else if (measurement == "hips") return "hips";
    else if (measurement == "forearms") return "forearms";
    else if (measurement == "weight") return "weight";
    else return ""; // Invalid measurement
}
#pragma endregion <Queries>

// Explicit template instantiation for common types
template std::vector<std::string> DatabaseManager::executeQuery<std::string>(
    const std::string&, std::function<std::string(sqlite3_stmt*)>);
template std::vector<int> DatabaseManager::executeQuery<int>(
    const std::string&, std::function<int(sqlite3_stmt*)>);
template std::vector<UserMeasurements> DatabaseManager::executeQuery<UserMeasurements>(
    const std::string&, std::function<UserMeasurements(sqlite3_stmt*)>);
template std::vector<UserMeasurements> DatabaseManager::executeQuery<UserMeasurements>(
    const std::string&, std::function<UserMeasurements(sqlite3_stmt*)>, int);