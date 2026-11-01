#include "waveshare_lcd_port.h"

using namespace esp_panel::drivers;

static LCD *lcd = nullptr;


/* =========================================================
   LCD CREATION
   ========================================================= */

static LCD *create_lcd_without_config(void)
{
    BusRGB *bus = new BusRGB(
#if EXAMPLE_LCD_RGB_DATA_WIDTH == 8

        EXAMPLE_LCD_RGB_IO_DATA0,
        EXAMPLE_LCD_RGB_IO_DATA1,
        EXAMPLE_LCD_RGB_IO_DATA2,
        EXAMPLE_LCD_RGB_IO_DATA3,
        EXAMPLE_LCD_RGB_IO_DATA4,
        EXAMPLE_LCD_RGB_IO_DATA5,
        EXAMPLE_LCD_RGB_IO_DATA6,
        EXAMPLE_LCD_RGB_IO_DATA7,

        EXAMPLE_LCD_RGB_IO_HSYNC,
        EXAMPLE_LCD_RGB_IO_VSYNC,
        EXAMPLE_LCD_RGB_IO_PCLK,
        EXAMPLE_LCD_RGB_IO_DE,
        EXAMPLE_LCD_RGB_IO_DISP,

        EXAMPLE_LCD_RGB_TIMING_FREQ_HZ,
        EXAMPLE_LCD_WIDTH,
        EXAMPLE_LCD_HEIGHT,

        EXAMPLE_LCD_RGB_TIMING_HPW,
        EXAMPLE_LCD_RGB_TIMING_HBP,
        EXAMPLE_LCD_RGB_TIMING_HFP,

        EXAMPLE_LCD_RGB_TIMING_VPW,
        EXAMPLE_LCD_RGB_TIMING_VBP,
        EXAMPLE_LCD_RGB_TIMING_VFP

#elif EXAMPLE_LCD_RGB_DATA_WIDTH == 16

        EXAMPLE_LCD_RGB_IO_DATA0,
        EXAMPLE_LCD_RGB_IO_DATA1,
        EXAMPLE_LCD_RGB_IO_DATA2,
        EXAMPLE_LCD_RGB_IO_DATA3,
        EXAMPLE_LCD_RGB_IO_DATA4,
        EXAMPLE_LCD_RGB_IO_DATA5,
        EXAMPLE_LCD_RGB_IO_DATA6,
        EXAMPLE_LCD_RGB_IO_DATA7,

        EXAMPLE_LCD_RGB_IO_DATA8,
        EXAMPLE_LCD_RGB_IO_DATA9,
        EXAMPLE_LCD_RGB_IO_DATA10,
        EXAMPLE_LCD_RGB_IO_DATA11,
        EXAMPLE_LCD_RGB_IO_DATA12,
        EXAMPLE_LCD_RGB_IO_DATA13,
        EXAMPLE_LCD_RGB_IO_DATA14,
        EXAMPLE_LCD_RGB_IO_DATA15,

        EXAMPLE_LCD_RGB_IO_HSYNC,
        EXAMPLE_LCD_RGB_IO_VSYNC,
        EXAMPLE_LCD_RGB_IO_PCLK,
        EXAMPLE_LCD_RGB_IO_DE,
        EXAMPLE_LCD_RGB_IO_DISP,

        EXAMPLE_LCD_RGB_TIMING_FREQ_HZ,
        EXAMPLE_LCD_WIDTH,
        EXAMPLE_LCD_HEIGHT,

        EXAMPLE_LCD_RGB_TIMING_HPW,
        EXAMPLE_LCD_RGB_TIMING_HBP,
        EXAMPLE_LCD_RGB_TIMING_HFP,

        EXAMPLE_LCD_RGB_TIMING_VPW,
        EXAMPLE_LCD_RGB_TIMING_VBP,
        EXAMPLE_LCD_RGB_TIMING_VFP

#endif
    );

    return new EXAMPLE_LCD_CLASS(
        EXAMPLE_LCD_NAME,
        bus,
        EXAMPLE_LCD_WIDTH,
        EXAMPLE_LCD_HEIGHT,
        EXAMPLE_LCD_COLOR_BITS,
        EXAMPLE_LCD_RST_IO
    );
}


/* =========================================================
   LCD CALLBACKS
   Keep these because they are part of the working
   Waveshare demo configuration.
   ========================================================= */

#if EXAMPLE_LCD_ENABLE_PRINT_FPS

DRAM_ATTR int frame_count = 0;
DRAM_ATTR int fps = 0;
DRAM_ATTR long start_time = 0;

IRAM_ATTR bool onLCD_RefreshFinishCallback(void *user_data)
{
    if (start_time == 0)
    {
        start_time = millis();
        return false;
    }

    frame_count++;

    if (frame_count >= EXAMPLE_LCD_PRINT_FPS_COUNT_MAX)
    {
        fps = EXAMPLE_LCD_PRINT_FPS_COUNT_MAX * 1000 /
              (millis() - start_time);

        esp_rom_printf("LCD FPS: %d\n", fps);

        frame_count = 0;
        start_time = millis();
    }

    return false;
}

#endif


#if EXAMPLE_LCD_ENABLE_DRAW_FINISH_CALLBACK

IRAM_ATTR bool onLCD_DrawFinishCallback(void *user_data)
{
    return false;
}

#endif


/* =========================================================
   LCD INITIALIZATION
   ========================================================= */

void waveshare_lcd_init(void)
{
    Serial.println("Initializing RGB LCD...");

    lcd = create_lcd_without_config();

    if (lcd == nullptr)
    {
        Serial.println("ERROR: LCD creation failed!");
        return;
    }

    auto bus = static_cast<BusRGB *>(lcd->getBus());

    bus->configRGB_BounceBufferSize(
        EXAMPLE_LCD_RGB_BOUNCE_BUFFER_SIZE
    );

    lcd->init();

#if EXAMPLE_LCD_ENABLE_PRINT_FPS

    lcd->attachRefreshFinishCallback(
        onLCD_RefreshFinishCallback
    );

#endif

#if EXAMPLE_LCD_ENABLE_DRAW_FINISH_CALLBACK

    lcd->attachDrawBitmapFinishCallback(
        onLCD_DrawFinishCallback
    );

#endif

    lcd->reset();

    assert(lcd->begin());

    if (lcd->getBasicAttributes()
            .basic_bus_spec
            .isFunctionValid(
                LCD::BasicBusSpecification::FUNC_DISPLAY_ON_OFF
            ))
    {
        lcd->setDisplayOnOff(true);
    }

    Serial.println("LCD initialized!");
}


/* =========================================================
   SIMPLE 5x7 FONT
   ========================================================= */

static const uint8_t font5x7[][5] =
{
    // SPACE
    {0x00,0x00,0x00,0x00,0x00},

    // A
    {0x7E,0x11,0x11,0x11,0x7E},

    // B
    {0x7F,0x49,0x49,0x49,0x36},

    // C
    {0x3E,0x41,0x41,0x41,0x22},

    // D
    {0x7F,0x41,0x41,0x22,0x1C},

    // E
    {0x7F,0x49,0x49,0x49,0x41},

    // F
    {0x7F,0x09,0x09,0x09,0x01},

    // G
    {0x3E,0x41,0x49,0x49,0x7A},

    // H
    {0x7F,0x08,0x08,0x08,0x7F},

    // I
    {0x00,0x41,0x7F,0x41,0x00},

    // J
    {0x20,0x40,0x41,0x3F,0x01},

    // K
    {0x7F,0x08,0x14,0x22,0x41},

    // L
    {0x7F,0x40,0x40,0x40,0x40},

    // M
    {0x7F,0x02,0x0C,0x02,0x7F},

    // N
    {0x7F,0x04,0x08,0x10,0x7F},

    // O
    {0x3E,0x41,0x41,0x41,0x3E},

    // P
    {0x7F,0x09,0x09,0x09,0x06},

    // Q
    {0x3E,0x41,0x51,0x21,0x5E},

    // R
    {0x7F,0x09,0x19,0x29,0x46},

    // S
    {0x46,0x49,0x49,0x49,0x31},

    // T
    {0x01,0x01,0x7F,0x01,0x01},

    // U
    {0x3F,0x40,0x40,0x40,0x3F},

    // V
    {0x1F,0x20,0x40,0x20,0x1F},

    // W
    {0x3F,0x40,0x38,0x40,0x3F},

    // X
    {0x63,0x14,0x08,0x14,0x63},

    // Y
    {0x07,0x08,0x70,0x08,0x07},

    // Z
    {0x61,0x51,0x49,0x45,0x43}
};


/* =========================================================
   PRINT TEXT ON LCD
   ========================================================= */

void lcd_print_text(const char *text)
{
    if (lcd == nullptr)
    {
        Serial.println("ERROR: LCD is not initialized!");
        return;
    }

    const int width  = EXAMPLE_LCD_WIDTH;
    const int height = EXAMPLE_LCD_HEIGHT;

    /*
       800 x 480 x 2 bytes
       = 768000 bytes
    */

    static uint8_t *buffer = nullptr;

    if (buffer == nullptr)
    {
        buffer = (uint8_t *)heap_caps_malloc(
            width * height * sizeof(uint16_t),
            MALLOC_CAP_SPIRAM
        );

        if (buffer == nullptr)
        {
            Serial.println("ERROR: LCD buffer allocation failed!");
            return;
        }

        Serial.println("LCD framebuffer allocated.");
    }

    /*
       Treat the byte buffer as RGB565 pixels.
    */

    uint16_t *pixelBuffer =
        (uint16_t *)buffer;

    /*
       Clear screen.
    */

    memset(
        buffer,
        0,
        width * height * sizeof(uint16_t)
    );

    /*
       Text position.
    */

    int x = 40;
    int y = 200;

    while (*text)
    {
        char c = *text++;

        /*
           Convert lowercase letters to uppercase.
        */

        if (c >= 'a' && c <= 'z')
        {
            c = c - 'a' + 'A';
        }

        /*
           A-Z
        */

        if (c >= 'A' && c <= 'Z')
        {
            const uint8_t *glyph =
                font5x7[c - 'A' + 1];

            for (int col = 0; col < 5; col++)
            {
                for (int row = 0; row < 7; row++)
                {
                    if (glyph[col] & (1 << row))
                    {
                        for (int dx = 0; dx < 6; dx++)
                        {
                            for (int dy = 0; dy < 6; dy++)
                            {
                                int px =
                                    x + col * 6 + dx;

                                int py =
                                    y + row * 6 + dy;

                                if (px >= 0 &&
                                    px < width &&
                                    py >= 0 &&
                                    py < height)
                                {
                                    pixelBuffer[
                                        py * width + px
                                    ] = 0xFFFF;
                                }
                            }
                        }
                    }
                }
            }

            x += 36;
        }

        /*
           Space.
        */

        else if (c == ' ')
        {
            x += 20;
        }

        /*
           Unsupported character.
        */

        else
        {
            x += 20;
        }

        if (x > width - 40)
            break;
    }

    /*
       Send framebuffer to LCD.
    */

    lcd->drawBitmap(
        0,
        0,
        width,
        height,
        buffer
    );
}