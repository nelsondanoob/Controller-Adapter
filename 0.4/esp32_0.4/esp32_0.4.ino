// Source: https://github.com/ricardoquesada/esp32-arduino-lib-builder/blob/4.1.0/bluepad32_files/libraries/Bluepad32_ESP32/examples/Controller/Controller.ino

#include <Bluepad32.h>

// Bluepad32 controller pointers
ControllerPtr controller = nullptr;
ControllerPtr touchpad = nullptr;

// Connected
// Adapted from Bluepad32
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

// Disconnected
// Adapted from Bluepad32
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

// Controller data
// Based on Bluepad32
void printControls(ControllerPtr pad)
{
  Serial2.printf(
    "LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d buttons=%04x dpad=%02x misc=%02x\n",
    pad->axisX(),
    pad->axisY(),
    pad->axisX(),
    pad->axisRY(),
    pad->brake(),
    pad->throttle(),
    pad->buttons(),
    pad->dpad(),
    pad->miscButtons());
}

// Touchpad data
// Based on Bluepad32
void PrintMouse(ControllerPtr pad)
{
  Serial2.printf("Touchpad buttons=%04x X=%d Y=%d\n",
    pad->buttons(),
    pad->deltaX(),
    pad->deltaY());
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  // Bluepad32 callback setup
  BP32.setup(&controllerConnected, &onDisconnectedController);
  BP32.enableVirtualDevice(true);
  Serial.println("Bluetooth ready");
}

void loop()
{
  // Bluepad32 update and data checks
  BP32.update();
  if (controller && controller->isConnected() && controller->hasData())
  {
    printControls(controller);
  }
  if (touchpad && touchpad->isConnected() && touchpad->hasData())
  {
    PrintMouse(touchpad);
  }
  delay(20);
}
