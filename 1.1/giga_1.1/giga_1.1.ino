// USB API: https://github.com/arduino/ArduinoCore-mbed/blob/4.6.0/libraries/USBHID/src/USBMouseKeyboard.h

#include <PluggableUSBHID.h>
#include <USBMouseKeyboard.h>
#include <stdio.h>

USBMouseKeyboard computer;
unsigned int previousButtons = 0;

void readControls(String incoming)
{
  // Assign Value esp32
  int lx, ly, rx, ry, l2, r2;
  unsigned int buttons, dpad, misc;

  int fields = sscanf(incoming.c_str(),
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%x dpad=%x misc=%x",
    &lx, &ly, &rx, &ry, &l2, &r2, &buttons, &dpad, &misc);

  // Deadzones for the joystick
  if (fields != 9)
  {
    return;
  }
  if (lx < -512 || lx > 512 || ly < -512 || ly > 512)
  {
    return;
  }
  if (rx < -512 || rx > 512 || ry < -512 || ry > 512)
  {
    return;
  }

  // Right joystick = Mouse
  if (rx >= -70 && rx <= 70)
  {
    rx = 0;
  }
  if (ry >= -70 && ry <= 70)
  {
    ry = 0;
  }
  computer.move(rx / 64, ry / 64);

  // Prevent hold keys
  if ((buttons & 1) && !(previousButtons & 1))
  {
    computer.click(MOUSE_LEFT);
  }

  if ((buttons & 4) && !(previousButtons & 4))
  {
    computer.key_code('\b');
  }

  previousButtons = buttons;
}

// Start serial connections
void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial1.setTimeout(20);
}

// Show the ESP32 data and pass it to the mouse and keyboard controls
void loop()
{
  if (Serial1.available())
  {
    String incoming = Serial1.readStringUntil('\n');
    incoming.trim();
    Serial.println(incoming);

    if (incoming == "Controller disconnected")
    {
      previousButtons = 0;
    }
    else
    {
      readControls(incoming);
    }
  }
}
