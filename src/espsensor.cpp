#include <Arduino.h>
#include "SPIFFS.h"
#include <espsensor.h>
#include <pinSetup.h>
#include <DHT.h>
#include "alarm.h"
#include <math.h>

// ****************SENSOR SETUP***************** //
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

ESPSENSOR::ESPSENSOR()
{
    _gas_alarm_value = 1600;
    alarms[0] = 100;
    alarms[1] = 0;
    alarms[2] = 0;
}

void ESPSENSOR::init()
{
    dht.begin();

    //   lettura file Allarmi Sensore
    float *alarms = readSensorAlarmFile(SPIFFS, "/alarmSensor.txt");
}

float ESPSENSOR::getHumidity()
{
    // Read Humidity
    return round(dht.readHumidity());
}
float ESPSENSOR::getTemperatureC()
{
    // Read temperature as Celsius (the default)
    return dht.readTemperature();
}
float ESPSENSOR::getTemperatureF()
{
    // Read temperature as Fahrenheit (isFahrenheit = true)
    return round(dht.readTemperature(true) * 100);
}
float ESPSENSOR::getHeatIndex()
{
    // calcolo temperatura percepita
    return dht.computeHeatIndex(getTemperatureC(), getHumidity(), false);
}

float ESPSENSOR::getHumidity_Alarm()
{
    return alarms[0];
}
float ESPSENSOR::getTempC_Alarm()
{
    return alarms[1];
}
float ESPSENSOR::getTempF_Alarm()
{
    return alarms[2];
}

void ESPSENSOR::setHumidityAlarm(float humidity_alarm)
{
    alarms[0] = humidity_alarm;
}

void ESPSENSOR::setTempCAlarm(float tempc_alarm)
{
    alarms[1] = tempc_alarm;
}
void ESPSENSOR::setTempFAlarm(float tempf_alarm)
{
    alarms[2] = tempf_alarm;
}

void ESPSENSOR::setAlarmLed()
{
    digitalWrite(ledHumPin, getHumidity() > alarms[0] ? HIGH : LOW);
    digitalWrite(ledTempCPin, getTemperatureC() < alarms[1] ? HIGH : LOW);
    digitalWrite(ledTempFPin, getTemperatureF() < alarms[2] ? HIGH : LOW);
}

// ***************sensore GAS MQ-2 Modulo Sensore di Gas***********************
int prevGasVal = 0;

float ESPSENSOR::getGasAnalog()
{
    int currentGasVal = analogRead(sensorGasAnalogPin);
    if (abs(currentGasVal - prevGasVal) >= 5)
    {
        prevGasVal = currentGasVal;
        return round(currentGasVal);
    }
    return prevGasVal;
}

float ESPSENSOR::getGasAnalogPerc()
{
    float gasVal = getGasAnalog();
    return round((gasVal / 4095) * 100.0);
}

int ESPSENSOR::getGasAlarmValue()
{
    return _gas_alarm_value;
}
void ESPSENSOR::setGasAlarmValue(int gas_alarm_value)
{
    _gas_alarm_value = gas_alarm_value;
}

bool ESPSENSOR::GasAlarm()
{
    return getGasAnalog() > getGasAlarmValue();
}

int ESPSENSOR::getGasAlarmLedState()
{
    return digitalRead(sensorGasLedPin);
}

void ESPSENSOR::setGasAlarmLED()
{
    if (getGasAnalog() > getGasAlarmValue())
    {
        // Serial.println("Gas");
        digitalWrite(sensorGasLedPin, HIGH); /*LED set HIGH if Gas detected */
        // buzer Alarm
        // playMelody(buzzer);
    }
    else
    {
        // Serial.println("No Gas");
        digitalWrite(sensorGasLedPin, LOW); /*LED set LOW if NO Gas detected */
    }
}

void ESPSENSOR::sensorSerialOutput()
{
    // Check if any reads failed and exit early (to try again).
    if (isnan(getHumidity()) || isnan(getTemperatureC()) || isnan(getTemperatureF()))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
    }

    Serial.println(F(""));
    Serial.print(F("Humidity: "));
    Serial.print(getHumidity());
    Serial.print(F("%  Temperature: "));
    Serial.print(getTemperatureC());
    Serial.print(F("°C "));
    Serial.print(getTemperatureF());
    Serial.print(F("°F"));
    Serial.println(F(""));

    Serial.print(F("Led Umidità: "));
    Serial.print(digitalRead(ledHumPin));
    Serial.print("\t");
    Serial.print(F("Led °C: "));
    Serial.print(digitalRead(ledTempCPin));
    Serial.print("\t");
    Serial.print(F("Led °F: "));
    Serial.print(digitalRead(ledTempFPin));
    Serial.print("\t");
    Serial.println(F(""));

    Serial.print("Gas Sensor: ");
    Serial.print(getGasAnalog()); /*Read value printed*/
    Serial.print("\t");
    Serial.print(getGasAnalogPerc()); /*Read value printed*/
    Serial.print("%");
    Serial.print("\t");
    Serial.print("\t");
    Serial.print("Gas Alarm: ");
    Serial.print(getGasAlarmValue()); /*Read value printed*/
    Serial.println(F(""));
}

// ****************READ AND WRITE FILE FUNCTIONS***************** //
// READ SENSOR ALARM VALUES FROM A FILE
float *ESPSENSOR::readSensorAlarmFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);
    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return NULL;
    }
    else
    {
        Serial.println("file aperto");
    }
    StaticJsonDocument<512> jsonDoc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error)
    {
        Serial.println(F("Failed to read file"));
        return NULL;
    }

    alarms[0] = jsonDoc["sensor_Umid_allarm"];
    alarms[1] = jsonDoc["sensor_TempC_allarm"];
    alarms[2] = jsonDoc["sensor_TempF_allarm"];
    _gas_alarm_value = jsonDoc["gas_alarm_value"];

    Serial.println("- read from file:");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();

    return alarms;
}

// WRITE SENSOR ALARM VALUES IN A FILE
void ESPSENSOR::writeSensorAlarmFile(fs::FS &fs, const char *path)
{
    // writeFile(SPIFFS, "/hello.txt");
    Serial.printf("Writing file: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    StaticJsonDocument<256> jsonDoc;
    // Set the values in the document
    jsonDoc["sensor_Umid_allarm"] = alarms[0];
    jsonDoc["sensor_TempC_allarm"] = alarms[1];
    jsonDoc["sensor_TempF_allarm"] = alarms[2];
    jsonDoc["gas_alarm_value"] = _gas_alarm_value;

    serializeJsonPretty(jsonDoc, file);
    // Serialize JSON to file
    if (serializeJsonPretty(jsonDoc, file) == 0)
    {
        Serial.println(F("Failed to write to file"));
    }
    file.close();
}

DynamicJsonDocument ESPSENSOR::jsonSensorValues()
{
    DynamicJsonDocument json(2048);

    json["tempC"] = getTemperatureC();
    json["tempF"] = getTemperatureF() / 100;
    json["hum"] = getHumidity();
    json["sensor_Umid_allarm"] = getHumidity_Alarm();
    json["sensor_TempC_allarm"] = getTempC_Alarm();
    json["sensor_TempF_allarm"] = getTempF_Alarm();
    json["gas_sensor_Aout"] = getGasAnalog();
    json["gas_sensor_AoutPerc"] = getGasAnalogPerc();
    json["gas_alarm_value"] = getGasAlarmValue();
    return json;
}

DynamicJsonDocument ESPSENSOR::jsonAlarmValues()
{
    DynamicJsonDocument json(2048);
    json["tempC"] = getTemperatureC();
    json["tempF"] = getTemperatureF() / 100;
    json["hum"] = getHumidity();
    json["sensor_Umid_allarm"] = getHumidity_Alarm();
    json["sensor_TempC_allarm"] = getTempC_Alarm();
    json["sensor_TempF_allarm"] = getTempF_Alarm();
    json["gas_sensor_Aout"] = getGasAnalog();
    json["gas_sensor_AoutPerc"] = getGasAnalogPerc();
    json["gas_alarm_value"] = getGasAlarmValue();
    json["gas_alarm_ledstate"] = getGasAlarmLedState();
    return json;
}

void ESPSENSOR::setAlarm(DynamicJsonDocument jsonDocument)
{
    if (jsonDocument["um_alarm"])
        setHumidityAlarm(jsonDocument["um_alarm"]);

    if (jsonDocument["TempC_alarm"])
        setTempCAlarm(jsonDocument["TempC_alarm"]);

    if (jsonDocument["TempF_alarm"])
        setTempFAlarm(jsonDocument["TempF_alarm"]);

    if (jsonDocument["gas_alarm_value"])
        setGasAlarmValue(jsonDocument["gas_alarm_value"]);

    writeSensorAlarmFile(SPIFFS, "/alarmSensor.txt");
}
