#ifndef SENSORI_H
#define SENSORI_H

void setupSensori();
void setLedAlarm();
void setAlarmVal(DynamicJsonDocument data);
DynamicJsonDocument SensorValue();

#endif