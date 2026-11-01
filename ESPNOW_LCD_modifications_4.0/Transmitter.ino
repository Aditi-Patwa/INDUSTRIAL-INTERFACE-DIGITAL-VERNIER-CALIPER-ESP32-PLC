/*
  ESP32 Vernier Caliper + Weighing Scale
  ESP-NOW Transmitter

  VERNIER:
  DATA  -> GPIO19
  CLOCK -> GPIO18

  SCALE:
  RX -> GPIO16
  TX -> GPIO17
*/

#include <WiFi.h>
#include <esp_now.h>


// =====================================================
// PIN DEFINITIONS
// =====================================================

#define CALIPER_CLOCK_PIN 18
#define CALIPER_DATA_PIN  19

#define SCALE_RX_PIN 16
#define SCALE_TX_PIN 17


// =====================================================
// ESP-NOW DATA STRUCTURE
// =====================================================

typedef struct
{
  float vernierMM;
  float weightGrams;

} SensorData;


SensorData txData;


// =====================================================
// WAVESHARE ESP32-S3 MAC ADDRESS
// =====================================================

uint8_t receiverMAC[] =
{
  0x58, 0xE6, 0xC5,
  0x72, 0xE4, 0x98
};


// =====================================================
// VERNIER VARIABLES
// =====================================================

volatile uint32_t caliperRaw = 0;
volatile uint8_t caliperBitCount = 0;
volatile bool caliperDataReady = false;
volatile unsigned long lastClockEdge = 0;

float heightMM = 0.0;


// =====================================================
// VERNIER CLOCK INTERRUPT
// =====================================================

void IRAM_ATTR caliperClockISR()
{
  unsigned long now = micros();

  // New frame detected
  if (now - lastClockEdge > 2000)
  {
    caliperBitCount = 0;
    caliperRaw = 0;
  }

  lastClockEdge = now;

  // BC547 inverter
  bool bit = !digitalRead(CALIPER_DATA_PIN);

  if (caliperBitCount < 24)
  {
    // Vernier sends LSB first
    caliperRaw |= ((uint32_t)bit << caliperBitCount);

    caliperBitCount++;

    if (caliperBitCount == 24)
    {
      caliperDataReady = true;
    }
  }
}


// =====================================================
// DECODE VERNIER
// =====================================================

float decodeCaliperToMM(uint32_t raw)
{
  bool negative = raw & (1UL << 20);

  uint32_t magnitude = raw & 0xFFFFF;

  float mm = magnitude / 200.0;

  if (negative)
  {
    mm = -mm;
  }

  return mm;
}


// =====================================================
// WEIGHING SCALE
// =====================================================

HardwareSerial ScaleSerial(1);

String scaleBuffer = "";

float weightGrams = 0.0;


// =====================================================
// PARSE SCALE FRAME
// =====================================================

void parseScaleFrame(String frame)
{
  frame.trim();

  if (frame.length() > 0)
  {
    weightGrams = frame.toFloat();
  }
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  // -----------------------------------------------------
  // HEADER
  // -----------------------------------------------------

  Serial.println();
  Serial.println("==============================");
  Serial.println(" ESP32 MEASUREMENT TRANSMITTER");
  Serial.println("==============================");


  // =====================================================
  // VERNIER
  // =====================================================

  pinMode(CALIPER_CLOCK_PIN, INPUT);
  pinMode(CALIPER_DATA_PIN, INPUT);

  attachInterrupt(
    digitalPinToInterrupt(CALIPER_CLOCK_PIN),
    caliperClockISR,
    RISING
  );


  // =====================================================
  // WEIGHING SCALE
  // =====================================================

  ScaleSerial.begin(
    9600,
    SERIAL_8N1,
    SCALE_RX_PIN,
    SCALE_TX_PIN
  );


  // =====================================================
  // ESP-NOW
  // =====================================================

  WiFi.mode(WIFI_STA);

  Serial.println();

  Serial.print("WROOM MAC: ");
  Serial.println(WiFi.macAddress());


  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization FAILED!");
    return;
  }


  // =====================================================
  // ADD WAVESHARE AS PEER
  // =====================================================

  esp_now_peer_info_t peerInfo = {};

  memcpy(
    peerInfo.peer_addr,
    receiverMAC,
    6
  );

  peerInfo.channel = 0;
  peerInfo.encrypt = false;


  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add ESP-NOW peer!");
    return;
  }


  Serial.println("ESP-NOW transmitter ready.");
  Serial.println("Waiting for Vernier and Scale...");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // =====================================================
  // READ VERNIER
  // =====================================================

  if (caliperDataReady)
  {
    noInterrupts();

    uint32_t raw = caliperRaw;

    caliperDataReady = false;

    interrupts();


    // Decode
    heightMM = decodeCaliperToMM(raw);
  }


  // =====================================================
  // READ WEIGHING SCALE
  // =====================================================

  while (ScaleSerial.available())
  {
    char c = ScaleSerial.read();

    if (c == '\n')
    {
      parseScaleFrame(scaleBuffer);

      scaleBuffer = "";
    }
    else if (c != '\r')
    {
      scaleBuffer += c;
    }
  }


  // =====================================================
  // SEND DATA
  // =====================================================

  static unsigned long lastSend = 0;

  if (millis() - lastSend >= 200)
  {
    lastSend = millis();


    // Prepare packet
    txData.vernierMM = heightMM;
    txData.weightGrams = weightGrams;


    // Send
    esp_err_t result = esp_now_send(
      receiverMAC,
      (uint8_t *)&txData,
      sizeof(txData)
    );


    if (result != ESP_OK)
    {
      Serial.print("ESP-NOW send error: ");
      Serial.println(result);
    }
  }


  // =====================================================
  // SERIAL MONITOR
  // =====================================================

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 500)
  {
    lastPrint = millis();


    Serial.print("Height = ");
    Serial.print(heightMM, 2);

    Serial.print(" mm | Weight = ");
    Serial.print(weightGrams, 1);

    Serial.println(" g");
  }


  delay(5);
}