#include <Arduino.h>
#include "messaggi.h"
#include "wiFi_connect.h"
#include "sensori.h"
#include "SPIFFS.h"

void setup()
{
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  WifISetup();
  setupMessaggi();
  setupSensori();
}

void loop()
{
  WiFiLoop();

  // connessione al server MQTT e sottoscrizione topic messaggi
  connectMessaggi();

  // ascolta per i messaggi in arrivo
  AscoltoMessaggi();

  // publicazione valori sensori
  public_val_sensori();
}
