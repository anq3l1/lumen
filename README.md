# Lumen

**Lumen** is an application launcher and usage tracker for Linux, written in C++.

The goal of Lumen is to create a fast, customizable launcher with application usage tracking, session statistics, and a modern graphical interface.

> 🚧 **Early stage of development** — Lumen currently features a CLI prototype, a graphical interface, and application usage tracking.

![Lumen screenshot](https://github.com/anq3l1/lumen/blob/main/screenshots/lumen.png)

## Features

### Implemented

- [x] Discover Linux `.desktop` files
- [x] Retrieve application names
- [x] Retrieve application executable commands (`Exec=`)
- [x] Retrieve application icons (`Icon=`)
- [x] Store applications in memory
- [x] Display discovered applications
- [x] Search applications by name
- [x] Launch applications
- [x] Application search
- [x] Application categories
- [x] Graphical user interface
- [x] Application icons
- [x] Application usage time tracking

### Planned

- [ ] Keyboard navigation
- [ ] Session tracking
- [ ] SQLite database
- [ ] Daily / weekly / monthly statistics
- [ ] Usage charts
- [ ] Current session timer
- [ ] Hyprland integration
- [ ] Customizable panel
- [ ] Themes and customization
- [ ] Configuration file
- [ ] Background / wallpaper support
- [ ] Clock and system widgets

---

# Building

Lumen is currently in an early stage of development.

Clone the repository:

```bash
git clone https://github.com/YOUR_USERNAME/lumen.git
cd lumen
```

Compile and build the current version:

```bash
./install.sh
```

List discovered applications:

```bash
lumen --list
```

Launch an application:

```bash
lumen --open kitty
```

Search for an application:

```bash
lumen --search firefox
```

Launch the graphical interface:

```bash
lumen
```

---

# Why Lumen?

Lumen started as a small C++ project for learning Linux internals, application management, and desktop environment integration.

Over time, the project aims to evolve from a simple launcher into a fully featured desktop activity panel.

The core principle behind the project is **learning by building a real-world project**:

- Linux filesystem APIs
- Process management
- The `.desktop` specification
- Inter-process communication (IPC)
- Databases

---

## Installing the Updated Version

On Arch Linux, run the following commands from the project root:

```bash
chmod +x install.sh
./install.sh
```

The installer will install the dependencies, build the project, and install the executable to `~/.local/bin/lumen`.

Tracker data is stored in `~/.local/share/lumen/proc.json` and `~/.local/share/lumen/stat.json`. The user CSS file is located at `~/.config/lumen/style.css`.

The Hyprland Lua configuration file is installed to `~/.config/hypr/lua/lumen.lua`. If `~/.config/hypr/hyprland.lua` exists, the installer will add a command to load this file. If you use a different Lua entry point, add the following line after initializing `hl`:

```lua
dofile(os.getenv("HOME") .. "/.config/hypr/lua/lumen.lua")
```

The process manager starts on the `hyprland.start` event using the `lumen --processmanager` command. The application list contains one entry per displayed application name; applications with identical names from user and system `.desktop` files are not duplicated.
