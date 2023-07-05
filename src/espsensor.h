#ifndef SENSOR
#define SENSOR
#include <ArduinoJson.h>
#include <FS.h>
class ESPSENSOR
{
public:
    ESPSENSOR();
    void init();
    float getHumidity();
    float getTemperatureC();
    float getTemperatureF();
    float getHeatIndex();
    float getHumidity_Alarm();
    float getTempC_Alarm();
    float getTempF_Alarm();
    float getGasAnalog();
    float getGasAnalogPerc();
    int getGasAlarmValue();
    void setAlarmLed();
    void setGasAlarmLED();
    void sensorSerialOutput();
    bool GasAlarm();
    int getGasAlarmLedState();
    DynamicJsonDocument jsonSensorValues();
    DynamicJsonDocument jsonAlarmValues();
    DynamicJsonDocument jsonLedState(int led);
    void setAlarm(DynamicJsonDocument jsonDocument);

private:
    float valuesClass[3];
    float alarms[3];
    float gas_valuesClass[2];
    int _gas_alarm_value;
    void setGasAlarmValue(int);
    void setHumidityAlarm(float);
    void setTempCAlarm(float);
    void setTempFAlarm(float);
    float *readSensorAlarmFile(FS &fs, const char *path);
    void writeSensorAlarmFile(FS &fs, const char *path);
};

#endif // SENSOR
