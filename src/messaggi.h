#ifndef MESSAGGI_H
#define MESSAGGI_H
#include <ArduinoJson.h>

// Funzione per la pubblicazione dei messaggi
void publishMessage(const char *topic, const char *message);

StaticJsonDocument<64> pin_state(DynamicJsonDocument doc);

// Funzione istruzione ON OFF PUSH sul pin
void pin_action(DynamicJsonDocument doc);
// Funzione per l'ascolto messaggi sottoscritti
void callback(char *topic, byte *payload, unsigned int length);

void reconnect();
void connectMessaggi();
void setupMessaggi();
void AscoltoMessaggi();
void public_val_sensori();
#endif