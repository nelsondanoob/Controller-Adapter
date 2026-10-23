// Source: https://github.com/ricardoquesada/esp32-arduino-lib-builder/blob/4.1.0/bluepad32_files/libraries/Bluepad32_ESP32/examples/Controller/Controller.ino

#include <Bluepad32.h> // Connect to the controller through Bluetooth

// Store the controller and touchpad
ControllerPtr controller = nullptr;
ControllerPtr touchpad = nullptr;

// Save the connected devices (adapted from Bluepad32)
void controllerConnected(ControllerPtr pad)
{
  if (pad->isGamepad())
  {
    controller = pad;
    Serial.println("Controller connected");
  }
  if (pad->isMouse())
  {
    touchpad = pad;
  }
}

// Clear the disconnected devices (adapted from Bluepad32)
void onDisconnectedController(ControllerPtr pad)
{
  if (controller == pad)
  {
    controller = nullptr;
    Serial2.println("Controller disconnected");
  }
  if (touchpad == pad)
  {
    touchpad = nullptr;
  }
}

// Send controller data to the GIGA (based on Bluepad32)
void printControls(ControllerPtr pad)
{
  unsigned int touch = 0;
  if (touchpad && touchpad->isConnected() && (touchpad->buttons() & 3))
  {
    touch = 1;
  }

  Serial2.printf(
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%04x dpad=%02x misc=%02x touch=%u\n",
    pad->axisX(),
    pad->axisY(),
    pad->axisRX(),
    pad->axisRY(),
    pad->brake(),
    pad->throttle(),
    pad->buttons(),
    pad->dpad(),
    pad->miscButtons(),
    touch);
}

// Send touchpad data to the GIGA (based on Bluepad32)
void PrintMouse(ControllerPtr pad)
{
  Serial2.printf("Touchpad buttons=%04x X=%d Y=%d\n",
    pad->buttons(),
    pad->deltaX(),
    pad->deltaY());
}

// Set up serial and Bluetooth (Bluepad32 setup)
void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  BP32.setup(&controllerConnected, &onDisconnectedController);
  BP32.enableVirtualDevice(true);
  Serial.println("Bluetooth ready");
}

// Update Bluetooth and keep sending the controls
void loop()
{
  BP32.update();
  if (controller && controller->isConnected())
  {
    printControls(controller);
  }
  if (touchpad && touchpad->isConnected() && touchpad->hasData())
  {
    PrintMouse(touchpad);
  }
  delay(20);
}
