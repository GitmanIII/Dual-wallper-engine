#include "monitors.h"
#include "composer.h"
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

// Run a command via fork/execvp — no shell involvement, so shell injection is impossible.
static int run_command(const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        // Child — build null-terminated argv array
        std::vector<const char*> cargs;
        cargs.reserve(args.size() + 1);
        for (const auto& a : args) {
            cargs.push_back(a.c_str());
        }
        cargs.push_back(nullptr);
        execvp(cargs[0], const_cast<char* const*>(cargs.data()));
        // Only reached on error
        perror("execvp");
        _exit(127);
    }
    // Parent — wait for child
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

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
        
        // Apply via gsettings — fork/execvp, no shell involved, no injection risk
        run_command({"gsettings", "set", "org.gnome.desktop.background", "picture-options", "spanned"});
        run_command({"gsettings", "set", "org.gnome.desktop.background", "picture-uri", "file://" + bg_path});
        run_command({"gsettings", "set", "org.gnome.desktop.background", "picture-uri-dark", "file://" + bg_path});
        
        std::cout << "Done!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
