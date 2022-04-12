/*!
 * @file Wippersnapper_LittleFS.cpp
 *
 * Interfaces with LittleFS filesystem for ESP32, ESP8266 platforms.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Copyright (c) Brent Rubell 2021-2022 for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */
#if defined(ARDUINO_FEATHER_ESP32) ||                                          \
    defined(ARDUINO_ESP8266_ADAFRUIT_HUZZAH) ||                                \
    defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
#include "WipperSnapper_LittleFS.h"

/**************************************************************************/
/*!
    @brief    Attempts to set up and initialize a pre-existing LittleFS
              filesystem.
*/
/**************************************************************************/
WipperSnapper_LittleFS::WipperSnapper_LittleFS() {
  // Attempt to initialize filesystem
  if (!LittleFS.begin()) {
    WS_DEBUG_PRINTLN("ERROR: Failure initializing LittleFS!");
    WS.setStatusLEDColor(RED);
    while (1)
      ;
  }
}

/**************************************************************************/
/*!
    @brief    Destructor for LittleFS
*/
/**************************************************************************/
WipperSnapper_LittleFS::~WipperSnapper_LittleFS() { LittleFS.end(); }

/**************************************************************************/
/*!
    @brief    Locates, opens and parses the WipperSnapper secrets file
              on the LittleFS filesystem.
*/
/**************************************************************************/
void WipperSnapper_LittleFS::parseSecrets() {

  // Check if `secrets.json` file exists on FS
  if (!LittleFS.exists("/secrets.json")) {
    WS_DEBUG_PRINTLN("ERROR: No secrets.json found on filesystem - did you "
                     "upload credentials?");
    fsHalt();
  }

  // open file for parsing
  File secretsFile = LittleFS.open("/secrets.json", "r");
  if (!secretsFile) {
    WS_DEBUG_PRINTLN("ERROR: Could not open secrets.json file for reading!");
    fsHalt();
  }

  // check if we can deserialize the secrets.json file
  DeserializationError err = deserializeJson(_doc, secretsFile);
  if (err) {
    WS_DEBUG_PRINT("ERROR: deserializeJson() failed with code ");
    WS_DEBUG_PRINTLN(err.c_str());
    fsHalt();
  }

  // Get IO username from JSON
  const char *io_username = _doc["io_username"];
  // error check against default values [ArduinoJSON, 3.3.3]
  if (io_username == nullptr) {
    WS_DEBUG_PRINTLN("ERROR: io_username not set!");
    fsHalt();
  }
  // Set IO username
  WS._username = io_username;

  // Get IO key from JSON
  const char *io_key = _doc["io_key"];
  // error check against default values [ArduinoJSON, 3.3.3]
  if (io_key == nullptr) {
    WS_DEBUG_PRINTLN("ERROR: io_key not set!");
    fsHalt();
  }
  // Set IO key
  WS._key = io_key;

  // Parse SSID
  const char *network_type_wifi_native_network_ssid =
      _doc["network_type_wifi_native"]["network_ssid"];
  if (network_type_wifi_native_network_ssid == nullptr) {
    WS_DEBUG_PRINTLN("ERROR: network_ssid not set!");
    fsHalt();
  }
  // set SSID
  WS._network_ssid = network_type_wifi_native_network_ssid;

  // Parse SSID password
  const char *network_type_wifi_native_network_password =
      _doc["network_type_wifi_native"]["network_password"];
  // validation
  if (network_type_wifi_native_network_password == nullptr) {
    WS_DEBUG_PRINTLN("ERROR: network_password not set!");
    fsHalt();
  }
  // set password
  WS._network_pass = network_type_wifi_native_network_password;

  // Optionally set the Adafruit.io URL
  WS._mqttBrokerURL = _doc["io_url"];

  // close the file
  secretsFile.close();

  // clear the document and release all memory from the memory pool
  _doc.clear();

  // stop LittleFS, we no longer need it
  LittleFS.end();
}

void WipperSnapper_LittleFS::fsHalt() {
  while (1) {
    WS.statusLEDBlink(WS_LED_STATUS_FS_WRITE);
    delay(1000);
    yield();
  }
}
#endif