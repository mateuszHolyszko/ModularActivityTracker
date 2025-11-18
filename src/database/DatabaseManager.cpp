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

#pragma region <Queries>
std::vector<std::string> DatabaseManager::getAllUserNames() {
    std::vector<std::string> users;
    
    const char* sql = "SELECT name FROM user;";
    sqlite3_stmt* stmt;
    
    // Prepare the SQL statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return users;
    }
    
    // Execute the statement and process each row
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (name) {
            users.push_back(std::string(name));
        }
    }
    
    // Check if the loop ended due to an error
    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
    }
    
    // Finalize the statement to free resources
    sqlite3_finalize(stmt);
    
    return users;
}
#pragma endregion <Queries>