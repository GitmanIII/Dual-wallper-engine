#pragma once
#include <vector>
#include <string>

struct Monitor {
    std::string name;
    int width;
    int height;
    int x;
    int y;
};

// Runs `xrandr` and parses the active monitors and their layout geometries.
std::vector<Monitor> get_active_monitors();
