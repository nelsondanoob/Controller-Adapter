// USB API: https://github.com/arduino/ArduinoCore-mbed/blob/4.6.0/libraries/USBHID/src/USBMouseKeyboard.h

#include <PluggableUSBHID.h> // Let the GIGA work as a USB input device
#include <USBMouseKeyboard.h> // Send mouse and keyboard controls to the PC
#include <stdio.h>
#include <string.h>

// Save incoming data and controls
USBMouseKeyboard computer;
String incomingMessage;
bool messageTooLong = false;
bool active = false;
bool touchHeld = false;
unsigned long lastReceived = 0;
unsigned long lastScrollTime = 0;
int previousScrollDirection = 0;
uint8_t previousKeys[6] = {};
uint8_t previousMouse = 0;

// Send held keys to the PC
void sendKeys(const uint8_t* keys)
{
  HID_REPORT report = {};
  report.length = 9;
  report.data[0] = REPORT_ID_KEYBOARD;
  memcpy(report.data + 3, keys, 6);
  computer.send(&report);
}

// Release the keys and mouse
void releaseInputs()
{
  uint8_t keys[6] = {};
  sendKeys(keys);
  computer.update(0, 0, 0, 0);
  memset(previousKeys, 0, sizeof(previousKeys));
  previousMouse = 0;
  lastScrollTime = 0;
  previousScrollDirection = 0;
  touchHeld = false;
  active = false;
}

// Joystick speed
int mouseStep(int axis)
{
  int magnitude = abs(axis);
  if (magnitude <= 70)
  {
    return 0;
  }
  int speed = 1 + (magnitude - 70) * (magnitude - 70) / 18000;
  if (axis < 0)
  {
    return -speed;
  }
  return speed;
}

// Scrolling
int scrollStep(int axis)
{
  int magnitude = abs(axis);
  if (magnitude <= 160)
  {
    previousScrollDirection = 0;
    return 0;
  }
  int direction = 1;
  if (axis < 0)
  {
    direction = -1;
  }
  unsigned long interval = 300 - (magnitude - 160) * 220 / (512 - 160);
  unsigned long now = millis();
  if (direction != previousScrollDirection || now - lastScrollTime >= interval)
  {
    previousScrollDirection = direction;
    lastScrollTime = now;
    return direction;
  }
  return 0;
}

void readControls(String incoming)
{
  // Assign values from esp32
  int lx, ly, rx, ry, l2, r2, charactersRead = 0;
  unsigned int buttons, dpad, misc, touch;

  int fields = sscanf(incoming.c_str(),
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%x dpad=%x misc=%x touch=%u%n",
    &lx, &ly, &rx, &ry, &l2, &r2, &buttons, &dpad, &misc, &touch, &charactersRead);

  // Check incoming values
  if (fields != 10 || incoming[charactersRead] != '\0')
  {
    return;
  }
  // Set range and buttons
  if (lx < -512 || lx > 512 || ly < -512 || ly > 512 ||
      rx < -512 || rx > 512 || ry < -512 || ry > 512)
  {
    return;
  }
  if (l2 < 0 || l2 > 1023 || r2 < 0 || r2 > 1023 ||
      buttons > 0x3ff || dpad > 15 || misc > 255 || touch > 1)
  {
    return;
  }
  lastReceived = millis();
  active = true;

  // Touchpad = Windows keyboard
  if (touch && !touchHeld)
  {
    uint8_t keys[6] = {};
    sendKeys(keys);
    computer.key_code('o', KEY_CTRL | KEY_LOGO);
    memset(previousKeys, 0, sizeof(previousKeys));
  }
  touchHeld = touch;

  // Left joystick left/right and D-pad = Arrow keys
  uint8_t keys[6] = {};
  size_t count = 0;
  if ((dpad & 1) && count < 6) keys[count++] = 0x52;
  if ((dpad & 2) && count < 6) keys[count++] = 0x51;
  if ((lx > 180 || (dpad & 4)) && count < 6) keys[count++] = 0x4f;
  if ((lx < -180 || (dpad & 8)) && count < 6) keys[count++] = 0x50;

  // Cross = Space, Circle = Escape, Square = Enter, Triangle = Backspace
  if ((buttons & 1) && count < 6) keys[count++] = 0x2c;
  if ((buttons & 2) && count < 6) keys[count++] = 0x29;
  if ((buttons & 4) && count < 6) keys[count++] = 0x28;
  if ((buttons & 8) && count < 6) keys[count++] = 0x2a;

  // Update held keys
  if (memcmp(keys, previousKeys, 6) != 0)
  {
    sendKeys(keys);
    memcpy(previousKeys, keys, 6);
  }

  // Right joystick = Mouse, left joystick up/down = Scroll
  uint8_t mouse = (r2 > 200 ? MOUSE_LEFT : 0) | (l2 > 200 ? MOUSE_RIGHT : 0);
  int dx = mouseStep(rx);
  int dy = mouseStep(ry);
  int wheel = scrollStep(ly);
  if (dx || dy || wheel || mouse != previousMouse)
  {
    computer.update(dx, dy, mouse, wheel);
  }
  previousMouse = mouse;
}

// Start serial connections
void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
}

// Read data from esp32 and show it in Serial Monitor
void loop()
{
  while (Serial1.available())
  {
    char incoming = (char)Serial1.read();
    if (incoming == '\n')
    {
      if (!messageTooLong)
      {
        incomingMessage.trim();
        Serial.println(incomingMessage);
        if (incomingMessage == "Controller disconnected")
        {
          releaseInputs();
        }
        else
        {
          readControls(incomingMessage);
        }
      }
      incomingMessage = "";
      messageTooLong = false;
    }
    else if (incoming != '\r')
    {
      if (incomingMessage.length() < 192)
      {
        incomingMessage += incoming;
      }
      else
      {
        messageTooLong = true;
      }
    }
  }

  // Release the controls if data stops
  if (active && millis() - lastReceived > 350)
  {
    releaseInputs();
  }
}
