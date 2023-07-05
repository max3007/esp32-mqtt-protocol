#ifndef LED_H
#define LED_H

// Pin SENSORI
#define DHTPIN 21 // Digital pin connected to the DHT sensor
// Set LED Umidità
const int ledHumPin = 2;
// Set LED Temperatura C
const int ledTempCPin = 17;
// Set LED Temperatura F
const int ledTempFPin = 19;
const int sensorGasLedPin = 32;
const int sensorGasAnalogPin = 36;
#define buzzer 33

// PIN LED RGB
// Set RED RGBLED
const int REDPIN = 27;
// Set GREEN RGBLED
const int GREENPIN = 26;
// Set BLUE RGBLED
const int BLUEPIN = 25;

// PIN CREPUSCOLARE
const int photoResistorPin = A0;  // Pin analogico collegato alla fotoresistenza
const int PinCrepuscolareOUT = 3; // Pin per il relè
#endif