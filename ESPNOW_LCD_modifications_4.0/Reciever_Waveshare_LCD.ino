#include <WiFi.h>
#include <esp_now.h>

#include "waveshare_lcd_port.h"


// =====================================================
// ESP-NOW DATA STRUCTURE
// MUST MATCH ESP32-WROOM TRANSMITTER
// =====================================================

typedef struct
{
    float vernierMM;
    float weightGrams;

} SensorData;


SensorData receivedData;

volatile bool newData = false;


// =====================================================
// ESP-NOW RECEIVE CALLBACK
// =====================================================

void onDataRecv(
    const esp_now_recv_info_t *recv_info,
    const uint8_t *incomingData,
    int len
)
{
    if (len != sizeof(SensorData))
    {
        Serial0.print("Invalid packet size: ");
        Serial0.println(len);
        return;
    }

    memcpy(
        &receivedData,
        incomingData,
        sizeof(receivedData)
    );

    newData = true;
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial0.begin(115200);

    delay(2000);

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println(" WAVESHARE MEASUREMENT RECEIVER");
    Serial0.println("==============================");


    // -------------------------------------------------
    // INITIALIZE LCD
    // -------------------------------------------------

    waveshare_lcd_init();

    Serial0.println("LCD initialized.");


    // -------------------------------------------------
    // INITIALIZE WIFI
    // -------------------------------------------------

    WiFi.mode(WIFI_STA);

    Serial0.print("Waveshare MAC: ");
    Serial0.println(WiFi.macAddress());


    // -------------------------------------------------
    // INITIALIZE ESP-NOW
    // -------------------------------------------------

    if (esp_now_init() != ESP_OK)
    {
        Serial0.println(
            "ERROR: ESP-NOW initialization failed!"
        );

        return;
    }


    esp_now_register_recv_cb(onDataRecv);


    Serial0.println("ESP-NOW receiver ready.");
    Serial0.println("Waiting for measurements...");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    if (newData)
    {
        noInterrupts();

        float vernier = receivedData.vernierMM;
        float weight = receivedData.weightGrams;

        newData = false;

        interrupts();


        // -------------------------------------------------
        // SERIAL OUTPUT
        // -------------------------------------------------

        Serial0.print("Vernier = ");
        Serial0.print(vernier, 2);

        Serial0.print(" mm | Weight = ");
        Serial0.print(weight, 1);

        Serial0.println(" g");


        // -------------------------------------------------
        // TEMPORARY LCD TEST
        // -------------------------------------------------

        // For now, send ONLY the Vernier value to the
        // existing LCD function.
        //
        // We will replace this with the final two-value
        // LCD layout in the next step.

        lcd_update_measurements(vernier, weight);
    }

    delay(5);
}