 // USB API: https://github.com/arduino/ArduinoCore-mbed/blob/4.6.0/libraries/USBHID/src/USBMouseKeyboard.h

#include <PluggableUSBHID.h> // Let the GIGA work as a USB input device
#include <USBMouseKeyboard.h> // Send mouse and keyboard controls to the PC
#include <stdio.h> // Read numbers from the ESP32 message
#include <string.h> // Copy, compare and clear the saved keys

// Save incoming data and controls
USBMouseKeyboard computer; // Create the USB mouse and keyboard
String incomingMessage; // Build text from ESP32
bool messageTooLong = false; // Remember if a message should be ignored
bool active = false; // Remember if valid controls are active
bool touchHeld = false; // Remember if the touchpad was already pressed
unsigned long lastReceived = 0; // Save the last valid message 
unsigned long lastScrollTime = 0; // Save when the last scroll
int previousScrollDirection = 0; // Save the last scroll direction
uint8_t previousKeys[6] = {}; // Save up to six held keys
uint8_t previousMouse = 0; // Remember mouse buttons

// Send held keys to the PC
void sendKeys(const uint8_t* keys)
{
  HID_REPORT report = {}; // Start with an empty USB report
  report.length = 9; // Set the report size to 9 bytes
  report.data[0] = REPORT_ID_KEYBOARD; // Mark this as a keyboard report
  memcpy(report.data + 3, keys, 6); // Put the six keys into the report
  computer.send(&report); // Send the report to the PC
}

// Release the keys and mouse
void releaseInputs()
{
  uint8_t keys[6] = {}; // Make six empty key slots
  sendKeys(keys); // Tell the PC no keyboard keys are held
  computer.update(0, 0, 0, 0); // Release mouse buttons with no movement or scrolling
  memset(previousKeys, 0, sizeof(previousKeys)); // Remember that no keyboard keys are held
  previousMouse = 0; 
  lastScrollTime = 0; // Reset scroll timer
  previousScrollDirection = 0; // Clear saved scroll direction
  touchHeld = false; // Clear remembered touchpad
  active = false;
}

// Joystick speed
int mouseStep(int axis) // Mouse movement from stick value
{
  int magnitude = abs(axis); // Joystick position
  if (magnitude <= 70) // Deadzone
  {
    return 0; // Return no mouse movement
  }
  int speed = 1 + (magnitude - 70) * (magnitude - 70) / 18000; // Gradual joystick movement to mouse
  if (axis < 0) // Check for movement in the negative direction
  {
    return -speed; // Keep negative movement direction
  }
  return speed; // Keep positive movement direction
}

// Scrolling
int scrollStep(int axis) // Work out when to send a scroll
{
  int magnitude = abs(axis); // Get how far the stick is from the centre
  if (magnitude <= 160) // Deadzone
  {
    previousScrollDirection = 0;
    return 0; // No scroll
  }
  int direction = 1; // Scroll up
  if (axis < 0) // Check for the negative direction
  {
    direction = -1; // Scroll down
  }
  unsigned long interval = 300 - (magnitude - 160) * 220 / (512 - 160); // Makes scrolling faster
  unsigned long now = millis(); // Read the current time in milliseconds
  if (direction != previousScrollDirection || now - lastScrollTime >= interval) // Gives order for scrolling
  {
    previousScrollDirection = direction; // For scrolling
    lastScrollTime = now; // For scrolling
    return direction; // Update whether to scroll or not
  }
  return 0;
}

void readControls(String incoming) // Turn an ESP32 message into PC controls
{
  // Assign values from esp32
  int lx, ly, rx, ry, l2, r2, charactersRead = 0; // Stick and trigger values plus position
  unsigned int buttons, dpad, misc, touch; // Button and touchpad values

  int fields = sscanf(incoming.c_str(), // Read the numbers from the serial
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%x dpad=%x misc=%x touch=%u%n",
    &lx, &ly, &rx, &ry, &l2, &r2, &buttons, &dpad, &misc, &touch, &charactersRead);

  // Check incoming values
  if (fields != 10 || incoming[charactersRead] != '\0') // Reject missing values
  {
    return;
  }
  // Set range and buttons
  if (lx < -512 || lx > 512 || ly < -512 || ly > 512 || // Reject stick values outside the expected range
      rx < -512 || rx > 512 || ry < -512 || ry > 512)
  {
    return;
  }
  if (l2 < 0 || l2 > 1023 || r2 < 0 || r2 > 1023 || // Reject trigger or button values outside their ranges
      buttons > 0x3ff || dpad > 15 || misc > 255 || touch > 1)
  {
    return;
  }
  lastReceived = millis();
  active = true; // Mark the controls as active

  // Touchpad opens Windows keyboard
  if (touch && !touchHeld) // Run the shortcut when pressed
  {
    uint8_t keys[6] = {}; // Make six empty key slots
    sendKeys(keys); // Release held keys before opening keyboard
    computer.key_code('o', KEY_CTRL | KEY_LOGO); // Open or close Windows keyboard
    memset(previousKeys, 0, sizeof(previousKeys)); // Clear keys after
  }
  touchHeld = touch; // Remember the touchpad state for next time

  // Arrow keys
  uint8_t keys[6] = {}; // Six empty key slots
  size_t count = 0; // Start filling the six key slots
  if ((dpad & 1) && count < 6) keys[count++] = 0x52; // Up Arrow
  if ((dpad & 2) && count < 6) keys[count++] = 0x51; // Down Arrow
  if ((lx > 180 || (dpad & 4)) && count < 6) keys[count++] = 0x4f; // Right Arrow
  if ((lx < -180 || (dpad & 8)) && count < 6) keys[count++] = 0x50; // Left Arrow

  // Cross = Space, Circle = Escape, Square = Enter, Triangle = Backspace
  if ((buttons & 1) && count < 6) keys[count++] = 0x2c; // Spacebar
  if ((buttons & 2) && count < 6) keys[count++] = 0x29; // Escape
  if ((buttons & 4) && count < 6) keys[count++] = 0x28; // Enter 
  if ((buttons & 8) && count < 6) keys[count++] = 0x2a; // Backspace 

  // Update held keys
  if (memcmp(keys, previousKeys, 6) != 0) // Send keyboard keys
  {
    sendKeys(keys); // Send keys to PC
    memcpy(previousKeys, keys, 6); // Remember previous keys
  }

  // Right joystick = Mouse, left joystick up/down = Scroll
  uint8_t mouse = (r2 > 200 ? MOUSE_LEFT : 0) | (l2 > 200 ? MOUSE_RIGHT : 0); // R2 holds left click; L2 holds right click
  int dx = mouseStep(rx); // Get movement
  int dy = mouseStep(ry); // Get movement
  int wheel = scrollStep(ly); // Get the next scroll step
  if (dx || dy || wheel || mouse != previousMouse) // Send if there is movement scrolling or a button change
  {
    computer.update(dx, dy, mouse, wheel); // Send movement to PC
  }
  previousMouse = mouse; // Remember which mouse buttons are held
}

// Start serial connections
void setup()
{
  Serial.begin(115200); // Start serial monitor connection
  Serial1.begin(115200); // Read the ESP32 through Serial
}

// Read data from esp32 and show it in Serial Monitor
void loop()
{
  while (Serial1.available()) // Read while ESP32 data is being sent
  {
    char incoming = (char)Serial1.read(); // Read incoming character
    if (incoming == '\n') // Handle the message when the line ends
    {
      if (!messageTooLong) // Only use messages that fit
      {
        incomingMessage.trim(); // Remove extra spaces around the message
        Serial.println(incomingMessage); // Show the message in Serial Monitor
        if (incomingMessage == "Controller disconnected") // Check for the ESP32 disconnect message
        {
          releaseInputs(); // Release the keyboard keys and mouse buttons
        }
        else
        {
          readControls(incomingMessage); // Read the values and update the PC controls
        }
      }
      incomingMessage = ""; // Clear text
      messageTooLong = false; // Allow the next message to be checked
    }
    else if (incoming != '\r') // Skip character
    {
      if (incomingMessage.length() < 192) // Keep each message within 192 characters
      {
        incomingMessage += incoming; // Add this character to the message
      }
      else
      {
        messageTooLong = true; // Ignore this line when it finishes
      }
    }
  }

  // Release the controls if data stops
  if (active && millis() - lastReceived > 350) // Check for 350 ms
  {
    releaseInputs(); // Release the keyboard keys and mouse buttons
  }
}
