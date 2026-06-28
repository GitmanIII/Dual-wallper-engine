#include "monitors.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <gdk/gdk.h>

// ---------------------------------------------------------------------------
// Runtime display-server detection
// ---------------------------------------------------------------------------
static bool is_wayland() {
    const char* wl = std::getenv("WAYLAND_DISPLAY");
    return wl != nullptr && wl[0] != '\0';
}

// ---------------------------------------------------------------------------
// Wayland backend — uses GDK (GTK3) display / monitor API
// ---------------------------------------------------------------------------
static std::vector<Monitor> get_monitors_via_gdk() {
    std::vector<Monitor> monitors;

    // gdk_display_open(NULL) opens the default display without needing
    // a prior gtk_init / gdk_init call — safe from a standalone binary.
    GdkDisplay* display = gdk_display_open(nullptr);
    if (!display) {
        std::cerr << "Warning [Wayland]: could not open GDK display." << std::endl;
        return monitors;
    }

    int n = gdk_display_get_n_monitors(display);
    for (int i = 0; i < n; ++i) {
        GdkMonitor* mon = gdk_display_get_monitor(display, i);
        if (!mon) continue;

        GdkRectangle rect;
        gdk_monitor_get_geometry(mon, &rect);

        // Build a descriptive name from manufacturer + model if available.
        std::string name;
        const char* manu = gdk_monitor_get_manufacturer(mon);
        const char* model = gdk_monitor_get_model(mon);
        if (manu && manu[0] && model && model[0]) {
            name = std::string(manu) + " " + model;
        } else if (manu && manu[0]) {
            name = manu;
        } else if (model && model[0]) {
            name = model;
        } else {
            name = "Monitor-" + std::to_string(i);
        }

        Monitor m;
        m.name   = std::move(name);
        m.width  = rect.width;
        m.height = rect.height;
        m.x      = rect.x;
        m.y      = rect.y;
        monitors.push_back(std::move(m));
    }

    g_object_unref(display);

    return monitors;
}

// ---------------------------------------------------------------------------
// X11 backend — parses `xrandr` output
// ---------------------------------------------------------------------------
static std::vector<Monitor> get_monitors_via_xrandr() {
    std::vector<Monitor> monitors;

    FILE* pipe = popen("xrandr", "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed! Is xrandr installed?");
    }

    char buffer[1024];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Regular expression to match an active output line from xrandr
    // e.g.  "HDMI-1 connected 1920x1080+1920+0 ..."
    std::regex monitor_regex(
        R"(^(\S+)\s+connected(?:\s+primary)?\s+(\d+)x(\d+)\+(\d+)\+(\d+))");

    std::istringstream stream(result);
    std::string line;
    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, monitor_regex)) {
            Monitor m;
            m.name   = match[1];
            m.width  = std::stoi(match[2]);
            m.height = std::stoi(match[3]);
            m.x      = std::stoi(match[4]);
            m.y      = std::stoi(match[5]);
            monitors.push_back(std::move(m));
        }
    }

    return monitors;
}

// ---------------------------------------------------------------------------
// Public entry point — auto-selects backend based on display server
// ---------------------------------------------------------------------------
std::vector<Monitor> get_active_monitors() {
    if (is_wayland()) {
        auto monitors = get_monitors_via_gdk();
        if (!monitors.empty()) {
            return monitors;
        }
        // GDK returned nothing — fall through to xrandr (unusual on Wayland
        // but handles compositors that provide XWayland with xrandr).
        std::cerr << "Warning: GDK returned no monitors on Wayland, "
                  << "falling back to xrandr." << std::endl;
    }

    return get_monitors_via_xrandr();
}
