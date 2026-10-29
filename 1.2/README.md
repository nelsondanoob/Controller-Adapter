Version 1.2

This version brings back most of the mouse and keyboard controls from the first finished prototype.

- Right joystick = Mouse
- Left joystick = WASD
- R2 = Left click, hold to drag
- L2 = Right click
- D-pad = Arrow keys
- Cross = Space
- Circle = Escape
- Square = Enter
- Triangle = Backspace
- Touchpad click = Open or close the Windows keyboard

Keys can stay held and repeat. If the controller disconnects or the data stops, the GIGA releases the keys and mouse buttons.

The ESP32 still gets the controller data through Bluetooth and sends it to the GIGA. Both 1.2 files go together because the messages now include the touchpad click. The GIGA still shows the data in Serial Monitor.
