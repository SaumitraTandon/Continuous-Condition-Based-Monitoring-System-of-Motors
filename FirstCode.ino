#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "your_ssid";
const char* password = "your_password";

// ThingSpeak API
const char* server = "http://api.thingspeak.com/update";
String apiKey = "your_thingspeak_api_key";

// MPU6050 setup
Adafruit_MPU6050 mpu;

// DHT22 setup
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// PZEM004T setup
HardwareSerial pzemSerial(1);  
PZEM004Tv30 pzem(pzemSerial, 12, 13);  

// GPS setup
HardwareSerial gpsSerial(2);  
// IR sensor setup for RPM calculation
#define IR_PIN 18
volatile int pulseCount = 0;
unsigned long lastPulseTime = 0;
float motorRPM = 0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
  unsigned long currentTime = millis();
  unsigned long duration = currentTime - lastPulseTime;
  motorRPM = (pulseCount * 60000.0) / duration; 
  lastPulseTime = currentTime;
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);

  // Initialize DHT22
  dht.begin();

  // Initialize PZEM004T
  pzemSerial.begin(9600, SERIAL_8N1, 12, 13);  

  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  
  // Initialize IR sensor
  pinMode(IR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_PIN), pulseCounter, FALLING);
}

void loop() {
  // Get MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float vibrationX = a.acceleration.x;
  float vibrationY = a.acceleration.y;
  float vibrationZ = a.acceleration.z;

  // Get DHT22 data (temperature and humidity)
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Get PZEM004T data (voltage, current, power)
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();

  // Get GPS data (latitude and longitude)
  String gpsData = "";
  if (gpsSerial.available()) {
    gpsData = gpsSerial.readString();
    Serial.println("GPS Data: " + gpsData);
  }

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(voltage) + 
                 "&field2=" + String(current) +
                 "&field3=" + String(power) + 
                 "&field4=" + String(motorRPM) +
                 "&field5=" + String(vibrationX) + 
                 "&field6=" + String(vibrationY) +
                 "&field7=" + String(vibrationZ) +
                 "&field8=" + String(temperature);
                 
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully: " + String(httpResponseCode));
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(20000);  // Send data every 20 seconds
}
