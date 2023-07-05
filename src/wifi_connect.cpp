#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "wifi_connect.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define TRIGGER_PIN 0

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = true; // change to true to use non blocking

WiFiManager wm;                    // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )

String getParam(String name)
{
    // read parameter from server, for customhmtl input
    String value;
    if (wm.server->hasArg(name))
    {
        value = wm.server->arg(name);
    }
    return value;
}

void saveParamCallback()
{
    Serial.println("[CALLBACK] saveParamCallback fired");
    Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void checkButton()
{
    // check for button press
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
        // poor mans debounce/press-hold, code not ideal for production
        delay(50);
        if (digitalRead(TRIGGER_PIN) == LOW)
        {
            Serial.println("Button Pressed");
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(3000); // reset delay hold
            if (digitalRead(TRIGGER_PIN) == LOW)
            {
                Serial.println("Button Held");
                Serial.println("Erasing Config, restarting");
                wm.resetSettings();
                ESP.restart();
            }

            // start portal w delay
            Serial.println("Starting config portal");
            wm.setConfigPortalTimeout(120);

            if (!wm.startConfigPortal("OnDemandAP", "password"))
            {
                Serial.println("failed to connect or hit timeout");
                delay(3000);
                // ESP.restart();
            }
            else
            {
                // if you get here you have connected to the WiFi
                Serial.println("connected...yeey :)");
            }
        }
    }
}

void WifISetup()
{
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.setDebugOutput(true);
    delay(3000);
    Serial.println("\n Starting");

    pinMode(TRIGGER_PIN, INPUT);

    wm.resetSettings(); // wipe settings

    if (wm_nonblocking)
        wm.setConfigPortalBlocking(false);

    std::vector<const char *> menu = {"wifi", "info", "sep", "restart", "exit"};
    wm.setMenu(menu);

    // set dark theme
    wm.setClass("invert");

    // set static ip
    //  wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
    //  wm.setShowStaticFields(true); // force show static ip fields
    //  wm.setShowDnsFields(true);    // force show dns field always

    wm.setConnectTimeout(20);       // how long to try to connect for before continuing
    wm.setConfigPortalTimeout(120); // auto close configportal after n seconds
    // wm.setCaptivePortalEnable(false); // disable captive portal redirection
    wm.setAPClientCheck(true); // avoid timeout if client connected to softap

    // wifi scan settings
    // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
    // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
    // wm.setShowInfoErase(false);      // do not show erase button on info page
    wm.setScanDispPerc(true); // show RSSI as percentage not graph icons

    // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

    bool res;
    res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    // res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap

    if (!res)
    {
        Serial.println("Failed to connect or hit timeout");
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }

    timeClient.begin();
    timeClient.setTimeOffset(7200); // Imposta il fuso orario a +1 (ad esempio, GMT+1)
}

void WiFiLoop()
{
    if (wm_nonblocking)
        wm.process(); // avoid delays() in loop when non-blocking and other long running code

    checkButton();

    timeClient.update();

    // Serial.println(timeClient.getFormattedTime());
}
const char *getMacAddress()
{
    uint8_t mac[6];
    if (esp_efuse_mac_get_default(mac) == ESP_OK)
    {
        char *id = new char[13];
        snprintf(id, 13, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return id;
    }
    else
    {
        return "";
    }
}

DynamicJsonDocument getWifiInfo()
{
    DynamicJsonDocument json(1024);
    json["status"] = "Connected";
    json["ssid"] = WiFi.SSID();
    json["ip_address"] = WiFi.localIP().toString();
    json["mac_address"] = getMacAddress();
    json["signal_strength"] = WiFi.RSSI();
    return json;
}