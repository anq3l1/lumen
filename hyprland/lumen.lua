-- Load this file from your existing Hyprland Lua entry point after `hl` is initialized.

hl.window_rule({
    name = "Lumen",
    match = {
        class = "^com.anq3l1.lumen$",
    },
    size = "800 500",
    float = true,
    center = true,
    fullscreen = false,
    pin = false,
})

hl.on("hyprland.start", function()
    hl.exec_cmd(os.getenv("HOME") .. "/.local/bin/lumen --processmanager &")
end)
