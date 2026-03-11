#include "monitors.h"
#include "composer.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image1> [image2 ...]" << std::endl;
        return 1;
    }

    std::vector<std::string> wallpapers;
    for (int i = 1; i < argc; ++i) {
        wallpapers.push_back(argv[i]);
    }

    try {
        std::cout << "Detecting monitors..." << std::endl;
        auto monitors = get_active_monitors();
        if (monitors.empty()) {
            std::cerr << "Error: No monitors detected." << std::endl;
            return 1;
        }

        std::cout << "Found " << monitors.size() << " monitor(s):" << std::endl;
        for (const auto& m : monitors) {
            std::cout << " - " << m.name << " (" << m.width << "x" << m.height 
                      << " at " << m.x << "," << m.y << ")" << std::endl;
        }

        std::cout << "Composing master background image..." << std::endl;
        std::string bg_path = compose_wallpapers(monitors, wallpapers);
        
        std::cout << "Saved composed background to " << bg_path << std::endl;
        
        std::cout << "Applying to GNOME via gsettings..." << std::endl;
        
        // Construct gsettings commands
        std::string cmd_options = "gsettings set org.gnome.desktop.background picture-options 'spanned'";
        std::string cmd_uri = "gsettings set org.gnome.desktop.background picture-uri 'file://" + bg_path + "'";
        std::string cmd_uri_dark = "gsettings set org.gnome.desktop.background picture-uri-dark 'file://" + bg_path + "'";
        
        system(cmd_options.c_str());
        system(cmd_uri.c_str());
        system(cmd_uri_dark.c_str());
        
        std::cout << "Done!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
