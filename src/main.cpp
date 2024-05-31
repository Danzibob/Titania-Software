#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define LED_PIN 1
#define TOGGLE 3

#define LOCAL_ENDPOINT "http://172.23.0.130:8080/test"
#define CLOUD_ENDPOINT "http://88.208.197.140:8080/test"

#define BUFF_SIZE 6000
#define MS_PER_READ 200

float c_buff[BUFF_SIZE * 2];
uint buff_ptr = 0;
ulong next_log_time = 0;

const char* ssid = "Danzi'sPixel";       // WiFi SSID
const char* password = "RocketGoWoosh";  // Password

Adafruit_BMP280 bmp;
float baseline_pressure;

void blinkN(int N) {
    for (; N > 0; N--) {
        digitalWrite(LED_PIN, LOW);
        delay(50);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
    }
}

void blinkVal(uint val) {
    for (uint place = 1; place < val; place *= 10) {
        delay(600);
        blinkN((val / place) % 10);
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(TOGGLE, INPUT);

    WiFi.disconnect();  // Ensure we're not trying to wifi yet

    delay(2500);

    Wire.begin(0, 2);
    while (!bmp.begin(0x76)) {
        blinkVal(313);
        delay(2000);
    }
    baseline_pressure = bmp.readPressure() / 100.0;
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
    float altitude;
    ulong time;
    switch (digitalRead(TOGGLE)) {
        case 1:  // Logging
            // Circular buffer
            buff_ptr = (buff_ptr + 1) % BUFF_SIZE;
            // Wait for next logging time
            time = millis();
            while (time < next_log_time) {
                delay(1);
                time = millis();
            }
            altitude = bmp.readAltitude(baseline_pressure);
            next_log_time = time + MS_PER_READ;
            c_buff[buff_ptr] = altitude;
            break;
        case 0:  // Exporting
            // Connect to hotspot
            if (WiFi.status() == WL_DISCONNECTED) WiFi.begin(ssid, password);
            while (WiFi.status() != WL_CONNECTED) {
                delay(400);
                blinkN(2);
            }
            // Send data via HTTP request
            HTTPClient http;
            WiFiClient wifiClient;
            // First, copy first part of ring buffer to after itself
            for (uint i = 0; i < buff_ptr; i++) {
                c_buff[i + BUFF_SIZE] = c_buff[i];
            }
            // Then identify the starting point for the buffer
            uint8_t* body = reinterpret_cast<uint8_t*>(c_buff) + static_cast<uint32_t>(buff_ptr) * sizeof(float);
            // and relay BUFF_SIZE floats from that point
            int responseCode = -1;
            while (1) {
                http.begin(wifiClient, CLOUD_ENDPOINT);
                http.addHeader("Content-Type", "application/octet-stream");
                responseCode = http.POST(body, BUFF_SIZE * sizeof(float));
                delay(1000);
                if (responseCode > 0) {
                    blinkN(responseCode / 100);
                } else {
                    blinkN(1);
                }
                // Check if the data was successfully recieved
                if ((responseCode / 100) == 2) {
                    while (1) {
                        delay(1000);
                        blinkN(2);
                    }
                }
            }
    }
}