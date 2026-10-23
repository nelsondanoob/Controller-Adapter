// Source: https://github.com/ricardoquesada/esp32-arduino-lib-builder/blob/4.1.0/bluepad32_files/libraries/Bluepad32_ESP32/examples/Controller/Controller.ino

#include <Bluepad32.h> // Connect to the controller through Bluetooth

// Store the controller and touchpad
ControllerPtr controller = nullptr; // Start with no saved controller
ControllerPtr touchpad = nullptr; // Start with no saved touchpad

// Save the connected devices (adapted from Bluepad32)
void controllerConnected(ControllerPtr pad) // Handle a connected device
{
  if (pad->isGamepad()) // Check whether this is a gamepad
  {
    controller = pad; // Save this controller
    Serial.println("Controller connected"); // Show the connection in Serial Monitor
  }
  if (pad->isMouse()) // Check whether this is a mouse device
  {
    touchpad = pad; // Save the touchpad device
  }
}

// Clear the disconnected devices (adapted from Bluepad32)
void onDisconnectedController(ControllerPtr pad) // Handle a disconnected device
{
  if (controller == pad) // Check whether this is the saved controller
  {
    controller = nullptr; // Clear the saved controller
    Serial2.println("Controller disconnected"); // Tell the GIGA the controller disconnected
  }
  if (touchpad == pad) // Check whether this is the saved touchpad
  {
    touchpad = nullptr; // Clear the saved touchpad
  }
}

// Send controller data to the GIGA (based on Bluepad32)
void printControls(ControllerPtr pad) // Prepare and send the controller values
{
  unsigned int touch = 0; // Start with the touchpad not pressed
  if (touchpad && touchpad->isConnected() && (touchpad->buttons() & 3)) // Check whether the touchpad is pressed
  {
    touch = 1; // Mark the touchpad as pressed
  }

  Serial2.printf( // Send these values to the GIGA
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%04x dpad=%02x misc=%02x touch=%u\n", // Set the message labels and order
    pad->axisX(), // Left stick left/right
    pad->axisY(), // Left stick up/down
    pad->axisRX(), // Right stick left/right
    pad->axisRY(), // Right stick up/down
    pad->brake(), // L2 value
    pad->throttle(), // R2 value
    pad->buttons(), // Button values
    pad->dpad(), // D-pad value
    pad->miscButtons(), // Extra button values
    touch); // Touchpad press
}

// Send touchpad data to the GIGA (based on Bluepad32)
void PrintMouse(ControllerPtr pad) // Prepare and send the touchpad values
{
  Serial2.printf("Touchpad buttons=%04x X=%d Y=%d\n", // Send touchpad movement and buttons to the GIGA
    pad->buttons(), // Touchpad buttons
    pad->deltaX(), // Touchpad left/right movement
    pad->deltaY()); // Touchpad up/down movement
}

// Set up serial and Bluetooth (Bluepad32 setup)
void setup() // Prepare the serial and Bluetooth connections
{
  Serial.begin(115200); // Start Serial Monitor at 115200
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Start the GIGA link at 115200, RX16 and TX17
  BP32.setup(&controllerConnected, &onDisconnectedController); // Choose what runs on connect and disconnect
  BP32.enableVirtualDevice(true); // Enable touchpad support
  Serial.println("Bluetooth ready"); // Show that Bluetooth setup is finished
}

// Update Bluetooth and keep sending the controls
void loop() // Keep reading and sending the controls
{
  BP32.update(); // Check for updated Bluetooth data
  if (controller && controller->isConnected()) // Check that the saved controller is connected
  {
    printControls(controller); // Send the saved controller values to the GIGA
  }
  if (touchpad && touchpad->isConnected() && touchpad->hasData()) // Check for data from the connected touchpad
  {
    PrintMouse(touchpad); // Send the touchpad values to the GIGA
  }
  delay(20); // Wait 20 milliseconds before repeating
}
