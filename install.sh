#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="$HOME/.local/bin"
DATA_DIR="$HOME/.local/share/lumen"
CONFIG_DIR="$HOME/.config/lumen"
HYPR_LUA_DIR="$HOME/.config/hypr/lua"

if ! command -v pacman >/dev/null 2>&1; then
    echo "Error: this installer currently supports Arch Linux (pacman) only." >&2
    exit 1
fi

sudo pacman -S --needed base-devel cmake pkgconf gtkmm-4.0 nlohmann-json

cmake -S "$PROJECT_DIR" -B "$PROJECT_DIR/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$PROJECT_DIR/build" -j"$(nproc)"

install -d "$BIN_DIR" "$DATA_DIR" "$CONFIG_DIR" "$HYPR_LUA_DIR" "$HOME/.local/share/applications"
install -m 755 "$PROJECT_DIR/build/lumen" "$BIN_DIR/lumen"

# Keep existing user CSS intact; install the project default only on first install.
if [[ ! -e "$CONFIG_DIR/style.css" ]]; then
    install -m 644 "$PROJECT_DIR/style.css" "$CONFIG_DIR/style.css"
fi

# Migrate old project-local JSON files once, without overwriting user data.
if [[ ! -e "$DATA_DIR/proc.json" && -f "$PROJECT_DIR/proc.json" ]]; then
    install -m 600 "$PROJECT_DIR/proc.json" "$DATA_DIR/proc.json"
fi
if [[ ! -e "$DATA_DIR/stat.json" && -f "$PROJECT_DIR/stat.json" ]]; then
    install -m 600 "$PROJECT_DIR/stat.json" "$DATA_DIR/stat.json"
fi
[[ -e "$DATA_DIR/proc.json" ]] || printf '{}\n' > "$DATA_DIR/proc.json"
[[ -e "$DATA_DIR/stat.json" ]] || printf '{}\n' > "$DATA_DIR/stat.json"
chmod 600 "$DATA_DIR/proc.json" "$DATA_DIR/stat.json"

install -m 644 "$PROJECT_DIR/hyprland/lumen.lua" "$HYPR_LUA_DIR/lumen.lua"

cat > "$HOME/.local/share/applications/lumen.desktop" <<EOF_DESKTOP
[Desktop Entry]
Type=Application
Name=Lumen
Comment=Application usage tracker
Exec=$BIN_DIR/lumen
Icon=application-x-executable
Terminal=false
Categories=Utility;System;
StartupWMClass=com.anq3l1.lumen
NoDisplay=true
EOF_DESKTOP
chmod 644 "$HOME/.local/share/applications/lumen.desktop"

# If the user's Lua entry point exists, back it up and add the loader once.
HYPR_ENTRY="$HOME/.config/hypr/hyprland.lua"
LOADER='dofile(os.getenv("HOME") .. "/.config/hypr/lua/lumen.lua")'
if [[ -f "$HYPR_ENTRY" ]]; then
    if ! grep -Fq "$LOADER" "$HYPR_ENTRY"; then
        cp -n "$HYPR_ENTRY" "$HYPR_ENTRY.bak.lumen" 2>/dev/null || true
        printf '\n-- Lumen integration\n%s\n' "$LOADER" >> "$HYPR_ENTRY"
        echo "Added Lumen Lua loader to $HYPR_ENTRY (backup: $HYPR_ENTRY.bak.lumen if it did not exist)."
    fi
else
    echo "Lumen Lua config installed at: $HYPR_LUA_DIR/lumen.lua"
    echo "Add this line to your existing Lua entry point after `hl` is initialized:"
    echo "  $LOADER"
fi

# Ensure ~/.local/bin is available to shells and Hyprland-launched commands.
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo "Note: add $BIN_DIR to PATH if the command 'lumen' is not found by Hyprland."
fi

echo
echo "Lumen installed successfully."
echo "Executable: $BIN_DIR/lumen"
echo "Data:       $DATA_DIR/{proc.json,stat.json}"
echo "CSS:        $CONFIG_DIR/style.css"
echo "Hyprland:   $HYPR_LUA_DIR/lumen.lua"
echo "Try:        $BIN_DIR/lumen --list"
