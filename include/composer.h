#pragma once
#include "monitors.h"
#include <vector>
#include <string>

// Composes a multi-monitor image using the given list of wallpapers.
// Maps each wallpaper to a monitor, scales it (aspect-fill), and blits it into a master buffer.
// Returns the path to the composed image.
std::string compose_wallpapers(const std::vector<Monitor>& monitors, const std::vector<std::string>& wallpaper_paths);
