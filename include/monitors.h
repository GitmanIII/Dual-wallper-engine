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

// Detects the display server at runtime (Wayland vs X11).
// On Wayland uses the GDK display API (via GTK3).
// On X11 (fallback) runs `xrandr` and parses active monitors.
std::vector<Monitor> get_active_monitors();
