#include "DatabaseManager.hpp"
#include <iostream>

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
#pragma endregion <Queries>

// Explicit template instantiation for common types
template std::vector<std::string> DatabaseManager::executeQuery<std::string>(
    const std::string&, std::function<std::string(sqlite3_stmt*)>);
template std::vector<int> DatabaseManager::executeQuery<int>(
    const std::string&, std::function<int(sqlite3_stmt*)>);