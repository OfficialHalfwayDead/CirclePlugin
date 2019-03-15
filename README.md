# CirclePlugin
Custom plugin for BakkesMod that allows you to measure how fast you can dribble in a circle. In other words, how fast you can turn the ball around.

## Installation

1. Copy `CirclePlugin.dll` to `<steam_directory>/steamapps/common/rocketleague/Binaries/Win32/bakkesmod/plugins`
2. Copy `circleplugin.set` to `<steam_directory>/steamapps/common/rocketleague/Binaries/Win32/bakkesmod/plugins/settings`
3. Add `plugin load circleplugin` to `<steam_directory>/steamapps/common/rocketleague/Binaries/Win32/bakkesmod/cfg/plugins.cfg`

## Commands

**circle_start** - Start measurement

**circle_stop** - Abort measurement

**circle_duration** - Set the duration of the measurement (default: 60)

**circle_gui hide|show** - Hide/show GUI

**circle_gui_x [coordinate]** - X origin of GUI from right border of the screen in pixels (default: 500)

**circle_gui_y [coordinate]** - Y origin of GUI from the top of the screen in pixels (default: 200)