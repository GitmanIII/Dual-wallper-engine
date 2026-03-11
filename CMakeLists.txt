cmake_minimum_required(VERSION 3.10)
project(dual_wallpaper_engine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# C++ Backend Binary
add_executable(dual-wallpaper-engine
    src/main.cpp
    src/monitors.cpp
    src/composer.cpp
    include/stb_impl.cpp
)

# Installation Rules
include(GNUInstallDirs)

# Install C++ Backend
install(TARGETS dual-wallpaper-engine DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install Python GUI to share
install(FILES gui/app.py DESTINATION share/dual-wallpaper-engine)

# Install Launcher Wrapper
install(PROGRAMS gui/dual-wallpaper-engine-gui DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install Desktop Entry
install(FILES com.github.dualwallpaperengine.desktop DESTINATION share/applications)

# CPack Configuration for Debian Package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "dual-wallpaper-engine")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Set distinct wallpapers for each monitor in Ubuntu/GNOME")
set(CPACK_PACKAGE_VENDOR "User")
set(CPACK_PACKAGE_CONTACT "User <user@example.com>")

# Dependencies for the Debian package
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3, python3-gi, gir1.2-adw-1, libadwaita-1-0, x11-xserver-utils")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)
