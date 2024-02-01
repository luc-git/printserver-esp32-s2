/*
    This file is part of printserver-esp8266.

    printserver-esp8266 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    printserver-esp8266 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with printserver-esp8266.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <FS.h>

#include "WiFiManager.h"
#include "TcpPrintServer.h"
#include "DirectParallelPortPrinter.h"
#include "ShiftRegParallelPortPrinter.h"
#include "SerialPortPrinter.h"
#include "PrintQueue.h"

/*#define STROBE 10
#define BUSY 9
int DATA[8] = {D0, D1, D2, D3, D4, D5, D6, D7};
DirectParallelPortPrinter printer("lpt1", DATA, STROBE, BUSY);*/

/*#define LPT_DATA D2
#define LPT_LATCH D3
#define LPT_CLK D4
#define LPT_BUSY D5
#define LPT_STROBE D6
ShiftRegParallelPortPrinter printer1("parallel", LPT_DATA, LPT_CLK, LPT_LATCH, LPT_STROBE, LPT_BUSY);*/

#define CH375_TX D3
#define CH375_RX D6
#define CH375_INT D4

SerialPortPrinter printer2("serial", &Serial);
Printer* printers[] = {&printer2};

#define PRINTER_COUNT (sizeof(printers) / sizeof(printers[0]))
TcpPrintServer server(printers, PRINTER_COUNT);

inline void printDebugAndYield();

void setup() {
  Serial.begin(115200);
  Serial.println("boot ok");
  SPIFFS.begin();
  for (unsigned int i = 0; i < PRINTER_COUNT; i++) {
    printers[i]->init();
  }
  Serial.println("initialized printers");
  WiFiManager::wifi_setup();
  server.start();
  Serial.println("setup ok");
}

void loop() {
  printDebugAndYield();
  server.process();
  for (unsigned int i = 0; i < PRINTER_COUNT; i++) {
    printers[i]->processQueue();
  }
}

inline void printDebugAndYield() {
  static unsigned long lastCall = 0;
  if (millis() - lastCall > 5000) {
    Serial.printf("Free heap: %d bytes\r\n", ESP.getFreeHeap());
    Serial.println(WiFiManager::info());
    server.printInfo();
    Serial.printf("[Filesystem] Total bytes: %d, Used bytes: %d, Max open files: %d\r\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());

    PrintQueue::updateAvailableFlashSpace();
    yield();

    lastCall = millis();
  }
}
