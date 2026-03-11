import sys
import os
import subprocess
import re
import json
import gi

gi.require_version('Gtk', '4.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, Gio, GLib, Gdk

# Common Monitor Manufacturer PNP ID mapping
PNP_IDS = {
    "SAM": "Samsung",
    "GSM": "LG",
    "DEL": "Dell",
    "HPQ": "HP",
    "LEN": "Lenovo",
    "ACR": "Acer",
    "ASU": "ASUS",
    "BNQ": "BenQ",
    "AOC": "AOC",
    "PHL": "Philips",
    "EIZ": "Eizo",
    "NEC": "NEC",
    "SNY": "Sony",
    "VIW": "ViewSonic",
    "MSI": "MSI",
    "GIG": "Gigabyte",
}

def get_monitors():
    monitors = []
    try:
        output = subprocess.check_output(['xrandr'], text=True)
        # Regex matching the exact one in the C++ engine
        pattern = re.compile(r"^(\S+)\s+connected(?:\s+primary)?\s+(\d+)x(\d+)\+(\d+)\+(\d+)")
        
        display = Gdk.Display.get_default()
        gdk_monitors = display.get_monitors() if display else None
        
        for line in output.split('\n'):
            match = pattern.match(line)
            if match:
                name = match.group(1)
                width = int(match.group(2))
                height = int(match.group(3))
                
                friendly_name = name
                if gdk_monitors:
                    for i in range(gdk_monitors.get_n_items()):
                        m = gdk_monitors.get_item(i)
                        if m and m.get_connector() == name:
                            manufacturer_id = m.get_manufacturer() or ""
                            # Use mapping if available, otherwise use raw ID
                            manufacturer = PNP_IDS.get(manufacturer_id, manufacturer_id)
                            model = m.get_model() or ""
                            combined = f"{manufacturer} {model}".strip()
                            if combined:
                                friendly_name = combined
                            break
                            
                monitors.append({'name': name, 'friendly_name': friendly_name, 'width': width, 'height': height})
    except Exception as e:
        print(f"Error getting monitors: {e}")
    return monitors

class WallpaperApp(Adw.Application):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)
        self.monitors = get_monitors()
        self.monitor_names = [m['name'] for m in self.monitors]
        
        self.cache_dir = os.path.expanduser("~/.cache/dual-wallpaper-engine")
        self.state_file = os.path.join(self.cache_dir, "state.json")
        self.selected_images = self.load_state()
        self.last_directory = None
        
        for name in self.monitor_names:
            if name not in self.selected_images:
                self.selected_images[name] = None

    def load_state(self):
        if os.path.exists(self.state_file):
            try:
                with open(self.state_file, 'r') as f:
                    return json.load(f)
            except Exception as e:
                print(f"Failed to load state: {e}")
        return {}

    def save_state(self):
        os.makedirs(self.cache_dir, exist_ok=True)
        try:
            with open(self.state_file, 'w') as f:
                json.dump(self.selected_images, f)
        except Exception as e:
            print(f"Failed to save state: {e}")

    def on_activate(self, app):
        self.win = Adw.ApplicationWindow(application=app, title="Dual Wallpaper Engine")
        self.win.set_default_size(500, 400)
        
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.win.set_content(box)

        header = Adw.HeaderBar()
        box.append(header)

        # Content box
        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=12)
        content.set_margin_top(24)
        content.set_margin_bottom(24)
        content.set_margin_start(24)
        content.set_margin_end(24)
        box.append(content)

        # Title
        title_label = Gtk.Label(label="Select Wallpapers")
        title_label.add_css_class("title-1")
        title_label.set_margin_bottom(16)
        content.append(title_label)

        # Monitors list
        listbox = Gtk.ListBox()
        listbox.add_css_class("boxed-list")
        listbox.set_selection_mode(Gtk.SelectionMode.NONE)
        content.append(listbox)

        for m in self.monitors:
            display_name = m.get('friendly_name', m['name'])
            row = Adw.ActionRow(title=f"{display_name} ({m['width']}x{m['height']})")
            
            saved_img = self.selected_images.get(m['name'])
            if saved_img and os.path.exists(saved_img):
                filename = os.path.basename(saved_img)
                row.set_subtitle(f"Selected: {filename}")
            
            button = Gtk.Button(label="Select Image")
            button.set_valign(Gtk.Align.CENTER)
            button.connect("clicked", self.on_select_image_clicked, m['name'], row)
            
            row.add_suffix(button)
            listbox.append(row)

        if not self.monitors:
            error_label = Gtk.Label(label="No monitors detected via xrandr.")
            error_label.add_css_class("error")
            content.append(error_label)

        # Apply button
        apply_btn = Gtk.Button(label="Apply Wallpapers")
        apply_btn.add_css_class("suggested-action")
        apply_btn.add_css_class("pill")
        apply_btn.set_margin_top(24)
        apply_btn.connect("clicked", self.on_apply_clicked)
        content.append(apply_btn)

        self.win.present()

    def on_select_image_clicked(self, button, monitor_name, row):
        print(f"Opening file picker for {monitor_name}...")
        try:
            # We store the dialog as an attribute to prevent garbage collection
            self.active_dialog = Gtk.FileChooserNative.new(
                title=f"Select Wallpaper for {monitor_name}",
                parent=self.win,
                action=Gtk.FileChooserAction.OPEN,
                accept_label="_Open",
                cancel_label="_Cancel"
            )
            
            start_dir = self.last_directory
            if not start_dir:
                for img in self.selected_images.values():
                    if img and os.path.exists(img):
                        start_dir = os.path.dirname(img)
                        break
            
            if start_dir and os.path.exists(start_dir):
                try:
                    self.active_dialog.set_current_folder(Gio.File.new_for_path(start_dir))
                except Exception as e:
                    print(f"Could not set initial folder: {e}")
            
            filter_img = Gtk.FileFilter()
            filter_img.set_name("Images")
            filter_img.add_mime_type("image/jpeg")
            filter_img.add_mime_type("image/png")
            self.active_dialog.add_filter(filter_img)

            self.active_dialog.connect("response", self.on_file_selected, monitor_name, row)
            self.active_dialog.show()
        except Exception as e:
            print(f"Error creating dialog: {e}")
            self.show_dialog("Error", f"Failed to open file picker: {e}")

    def on_file_selected(self, dialog, response, monitor_name, row):
        try:
            if response == Gtk.ResponseType.ACCEPT:
                file = dialog.get_file()
                if file:
                    path = file.get_path()
                    self.selected_images[monitor_name] = path
                    self.last_directory = os.path.dirname(path)
                    filename = os.path.basename(path)
                    row.set_subtitle(f"Selected: {filename}")
                    print(f"Selected {path} for {monitor_name}")
            
            # Clear the reference and unref
            self.active_dialog = None
        except Exception as e:
            print(f"Error in file selection: {e}")

    def on_apply_clicked(self, button):
        images_to_pass = []
        for name in self.monitor_names:
            img = self.selected_images.get(name)
            if not img or not os.path.exists(img):
                self.show_dialog("Error", f"Please select an image for {name}")
                return
            images_to_pass.append(img)

        # Robust binary search
        bin_name = "dual-wallpaper-engine"
        base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        
        # Candidate 1: Local build folder (development)
        # Candidate 2: Same folder as script (portable)
        # Candidate 3: System PATH (installed)
        candidates = [
            os.path.join(base_dir, "build", bin_name),
            os.path.join(os.path.dirname(os.path.abspath(__file__)), bin_name),
            bin_name # Shell search
        ]
        
        bin_path = None
        for c in candidates:
            if "/" in c: # Absolute path
                if os.path.exists(c):
                    bin_path = c
                    break
            else: # Command name only, check if it exists in PATH
                if subprocess.call(["which", c], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) == 0:
                    bin_path = c
                    break

        if not bin_path:
            self.show_dialog("Error", f"Could not find the engine binary. Have you built/installed it?")
            return

        cmd = [bin_path] + images_to_pass
        
        try:
            self.save_state()
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            self.quit()
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if e.output else str(e)
            self.show_dialog("Error", f"Engine failed to apply wallpapers.\n{output}")
        except Exception as e:
            self.show_dialog("Error", f"Unexpected error:\n{e}")

    def show_dialog(self, title, message):
        dialog = Adw.MessageDialog(parent=self.win, heading=title, body=message)
        dialog.add_response("ok", "OK")
        dialog.present()

if __name__ == '__main__':
    app = WallpaperApp(application_id="com.github.dualwallpaperengine")
    app.run(sys.argv)
