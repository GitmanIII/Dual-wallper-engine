#include "monitors.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <regex>
#include <stdexcept>

std::vector<Monitor> get_active_monitors() {
    std::vector<Monitor> monitors;
    
    FILE* pipe = popen("xrandr", "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed! Is xrandr installed?");
    }
    
    char buffer[1024];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Regular expression to match an active output line from xrandr
    // e.g., "HDMI-1 connected 1920x1080+1920+0 (normal left inverted right x axis y axis) 527mm x 296mm"
    std::regex monitor_regex(R"(^(\S+)\s+connected(?:\s+primary)?\s+(\d+)x(\d+)\+(\d+)\+(\d+))");
    
    std::istringstream stream(result);
    std::string line;
    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, monitor_regex)) {
            Monitor m;
            m.name = match[1];
            m.width = std::stoi(match[2]);
            m.height = std::stoi(match[3]);
            m.x = std::stoi(match[4]);
            m.y = std::stoi(match[5]);
            monitors.push_back(m);
        }
    }
    
    return monitors;
}
