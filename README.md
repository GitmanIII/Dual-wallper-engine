# Dual Wallpaper Engine

Native per-monitor wallpapers for Ubuntu and GNOME.

Ubuntu 24.04 (and GNOME in general) natively spans or clones a single wallpaper, but lacks built-in support for assigning distinct wallpapers to different monitors. **Dual Wallpaper Engine** is here to fix that!

This application provides a seamless, native-feeling GTK4 GUI that lets you easily select different wallpapers for each of your connected monitors. It supports any number of screens, dynamically calculates offsets and resolutions, and supports any monitor orientation (horizontal/vertical). 

## Features

- **Native GTK4 Interface:** Designed to look and feel exactly like a built-in GNOME settings panel.
- **Hardware Detection:** Queries your system's hardware to display actual manufacturer and model names instead of just display port codes.
- **Fast C++ Backend:** The heavy lifting (image sizing, cropping, and composition) is done via an ultra-fast, optimized C++ binary.
- **Zero External Image Dependencies:** Statically compiled using `stb_image` libraries, meaning no complex library dependencies are required to run the core engine.
- **Aspect-Fill Scaling:** Automatically centers and crops your wallpapers to fit your monitor's geometry perfectly without distortion.
- **Wayland & X11 Support:** Works flawlessly on default Ubuntu by leveraging GNOME's native `gsettings` desktop configuration to seamlessly map the composed image.

## Installation (Debian / Ubuntu)

The easiest way to install Dual Wallpaper Engine is by using the provided `.deb` package.

1. Download the latest `.deb` release.
2. Install it using `apt` (this automatically installs any missing dependencies like Python 3 and GTK4 libraries):
   ```bash
   sudo apt install ./dual-wallpaper-engine-1.0.0-Linux.deb
   ```
3. Open your app grid and launch **Dual Wallpaper Engine**.

## Building from Source

If you prefer to build the application yourself, ensure you have CMake and a C++17 compiler installed. 

Dependencies: `g++`, `cmake`, `python3`, `python3-gi`, `gir1.2-adw-1`, `libadwaita-1-0`, `x11-xserver-utils` (for `xrandr`).

```bash
# Clone the repository
git clone https://github.com/yourusername/dual-wallpaper-engine.git
cd dual-wallpaper-engine

# Build the project
mkdir build
cd build
cmake ..
make

# Generate the .deb package
make package
```

## Credits

- **Vision & Conception:** Designed by the user to fix a long-standing limitation in the Ubuntu/GNOME desktop environment.
- **Development & Implementation:** Engineered and programmed by the [Gemini CLI](https://github.com/google/gemini-cli) AI Assistant.
