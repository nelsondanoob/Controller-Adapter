# Controller Adapter

Use a PS5 DualSense controller as a mouse and keyboard for a Windows PC. The ESP32 receives the controller's Bluetooth input and sends it to the Arduino GIGA R1 WiFi. The GIGA turns those values into mouse and keyboard commands through USB.

Use **[version 1.3](1.3/)** for this setup.

## What you need

- A Windows PC with two USB ports, or a USB hub.
- A charged PS5 DualSense controller.
- An **original ESP32 board**, such as an ESP32-WROOM-32, with GPIO17 available. ESP32-C3 and ESP32-S3 are not compatible with this controller.
- An **Arduino GIGA R1 WiFi**.
- Two USB data cables: USB-C for the GIGA and the correct cable for your ESP32.
- Two jumper wires.
- [Arduino IDE 2](https://www.arduino.cc/en/software).

## Setup

### 1. Download the project

### 2. Install the board packages

In Arduino IDE, open **File > Preferences > Additional Boards Manager URLs**. Add both URLs:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
```

Open **Boards Manager** and install:

- **esp32** by Espressif Systems.
- **esp32_bluepad32**, version **4.1.0**.
- **Arduino Mbed OS GIGA Boards**, version **4.6.0**.

`USBMouseKeyboard` is included in the GIGA package.

### 3. Upload to the ESP32

1. Connect the ESP32 to the PC with its USB data cable.
2. Open [`1.3/esp32_1.3/esp32_1.3.ino`](1.3/esp32_1.3/esp32_1.3.ino).
3. Select **ESP32 Dev Module** from the **ESP32 Arduino + Bluepad32** board group.
4. Select the ESP32's port under **Tools > Port**, then click **Upload**.

### 4. Upload to the GIGA

1. Connect the GIGA to the PC through its **USB-C** port.
2. Open [`1.3/giga_1.3/giga_1.3.ino`](1.3/giga_1.3/giga_1.3.ino).
3. Select **Arduino Giga R1** from **Arduino Mbed OS GIGA Boards**. Choose **Main Core (M7)** if the option is shown.
4. Select the GIGA's port, then click **Upload**.

### 5. Connect the boards

Unplug both boards, then connect these two wires:

| ESP32 | GIGA |
| --- | --- |
| **GPIO17** | **D0 / RX** |
| **GND** | **GND** |

Reconnect both boards through USB to power them. The GIGA's **USB-C cable must connect to the PC** to send mouse and keyboard input.

### 6. Pair the controller

1. Power the ESP32 and GIGA, with GIGA connecting directly to your PC.
2. Hold **Create + PS** until the light bar flashes. Create is the small button to the left of the touchpad.
3. Let the controller connect to the **ESP32**.
4. If everything works as it should, you should be able to use the controls below.

## Controls

| Controller input | PC action |
| --- | --- |
| Right stick | Move the mouse pointer |
| Left stick left/right | Left/right arrow keys |
| Left stick up/down | Scroll vertically; push further to scroll faster |
| D-pad | Arrow keys |
| R2 | Left mouse button; hold to drag |
| L2 | Right mouse button |
| Cross / X | Space |
| Circle | Escape |
| Square | Enter |
| Triangle | Backspace |
| Touchpad click | Open or close the Windows on-screen keyboard |

Use the right stick and R2 to click letters on the on-screen keyboard.

## About the project

This began as a UTS assignment exploring a universal controller adapter. Version 1.3 works as a Windows mouse and keyboard. Older folders show the development stages.

### Credits

- Bluetooth input is based on [Ricardo Quesada's Bluepad32 Controller example](https://github.com/ricardoquesada/esp32-arduino-lib-builder/blob/4.1.0/bluepad32_files/libraries/Bluepad32_ESP32/examples/Controller/Controller.ino).
- USB output uses [ArduinoCore-mbed 4.6.0](https://github.com/arduino/ArduinoCore-mbed/tree/4.6.0).

#### Video tutorials

Concepts used in this project:

- [Bluepad32 for Arduino IDE — Ricardo Quesada](https://www.youtube.com/watch?v=0jnY-XXiD8Q): setting up Bluepad32 in Arduino IDE.
- [Arduino Tutorial 11 — Paul McWhorter](https://www.youtube.com/watch?v=b5kndEtAKl8): printing values to the Serial Monitor.
- [Pointers in C++ — The Cherno](https://www.youtube.com/watch?v=DTxHyVn0ODg): understanding pointers such as `ControllerPtr`.
- [The Arrow Operator in C++ — The Cherno](https://www.youtube.com/watch?v=4p3grlSpWYA): understanding calls such as `pad->axisX()`.
