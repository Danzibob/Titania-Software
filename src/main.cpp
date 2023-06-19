#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


#define LED_PIN 1
#define TOGGLE  3

#define BUFF_SIZE 4000

#define ENDPOINT "http://192.168.1.205:8080/test"

const char* ssid = "Danzi'sPixel";      // WiFi SSID
const char* password = "RocketGoWoosh"; // Password

Adafruit_BMP280 bmp;

int toggle_val;
int ok = 0;
float max_alt = 0.0;
float base_pressure = 0.0;

// Circular buffer to hold recent altitude data
uint8_t buffer_ptr = 0;
float c_buff[BUFF_SIZE*2];


void blinkN(int N){
  for(; N > 0; N--){
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
  }
}

void blinkVal(int val){
    int hundreds = (int)(val / 100) % 10;
    int tens = (int)(val / 10)%10;
    int ones = (int)(val)%10;
    delay(1000);
    blinkN(hundreds);
    delay(500);
    blinkN(tens);
    delay(500);
    blinkN(ones);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(TOGGLE, INPUT);

  WiFi.disconnect();  // Ensure we're not trying to wifi yet

  delay(2500);

  Wire.begin (0,2);
  ok = bmp.begin(0x76);
  base_pressure = bmp.readPressure()/100.0;

  EEPROM.begin(16);
  digitalWrite(LED_PIN, HIGH);
}

// the loop function runs over and over again forever
void loop() {
  if(ok){
    if(digitalRead(3)){
      float altitude = bmp.readAltitude(base_pressure);
      c_buff[buffer_ptr++] = altitude;
      if(buffer_ptr >= BUFF_SIZE) buffer_ptr = 0;
      delay(250);
    } else {
      if (WiFi.status() == WL_DISCONNECTED) WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        blinkN(2);
      }
      int lastByte = WiFi.localIP()[3];
      blinkVal(lastByte);

      // Send data via HTTP request
      HTTPClient http;
      WiFiClient wifiClient;
      http.begin(wifiClient,ENDPOINT);
      http.addHeader("Content-Type", "application/octet-stream");
      // First, copy first part of rung buffer to after itself
      for(int i=0; i<buffer_ptr; i++){
        c_buff[i+BUFF_SIZE] = c_buff[i];
      }
      // Then identify the starting point for the buffer
      byte* body = (byte*)c_buff + (u32)buffer_ptr*sizeof(float);
      // and relay BUFF_SIZE floats from that point
      int responseCode = http.POST(body, BUFF_SIZE*sizeof(float));
      if(responseCode > 0){
        blinkN(responseCode/100);
      } else {
        blinkN(10);
      }
    }
  }
}
