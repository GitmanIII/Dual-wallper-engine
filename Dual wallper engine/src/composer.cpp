#include "composer.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include "stb/stb_image.h"
#include "stb/stb_image_resize2.h"
#include "stb/stb_image_write.h"

std::string get_cache_dir() {
    const char* homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    std::string path = std::string(homedir) + "/.cache/dual-wallpaper-engine";
    
    // Create directory if it doesn't exist
    struct stat st = {0};
    if (stat(path.c_str(), &st) == -1) {
        mkdir(path.c_str(), 0700);
    }
    return path;
}

std::string compose_wallpapers(const std::vector<Monitor>& monitors, const std::vector<std::string>& wallpaper_paths) {
    if (monitors.empty()) {
        throw std::runtime_error("No monitors found.");
    }
    
    int master_w = 0;
    int master_h = 0;
    for (const auto& m : monitors) {
        if (m.x + m.width > master_w) master_w = m.x + m.width;
        if (m.y + m.height > master_h) master_h = m.y + m.height;
    }
    
    std::cout << "Calculated desktop bounding box: " << master_w << "x" << master_h << std::endl;
    
    int channels = 3;
    std::vector<unsigned char> master_buffer(master_w * master_h * channels, 0); // Black background
    
    for (size_t i = 0; i < monitors.size(); i++) {
        const auto& m = monitors[i];
        std::string img_path = wallpaper_paths[i % wallpaper_paths.size()]; // Loop images if fewer than monitors
        
        int img_w, img_h, img_c;
        unsigned char* img_data = stbi_load(img_path.c_str(), &img_w, &img_h, &img_c, channels);
        
        if (!img_data) {
            std::cerr << "Failed to load image: " << img_path << std::endl;
            continue;
        }
        
        // Calculate aspect-fill scale
        float scale_w = (float)m.width / img_w;
        float scale_h = (float)m.height / img_h;
        float scale = std::max(scale_w, scale_h);
        
        int new_w = (int)(img_w * scale);
        int new_h = (int)(img_h * scale);
        
        std::vector<unsigned char> resized_buffer(new_w * new_h * channels);
        stbir_resize_uint8_linear(img_data, img_w, img_h, 0,
                                  resized_buffer.data(), new_w, new_h, 0,
                                  (stbir_pixel_layout)channels);
        stbi_image_free(img_data);
        
        // Center crop
        int crop_x = (new_w - m.width) / 2;
        int crop_y = (new_h - m.height) / 2;
        
        // Blit to master buffer
        for (int y = 0; y < m.height; y++) {
            for (int x = 0; x < m.width; x++) {
                int src_y = crop_y + y;
                int src_x = crop_x + x;
                
                int master_idx = ((m.y + y) * master_w + (m.x + x)) * channels;
                int src_idx = (src_y * new_w + src_x) * channels;
                
                for (int c = 0; c < channels; c++) {
                    master_buffer[master_idx + c] = resized_buffer[src_idx + c];
                }
            }
        }
        std::cout << "Blitted " << img_path << " to monitor " << m.name << " at " << m.x << "," << m.y << std::endl;
    }
    
    std::string out_path = get_cache_dir() + "/background.jpg";
    if (!stbi_write_jpg(out_path.c_str(), master_w, master_h, channels, master_buffer.data(), 90)) {
        throw std::runtime_error("Failed to write composite background image!");
    }
    
    return out_path;
}
