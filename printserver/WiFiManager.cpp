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

#include <Arduino.h>
#include <WiFi.h>
#include "WiFiManager.h"

#define CONNECTION_TIMEOUT_MS 10*1000

bool WiFiManager::apEnabled = false;

void WiFiManager::wifi_setup() {
  Serial.println("Connecting to WiFi...");
  WiFi.setAutoConnect(true);
  unsigned long connectionStarted = millis();
  while(!(WiFi.isConnected() || (millis() - connectionStarted > CONNECTION_TIMEOUT_MS))) {
    delay(200);
  }
  if (WiFi.isConnected()) {
    apEnabled = false;
    WiFi.setAutoReconnect(true);
    Serial.println("WiFi connected");
  } else {
    Serial.println("Connection timed out, starting WiFi access point");
    String apSSID = "ESP32PrintServer" + String(ESP.getChipModel());
    if (WiFi.softAP(apSSID.c_str())) {
      apEnabled = true;
      Serial.println("SoftAP started with SSID " + apSSID);
    } else {
      Serial.println("Failed to start SoftAP");
    }
  }
}

String WiFiManager::info() {
  if (apEnabled) {
    return "Access point mode, IP = " + WiFi.softAPIP().toString();
  } else if (WiFi.status() == WL_CONNECTED) {
    return "Connected to " + WiFi.SSID() + ", IP = " + WiFi.localIP().toString() + ", signal = " + WiFi.RSSI() + "dBm";
  } else {
    return "Offline";
  }
}

String WiFiManager::getIP() {
  if (apEnabled) {
    return WiFi.softAPIP().toString();
  } else if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  } else {
    return "";
  }
}

char* WiFiManager::getEncryptionTypeName(int t) {
  //Source: http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/scan-class.html#encryptiontype
  switch (WiFi.encryptionType(t)) {
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA/PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2/PSK";
    case WIFI_AUTH_OPEN: return "Open network";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2/PSK";
    default: return "Unknown";
  }
}

void WiFiManager::getAvailableNetworks(std::function<void(String, int, int)> forEachNet) {
  int count = WiFi.scanNetworks();
  for (int i = 0; i < count; i++) {
    forEachNet(WiFi.SSID(i), WiFi.encryptionType(i),  WiFi.RSSI(i));
  }
  WiFi.scanDelete();
}

void WiFiManager::connectTo(String ssid, String password) {
  if (apEnabled) {
    WiFi.softAPdisconnect(true);
    apEnabled = false;
  }
  Serial.printf("Connecting to %s\r\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());
  wifi_setup();
}
