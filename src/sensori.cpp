#ifndef SENSORI_H
#define SENSORI_H
#include "sensori.h"
#include "espsensor.h"
#include "pinSetup.h"

// #include "messaggi.h"

// dichiarazione classe Sensori
ESPSENSOR espsensor;

void setLedAlarm()
{
    // set pin mode
    pinMode(sensorGasLedPin, OUTPUT);
    pinMode(ledHumPin, OUTPUT);
    pinMode(ledTempCPin, OUTPUT);
    pinMode(ledTempFPin, OUTPUT);
    pinMode(buzzer, OUTPUT);
    // Controllo allarmi sensore
    espsensor.setAlarmLed();

    // imposta il led del gas accesso o spento
    espsensor.setGasAlarmLED();
}

void setupSensori()
{
    // Inizializza i  Sensori
    espsensor.init();
    setLedAlarm();
}

void setAlarmVal(DynamicJsonDocument data)
{
    espsensor.setAlarm(data);
    setLedAlarm();
}

DynamicJsonDocument SensorValue()
{
    return espsensor.jsonSensorValues();
}

#endif