#include "waveshare_lcd_port.h"

#define RS485_RX_PIN  15
#define RS485_TX_PIN  16
#define RS485 Serial1

float heightMM = 0.0;
float weightGrams = 0.0;
String rxBuffer = "";

void parseIncoming(String frame) {
  int commaIdx = frame.indexOf(',');
  if (commaIdx > 0) {
    heightMM = frame.substring(0, commaIdx).toFloat();
    weightGrams = frame.substring(commaIdx + 1).toFloat();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("LCD Master starting...");

  waveshare_lcd_init();

  RS485.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  while (!RS485) {
    delay(10);
  }

  Serial.println("Waiting for slave data via RS485...");
}

void loop() {
  while (RS485.available()) {
    char c = RS485.read();
    if (c == '\n') {
      parseIncoming(rxBuffer);
      rxBuffer = "";
      Serial.print("Received -> Height = ");
      Serial.print(heightMM, 2);
      Serial.print(" mm | Weight = ");
      Serial.print(weightGrams, 1);
      Serial.println(" g");
    } else if (c != '\r') {
      rxBuffer += c;
    }
  }
}