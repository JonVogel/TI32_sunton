// Step-1 placeholder. This stub only proves that the IDF scaffolding
// (top-level CMakeLists, sdkconfig.defaults, main component registration,
// arduino-esp32 dependency, components/arduino shim) is wired up
// correctly and the project links into a runnable image.
//
// The real .ino → main.cpp port replaces this file in the next commit,
// at which point we also move rgb_db / sd_manager / ble_* headers into
// main/ and apply the forward-decls / header-path edits per the IDF
// port playbook.

#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("IDF scaffolding alive — stub from main.cpp");
}

void loop()
{
  delay(1000);
}
