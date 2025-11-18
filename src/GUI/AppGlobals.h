#pragma once
#include <string>
#include <map>
#include <mutex>

namespace AppGlobals {
    // Internal storage
    extern std::map<std::string, void*> config;
    extern std::mutex configMutex;
    
    // Set a value (stores a copy on heap)
    template<typename T>
    void set(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(configMutex);
        
        // Clean up old value if exists
        std::map<std::string, void*>::iterator it = config.find(key);
        if (it != config.end()) {
            delete static_cast<T*>(it->second);
        }
        
        // Store new value
        config[key] = new T(value);
    }
    
    // Get a value (returns copy)
    template<typename T>
    T get(const std::string& key) {
        std::lock_guard<std::mutex> lock(configMutex);
        
        std::map<std::string, void*>::iterator it = config.find(key);
        if (it == config.end()) {
            // Return default constructed value if not found
            return T();
        }
        
        return *static_cast<T*>(it->second);
    }
    
    // Get with default value
    template<typename T>
    T getOr(const std::string& key, const T& defaultValue) {
        std::lock_guard<std::mutex> lock(configMutex);
        
        std::map<std::string, void*>::iterator it = config.find(key);
        if (it == config.end()) {
            return defaultValue;
        }
        
        return *static_cast<T*>(it->second);
    }
    
    // Check if key exists
    inline bool has(const std::string& key) {
        std::lock_guard<std::mutex> lock(configMutex);
        return config.find(key) != config.end();
    }
    
    // Remove a key
    template<typename T>
    void remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(configMutex);
        
        std::map<std::string, void*>::iterator it = config.find(key);
        if (it != config.end()) {
            delete static_cast<T*>(it->second);
            config.erase(it);
        }
    }
    
    // Clean up all memory (call at program exit)
    void cleanup();
}