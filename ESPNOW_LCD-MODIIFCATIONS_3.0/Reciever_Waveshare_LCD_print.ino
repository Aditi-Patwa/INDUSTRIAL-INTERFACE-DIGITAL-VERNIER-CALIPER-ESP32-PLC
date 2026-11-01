#include <WiFi.h>
#include <esp_now.h>

#include "waveshare_lcd_port.h"


// =====================================================
// ESP-NOW DATA STRUCTURE
// Must be EXACTLY the same as transmitter
// =====================================================

typedef struct
{
    float vernierMM;
} SensorData;


// Received data
SensorData receivedData;


// =====================================================
// ESP-NOW RECEIVE CALLBACK
// =====================================================

void onDataRecv(
    const esp_now_recv_info_t *recv_info,
    const uint8_t *incomingData,
    int len)
{
    // Check packet size
    if (len != sizeof(SensorData))
    {
        Serial0.print("Invalid packet size: ");
        Serial0.println(len);
        return;
    }

    // Copy received data
    memcpy(
        &receivedData,
        incomingData,
        sizeof(receivedData)
    );


    // =================================================
    // PRINT TO SERIAL MONITOR
    // =================================================

    Serial0.print("Received Vernier = ");
    Serial0.print(receivedData.vernierMM, 2);
    Serial0.println(" mm");


    // =================================================
    // CREATE TEXT FOR LCD
    // =================================================

    char displayText[50];

    snprintf(
        displayText,
        sizeof(displayText),
        "VERNIER %.2f mm",
        receivedData.vernierMM
    );


    // =================================================
    // SEND TO LCD
    // =================================================

    lcd_print_text(displayText);
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    // IMPORTANT:
    // Your Waveshare board uses Serial0 for the
    // serial connection that you are currently using.
    Serial0.begin(115200);

    delay(2000);


    Serial0.println();
    Serial0.println("==============================");
    Serial0.println("WAVESHARE ESP-NOW RECEIVER");
    Serial0.println("==============================");


    // =================================================
    // INITIALIZE LCD
    // =================================================

    waveshare_lcd_init();

    Serial0.println("LCD initialized.");


    // =================================================
    // INITIALIZE WIFI
    // =================================================

    WiFi.mode(WIFI_STA);

    Serial0.print("Waveshare MAC: ");
    Serial0.println(WiFi.macAddress());


    // =================================================
    // INITIALIZE ESP-NOW
    // =================================================

    if (esp_now_init() != ESP_OK)
    {
        Serial0.println("ERROR: ESP-NOW initialization failed!");
        return;
    }


    // Register receive callback
    esp_now_register_recv_cb(onDataRecv);


    Serial0.println("ESP-NOW Receiver Ready");
    Serial0.println("Waiting for Vernier data...");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    // Nothing required here.
    // ESP-NOW receives packets through the callback.
}