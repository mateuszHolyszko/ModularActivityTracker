#include "AppGlobals.h"

namespace AppGlobals {
    std::map<std::string, void*> config;
    std::mutex configMutex;
    
    void cleanup() {
        std::lock_guard<std::mutex> lock(configMutex);
        // Note: This doesn't know the types, so manual cleanup per type needed
        // Or just let OS clean up at program exit
        config.clear();
    }
}

/* Usage example:

#include "AppGlobals.h"
#include <string>

// Set values from any menu/thread
AppGlobals::set<std::string>("CurrentUser", "Mateuszek");
AppGlobals::set<int>("UserAge", 25);
AppGlobals::set<bool>("IsLoggedIn", true);
AppGlobals::set<double>("Balance", 123.45);

// Get values from any menu/thread
std::string user = AppGlobals::get<std::string>("CurrentUser");
int age = AppGlobals::get<int>("UserAge");
bool loggedIn = AppGlobals::get<bool>("IsLoggedIn");

// Get with default
std::string theme = AppGlobals::getOr<std::string>("Theme", "dark");

// Check if exists
if (AppGlobals::has("CurrentUser")) {
    // ...
}

// Remove
AppGlobals::remove<std::string>("CurrentUser");

*/