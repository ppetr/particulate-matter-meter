/*
Copyright 2016 Petr Pudlak

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Based on examples on the Seeed Technology Inc. wiki
www.seeedstudio.com/wiki/
*/
#include <stdio.h>
#include <string.h>
#include <DHT.h>
#include <Wire.h>
#include "running_average.h"

#define MAXWIDTH 12

#define AVERAGE_TIME_MS 300000L

#define DHTPIN A0     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

#define GASPIN A2

// Must be D8
#define DUSTPIN 8
#define DUST_SAMPLE_TIME_US (30L*1000*1000)

struct {
    float humidity; // %
    float temperature;

    float mq9_gas;
    float mq9_gas_raw_volts;

    float dust;  // pcs/0.01cf
    float dust_raw;  // %
    unsigned int dust_cycles;
} data;

void setup()
{

    dht.begin();
    Wire.begin();
    
    Serial.begin(9600);

    pinMode(8, INPUT);
}

void tempSensor(unsigned char row) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();
}

void gasSensor(unsigned char row) {
    // TODO: check for invalid values
    static RunningAverage RS_avg(AVERAGE_TIME_MS);
    int sensorValue = analogRead(GASPIN);
    float sensor_volts = (float)sensorValue / 1024 * 5.0;

    data.mq9_gas = (5.0 - sensor_volts) / sensor_volts;
    data.mq9_gas_raw_volts = sensor_volts;
    RS_avg.addSample(data.mq9_gas);
}

void dustSensor(unsigned char row) {
    static RunningAverage concentration_avg(AVERAGE_TIME_MS);
    float ratio_pct;
    float concentration;
    unsigned long starttime;
    register unsigned long sample_time_us;
    register unsigned long lowpulseoccupancy_us = 0;
    register unsigned int cycles = 0;

    // Wait for the signal to return to HIGH
    pulseIn(DUSTPIN, LOW);
    // Now start measuring
    starttime = micros();
    while ((sample_time_us = (micros() - starttime)) < DUST_SAMPLE_TIME_US) {
        lowpulseoccupancy_us += pulseIn(DUSTPIN, LOW);
        cycles++;
    }

    ratio_pct = 100.0f * lowpulseoccupancy_us / sample_time_us;
    concentration = 1.1 * pow(ratio_pct, 3)
                    - 3.8 * pow(ratio_pct, 2)
                    + 520 * ratio_pct
                    + 0.62; // using spec sheet curve
    // TODO: convert from pcs/0.01cf to metric system
    concentration_avg.addSample(concentration);

    data.dust_cycles = cycles;
    data.dust = concentration;
    data.dust_raw = ratio_pct;
}

bool dumpFloat(const char* name, float value, char* separator) {
    if (isnan(value)) {
        return false;
    }
    Serial.print(*separator);
    Serial.print(name);
    Serial.print("=");
    Serial.print(value);
    *separator = ',';
    return true;
}

void dumpData() {
    Serial.print("sample");
    // Optionally tags would go here.

    char separator = ' ';
    dumpFloat("dust_pc", data.dust, &separator);
    dumpFloat("dust_raw", data.dust_raw, &separator);

    dumpFloat("humidity", data.humidity, &separator);
    dumpFloat("temperature", data.temperature, &separator);

    // TODO: Make sure the output values are correct and include them as well.
    // dumpFloat("mq9", data.mq9_gas, &separator);
    // dumpFloat("mq9_volts", data.mq9_gas_raw_volts, &separator);

    Serial.println();
}

void loop() {
    tempSensor(0);
    gasSensor(2);
    dustSensor(4);
    dumpData();
}
