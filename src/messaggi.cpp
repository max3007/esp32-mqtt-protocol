#include "messaggi.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include "wiFi_connect.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include "pinSetup.h"
#include "sensori.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char *macadress = getMacAddress();
char jsonConnState[128];
char PinState[64];
int Stato;

DynamicJsonDocument prevJson(2048);
char jsonSensori[500];

void publishMessage(const char *topic, const char *message)
{
    client.publish(topic, message);
}

StaticJsonDocument<64> pin_state(DynamicJsonDocument doc)
{
    int pin = doc["pin"];
    StaticJsonDocument<64> json;
    json["Pin"] = pin;
    json["pin_state"] = digitalRead(pin);
    return json;
}

void rgb_Led(DynamicJsonDocument doc)
{
    Serial.println("Led RGB");
    int redValue = doc["RED"];
    int greenValue = doc["GREEN"];
    int blueValue = doc["BLUE"];
    analogWrite(REDPIN, redValue);
    analogWrite(GREENPIN, greenValue);
    analogWrite(BLUEPIN, blueValue);
}

void pin_action(DynamicJsonDocument doc)
{
    int pin = doc["pin"];

    if (digitalPinToBitMask(pin) & *portModeRegister(digitalPinToPort(pin)))
    {
        Serial.print("Pin: ");
        Serial.print(pin);
        Serial.println(" configurato come OUTPUT");
    }
    else
    {
        Serial.print("Il pin: ");
        Serial.print(pin);
        Serial.println(" non è configurato come OUTPUT");
        Serial.print("configuro: ");
        Serial.print(pin);
        Serial.println(" come OUTPUT");
        pinMode(pin, OUTPUT);

        if (digitalPinToBitMask(pin) & *portModeRegister(digitalPinToPort(pin)))
        {
            Serial.print("pin: ");
            Serial.print(pin);
            Serial.println(" Configurato Correttamente");
        }
        else
        {
            Serial.println("Configurazione non riuscita");
        }
    }

    String instruction = doc["instruction"];

    if (instruction == "OFF")
    {
        digitalWrite(pin, LOW);
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Spento");
    }
    else if (instruction == "TOGGLE")
    {
        Stato = digitalRead(pin);
        digitalWrite(pin, !Stato);
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Toggle");
    }
    else if (instruction == "ON")
    {
        digitalWrite(pin, HIGH);
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Acceso");
    }
    else if (instruction == "PUSH")
    {
        digitalWrite(pin, HIGH);
        delay(100);
        digitalWrite(pin, LOW);
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Effetto Pulsante");
    }
    else if (instruction == "FADE")
    {
        analogWrite(pin, 0);
        for (int i = 0; i < 255; i++)
        {
            analogWrite(pin, i);
            delay(10);
        }
        for (int i = 255; i > 0; i--)
        {
            analogWrite(pin, i);
            delay(10);
        }
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Effetto Fade");
    }
    else if (instruction == "FADE_VAL")
    {
        int fadeValue = doc["fade_value"];
        analogWrite(pin, fadeValue);
        Serial.print("Pin:");
        Serial.print(pin);
        Serial.println(" Acceso con Valore Fade");
    }
    else
    {
        Serial.println(F("Istruzione non riconosciuta."));
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    if (strcmp(topic, macadress) == 0)
    {
        Serial.print("Messaggio Arrivato [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++)
        {
            Serial.print((char)payload[i]);
        }
        Serial.println();

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error)
        {
            Serial.print(F("Errore di parsing JSON: "));
            Serial.println(error.c_str());
            return;
        }

        if (doc.containsKey("pin"))
        {
            pin_action(doc);
            serializeJson(pin_state(doc), PinState);
            publishMessage(macadress, PinState);
        }

        if (doc.containsKey("get_pin_state"))
        {
            serializeJson(pin_state(doc), PinState);
            publishMessage(macadress, PinState);
        }

        if (doc.containsKey("RED") || doc.containsKey("GREEN") || doc.containsKey("BLUE"))
        {
            rgb_Led(doc);
        }

        if (doc.containsKey("allarme_sensore"))
        {
            Serial.println("Imposta nuovi valori allarme");
            setAlarmVal(doc);
            public_val_sensori();
        }

        if (doc.containsKey("get_connection_info"))
        {
            DynamicJsonDocument jsonconn = getWifiInfo();
            serializeJson(jsonconn, jsonConnState);
            publishMessage(macadress, jsonConnState);
            publishMessage(macadress, "connesso");
        }
    }
}

void reconnect()
{
    static unsigned long lastAttemptTime = 0; // initialize with 0
    const unsigned long retryInterval = 5000; // retry interval in milliseconds

    if (client.connected())
    {
        return;
    }

    if (millis() - lastAttemptTime > retryInterval)
    {
        char topic[50];
        strcpy(topic, macadress);
        strcat(topic, "/LWT");

        Serial.print("Connecting to MQTT...");
        if (client.connect(macadress, mqtt_username, mqtt_password, topic, 1, true, "OFF-LINE"))
        {
            Serial.println("Connected to MQTT");
            client.subscribe(macadress);
            Serial.println("subscribed to: ");
            Serial.println(macadress);
            client.publish(topic, NULL, true);
            DynamicJsonDocument jsonconn = getWifiInfo();
            serializeJson(jsonconn, jsonConnState);
            publishMessage(macadress, jsonConnState);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again later");
        }

        lastAttemptTime = millis(); // update the last attempt time
    }
}

void connectMessaggi()
{
    reconnect();
}

void AscoltoMessaggi()
{
    client.loop();
}

void setupMessaggi()
{
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void public_val_sensori()
{
    DynamicJsonDocument currentJson = SensorValue();
    if (currentJson != prevJson)
    {
        serializeJson(currentJson, jsonSensori);
        publishMessage(macadress, jsonSensori);
        prevJson = currentJson;
    }
}