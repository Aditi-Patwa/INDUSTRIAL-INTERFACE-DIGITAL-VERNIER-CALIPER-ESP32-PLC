#include "waveshare_lcd_port.h"

void setup()
{
    Serial0.begin(115200);

    delay(2000);

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println("WAVESHARE SERIAL LCD TEST");
    Serial0.println("==============================");

    waveshare_lcd_init();

    Serial0.println("LCD initialized.");
    Serial0.println();
    Serial0.println("TYPE YOUR TEXT BELOW AND PRESS ENTER");
    Serial0.println("------------------------------------");
}

void loop()
{
    if (Serial0.available() > 0)
    {
        String text = Serial0.readStringUntil('\n');

        text.trim();

        if (text.length() > 0)
        {
            Serial0.print("You typed: ");
            Serial0.println(text);

            lcd_print_text(text.c_str());
        }

        Serial0.println("TYPE ANOTHER MESSAGE:");
    }

    delay(10);
}