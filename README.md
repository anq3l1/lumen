# Lumen

**Lumen** is a Linux application launcher and usage tracker written in C++.

**[Версия на русском языке](README_RU.md)**

The goal of Lumen is to provide a fast, customizable launcher with application usage statistics, session tracking and a modern graphical interface.

> 🚧 **Early development** — Lumen is currently a CLI prototype. The GUI and usage tracking are planned for future releases.

## Features

### Currently working

* [x] Scan Linux `.desktop` application files
* [x] Parse application name
* [x] Parse application executable (`Exec=`)
* [x] Parse application icon (`Icon=`)
* [x] Store applications in memory
* [x] List detected applications
* [x] Find an application by name
* [x] Launch applications from the launcher
* [x] Application search

### Planned

* [ ] Application categories
* [ ] Graphical user interface
* [ ] Application icons
* [ ] Keyboard navigation
* [ ] Running application detection
* [ ] Application usage tracking
* [ ] Session tracking
* [ ] SQLite database
* [ ] Daily / weekly / monthly statistics
* [ ] Usage graphs
* [ ] Current session timer
* [ ] Hyprland integration
* [ ] Customizable dashboard
* [ ] Themes and customization
* [ ] Configuration file
* [ ] Background / wallpaper support
* [ ] Clock and system widgets

---

# Roadmap

## v0.1 — Application Discovery

The current development stage.

* [x] Project structure
* [x] Read `/usr/share/applications`
* [x] Find `.desktop` files
* [x] Parse `Name=`
* [x] Parse `Exec=`
* [x] Parse `Icon=`
* [x] Store applications in `std::vector`
* [x] List applications
* [x] Launch applications

---

## v0.2 — Launcher Core

Improve the core before starting the GUI.

* [ ] Separate application loading from listing
* [ ] Create dedicated `Application` class/structure
* [ ] Store `.desktop` file path
* [ ] Handle duplicate application names
* [ ] Implement application search
* [ ] Parse `Exec=` correctly
* [ ] Handle `.desktop` field codes
* [ ] Handle invalid `.desktop` files
* [ ] Search additional application directories
* [ ] Add command-line arguments

Example:

```bash
lumen --list
lumen --open Firefox
lumen --search firefox
```

---

## v0.3 — GUI

Create the first graphical interface.

Planned layout:

```text
┌─────────────────────────────────────────────────────────┐
│                         Lumen                            │
├──────────────────┬──────────────────────────────────────┤
│ Applications     │                                      │
│                  │                                      │
│ Firefox          │                                      │
│ Dolphin          │             Dashboard                │
│ Kitty            │                                      │
│ VS Code          │                                      │
│ Telegram         │                                      │
│ ...              │                                      │
│                  │                                      │
└──────────────────┴──────────────────────────────────────┘
```

* [ ] Choose GUI framework
* [ ] Application list
* [ ] Search bar
* [ ] Application icons
* [ ] Application launching
* [ ] Keyboard navigation
* [ ] Scrollable application list
* [ ] Resizable sidebar
* [ ] Modern dark interface

---

## v0.4 — Usage Tracking

Start collecting application usage data.

* [ ] Detect running applications
* [ ] Track application start
* [ ] Track application exit
* [ ] Track session duration
* [ ] Store sessions
* [ ] SQLite database
* [ ] Application history
* [ ] Current session timer

Example:

```text
Firefox

Today
────────────────────
2h 43m

Sessions
────────────────────
10:42 - 11:17    35m
13:21 - 14:04    43m
18:02 - 19:27    1h 25m
```

---

## v0.5 — Statistics

Turn collected sessions into useful statistics.

* [ ] Today
* [ ] Yesterday
* [ ] Last 7 days
* [ ] Last 30 days
* [ ] Monthly statistics
* [ ] Most used applications
* [ ] Total usage time
* [ ] Usage graphs
* [ ] Application comparison
* [ ] Daily activity timeline

---

## v0.6 — Hyprland Integration

Add deeper integration with the Linux desktop.

* [x] Hyprland IPC integration
* [ ] Detect active window
* [ ] Detect active application
* [ ] Track active time
* [ ] Distinguish running time from active time
* [ ] Workspace information
* [ ] Better application detection

Example:

```text
Running time:
Firefox     4h 21m

Active time:
Firefox     2h 47m
```

---

## v0.7 — Dashboard

Build the customizable right-side dashboard.

Possible widgets:

* [ ] Clock
* [ ] Date
* [ ] Today's activity
* [ ] Weekly activity
* [ ] Most used applications
* [ ] Current application
* [ ] Usage graph
* [ ] System information
* [ ] Custom wallpaper
* [ ] Custom background
* [ ] Widget visibility settings

---

## v0.8 — Customization

Make Lumen configurable.

* [ ] Theme system
* [ ] Accent color
* [ ] Background configuration
* [ ] Sidebar width
* [ ] Widget configuration
* [ ] Application sorting
* [ ] Application filtering
* [ ] JSON configuration
* [ ] User settings

Example configuration:

```json
{
    "theme": "dark",
    "accent": "purple",
    "sidebar_width": 320,
    "show_clock": true,
    "show_statistics": true
}
```

---

## v0.9 — Polish

Prepare the project for a first stable release.

* [ ] Improve performance
* [ ] Improve error handling
* [ ] Clean up architecture
* [ ] Refactor launcher core
* [ ] Improve UI
* [ ] Add animations
* [ ] Add keyboard shortcuts
* [ ] Documentation
* [ ] Screenshots
* [ ] Installation instructions
* [ ] Configuration documentation

---

## v1.0 — First Stable Release

Target:

> A complete Linux launcher with application discovery, launching, usage tracking, statistics and a customizable dashboard.

Planned v1.0 feature set:

* Application launcher
* Application search
* Application icons
* Usage tracking
* Session history
* SQLite database
* Daily / weekly / monthly statistics
* Usage graphs
* Hyprland integration
* Customizable dashboard
* Themes
* Configuration
* Keyboard navigation

---

# Architecture

The project is planned around several separate components:

```text
                 ┌─────────────────┐
                 │   .desktop      │
                 │     files       │
                 └────────┬────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │ Application     │
                 │     Loader      │
                 └────────┬────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │  Application    │
                 │      Model      │
                 └────────┬────────┘
                          │
             ┌────────────┼────────────┐
             ▼            ▼            ▼
        Launcher      Tracker       GUI
             │            │            │
             │            ▼            │
             │         SQLite          │
             │            │            │
             └────────────┼────────────┘
                          ▼
                     Statistics
```

---

# Tech Stack

Current / planned technologies:

* **C++**
* **Linux**
* **STL**
* **`std::filesystem`**
* **`.desktop` files**
* **SQLite**
* **Hyprland IPC**
* **Git / GitHub**
* GUI framework — *TBD*

---

# Building

Currently Lumen is in early development.

Clone the repository:

```bash
git clone https://github.com/YOUR_USERNAME/lumen.git
cd lumen
```

Compile the current prototype:

```bash
g++ src/main.cpp src/handlers.cpp -Iinclude -o lumen
```

List detected applications:

```bash
./lumen --list
```

Open an application:

```bash
./lumen --open kitty
```

---

# Project Status

| Component             | Status |
| --------------------- | ------ |
| Project setup         | 🟢     |
| `.desktop` discovery  | 🟢     |
| Application parsing   | 🟢     |
| Application storage   | 🟢     |
| CLI listing           | 🟢     |
| Application launching | 🟢     |
| Search                | 🔴     |
| GUI                   | 🔴     |
| Usage tracking        | 🔴     |
| SQLite                | 🔴     |
| Statistics            | 🔴     |
| Hyprland integration  | 🔴     |
| Dashboard             | 🔴     |
| Customization         | 🔴     |

**Legend:**

* 🟢 Working
* 🟡 In development
* 🔴 Planned

---

# Why Lumen?

Lumen started as a small C++ project for learning Linux internals, application management and desktop integration.

The project is intended to grow from a simple launcher into a complete desktop activity dashboard.

The main focus is learning by building:

* Linux filesystem APIs
* Process management
* `.desktop` specifications
* Inter-process communication
* Databases
* GUI development
* Software architecture
* C++ project organization

---

# License

License: **TBD**
