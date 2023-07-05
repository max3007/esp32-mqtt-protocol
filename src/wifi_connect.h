#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H
#include <ArduinoJson.h>
#include <string>
using namespace std;

// ****************CUSTOM FUNCTIONS***************** //
void saveParamCallback();

// GET ESP32 MAC ADDRESS
const char *getMacAddress();

String getParam(String name);

void checkButton();

void WifISetup();

void WiFiLoop();

void printEncryptionType(int thisType);

void setup_wifi();

DynamicJsonDocument getWifiInfo();

#endif