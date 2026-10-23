// Original Arduino SerialPassthrough example by Erik Nyquist
// Source: https://github.com/arduino/arduino-examples/blob/main/examples/04.Communication/SerialPassthrough/SerialPassthrough.ino

// Arduino SerialPassthrough setup, changed to 115200
void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
}

// Read data
void loop()
{
  if (Serial.available())
  {
    Serial.write(Serial1.read());
  }
}
