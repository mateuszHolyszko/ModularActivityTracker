#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <vector>

namespace Constants {
    // Muscle list
    const std::vector<std::string> MUSCLES = {
        "Chest", "Back", "Quads", "Hamstrings", "Glutes", 
        "Shoulders", "Biceps", "Triceps", "Abs", "Calves", "Forearms"
    };

    // Measurement list
    const std::vector<std::string> MEASUREMENTS = {
        "arms", "calves", "neck", "thighs", "chest", "waist", "hips", "forearms"
    };
}

#endif