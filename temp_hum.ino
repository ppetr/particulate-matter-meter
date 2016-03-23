#include <stdio.h>
#include <string.h>
#include <DHT.h>
#include <SeeedGrayOLED.h>
#include <Wire.h>

#define MAXWIDTH 12

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

void setup()
{

    dht.begin();
    Wire.begin();
    
    Serial.begin(9600);
  
    SeeedGrayOled.init();  //initialize SEEED Gray OLED display
    SeeedGrayOled.clearDisplay();     // clear the screen and set start position to top left corner
    SeeedGrayOled.setNormalDisplay(); // Set display to Normal mode
    SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying textbvg

    pinMode(8, INPUT);
}

void putFloat(unsigned char row, const char* label, float v,
              unsigned char decimals, const char* suffix) {
    const unsigned char used = strlen(label) + strlen(suffix);
    char buf[16];
    SeeedGrayOled.setTextXY(row, 0);
    SeeedGrayOled.putString(label);
    SeeedGrayOled.putString(dtostrf(v, MAXWIDTH - used, decimals, buf));
    SeeedGrayOled.putString(suffix);
}

void dumpFloatInternal(const char* label, float value, const char* unit) {
    Serial.print(label);
    Serial.print(",");
    Serial.print(value);
    Serial.print(",");
    Serial.print(unit);
    Serial.print(",");
}

void dumpFloat(const char* label, float value, const char* unit) {
    dumpFloatInternal(label, value, unit);
    Serial.println();
}

void dumpFloatRaw(const char* label, float value, const char* unit, float raw) {
    dumpFloatInternal(label, value, unit);
    Serial.println(raw);
}

void tempSensor(unsigned char row) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
        SeeedGrayOled.setTextXY(row, 0);
        SeeedGrayOled.putString("Failed to read from DHT");

    } else {
        putFloat(row, "Temp: ", t, 1, "C");
        dumpFloat("temperature", t, "C");    
    
        putFloat(row + 1, "Hum: ", h, 1, "%");
        dumpFloat("humidity", h, "%");
    }
}

void gasSensor(unsigned char row) {
    // gas sensor
    // TODO: check for invalid values
    int sensorValue = analogRead(GASPIN);
    float sensor_volt = (float)sensorValue / 1024 * 5.0;
    float RS_gas = (5.0 - sensor_volt) / sensor_volt;

    SeeedGrayOled.setTextXY(row, 0);
    SeeedGrayOled.putString("Gas: ");
    putFloat(row, "Gas: ", RS_gas, 2, "");
    dumpFloatRaw("MQ9", RS_gas, "RS", sensorValue);
}

void dustSensor(unsigned char row) {
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

    putFloat(row, "Dust: ", concentration, 0, "");
    dumpFloat("dustcycles", cycles, "");
    dumpFloatRaw("dust", concentration, "pcs/0.01cf", ratio_pct);
}

void loop() {
    tempSensor(0);
    gasSensor(2);
    dustSensor(3);
}
