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
   ========================================================= */

#if EXAMPLE_LCD_ENABLE_PRINT_FPS

DRAM_ATTR int frame_count = 0;
DRAM_ATTR int fps = 0;
DRAM_ATTR long start_time = 0;

#endif


// Tells us when the previous bitmap transfer has finished
volatile bool lcdDrawBusy = false;


// ---------------------------------------------------------
// LCD refresh finish callback
// ---------------------------------------------------------

IRAM_ATTR bool onLCD_RefreshFinishCallback(void *user_data)
{
    return false;
}

// ---------------------------------------------------------
// LCD draw finish callback
// ---------------------------------------------------------

IRAM_ATTR bool onLCD_DrawFinishCallback(void *user_data)
{
    lcdDrawBusy = false;
    return false;
}
/* =========================================================
   LCD INITIALIZATION
   ========================================================= */

void waveshare_lcd_init(void)
{
    Serial0.println("Initializing RGB LCD...");

    lcd = create_lcd_without_config();

    if (lcd == nullptr)
    {
        Serial0.println("ERROR: LCD creation failed!");
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

    Serial0.println("LCD initialized!");
}


/* =========================================================
   5x7 FONT
   Supports:
   SPACE
   A-Z
   0-9
   .
   -
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
    {0x61,0x51,0x49,0x45,0x43},

        // 0
    {0x3E,0x51,0x49,0x45,0x3E},

    // 1
    {0x00,0x42,0x7F,0x40,0x00},

    // 2
    {0x42,0x61,0x51,0x49,0x46},

    // 3
    {0x41,0x49,0x49,0x49,0x36},

    // 4
    {0x18,0x14,0x12,0x7F,0x10},

    // 5
    {0x4F,0x49,0x49,0x49,0x31},

    // 6
    {0x3E,0x49,0x49,0x49,0x30},

    // 7
    {0x01,0x71,0x09,0x05,0x03},

    // 8
    {0x36,0x49,0x49,0x49,0x36},

    // 9
    {0x06,0x49,0x49,0x49,0x3E},
    
    // .
    {0x00,0x00,0x00,0x60,0x60},

    // -
    {0x08,0x08,0x08,0x08,0x08}
};


/* =========================================================
   GET CHARACTER GLYPH
   ========================================================= */

static const uint8_t *getGlyph(char c)
{
    // Convert lowercase to uppercase
    if (c >= 'a' && c <= 'z')
    {
        c = c - 'a' + 'A';
    }

    // Space
    if (c == ' ')
    {
        return font5x7[0];
    }

    // A-Z
    if (c >= 'A' && c <= 'Z')
    {
        return font5x7[c - 'A' + 1];
    }

    // 0-9
    if (c >= '0' && c <= '9')
    {
        return font5x7[27 + (c - '0')];
    }

    // Decimal point
    if (c == '.')
    {
        return font5x7[37];
    }

    // Minus sign
    if (c == '-')
    {
        return font5x7[38];
    }

    // Unsupported character
    return font5x7[0];
}

/* =========================================================
   PARTIAL LCD UPDATE
   Static words are drawn once.
   Only the number area is refreshed.
   ========================================================= */

// =========================================================
// LCD SCREEN LAYOUT
// =========================================================

#define COMPANY_X       40
#define COMPANY_Y       60
#define COMPANY_SCALE   5

#define VERNIER_X       40
#define VERNIER_Y       190

#define VERNIER_NUMBER_X       320
#define VERNIER_NUMBER_Y       190
#define VERNIER_NUMBER_WIDTH   230
#define VERNIER_NUMBER_HEIGHT  42

#define MM_X            560
#define MM_Y            190

#define VERNIER_LED_X   700
#define VERNIER_LED_Y   210
#define LED_RADIUS      18

#define WEIGHT_X        40
#define WEIGHT_Y        310

#define WEIGHT_NUMBER_X       320
#define WEIGHT_NUMBER_Y       310
#define WEIGHT_NUMBER_WIDTH   230
#define WEIGHT_NUMBER_HEIGHT  42

#define GM_X            560
#define GM_Y            310

#define WEIGHT_LED_X    700
#define WEIGHT_LED_Y    330

#define GREEN_LED 0x07E0
#define RED_LED   0xF800
// =========================================================
// EDIT THIS TO YOUR COMPANY NAME
// =========================================================

#define COMPANY_NAME "Titanelectric"
static uint8_t *staticBuffer = nullptr;
static uint8_t *vernierNumberBuffer = nullptr;
static uint8_t *weightNumberBuffer = nullptr;

static bool staticScreenDrawn = false;


/* ---------------------------------------------------------
   Draw one character into a buffer
   --------------------------------------------------------- */

static void drawCharToBuffer(
    uint16_t *buffer,
    int bufferWidth,
    int bufferHeight,
    int x,
    int y,
    char c,
    int scale
)
{
    const uint8_t *glyph = getGlyph(c);

    if (glyph == nullptr)
        return;

    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 7; row++)
        {
            if (glyph[col] & (1 << row))
            {
                for (int dx = 0; dx < scale; dx++)
                {
                    for (int dy = 0; dy < scale; dy++)
                    {
                        int px = x + col * scale + dx;
                        int py = y + row * scale + dy;

                        if (px >= 0 &&
                            px < bufferWidth &&
                            py >= 0 &&
                            py < bufferHeight)
                        {
                            buffer[py * bufferWidth + px] = 0xFFFF;
                        }
                    }
                }
            }
        }
    }
}


/* ---------------------------------------------------------
   Draw text into a buffer
   --------------------------------------------------------- */

static void drawTextToBuffer(
    uint16_t *buffer,
    int bufferWidth,
    int bufferHeight,
    const char *text,
    int x,
    int y,
    int scale
)
{
    while (*text)
    {
        char c = *text++;

        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';

        if (c == ' ')
        {
            x += 20;
        }
        else
        {
            drawCharToBuffer(
                buffer,
                bufferWidth,
                bufferHeight,
                x,
                y,
                c,
                scale
            );

            x += 36;
        }
    }
}


/* ---------------------------------------------------------
   Draw STATIC part of screen
   --------------------------------------------------------- */

static void drawFilledCircle(
    uint16_t *buffer,
    int bufferWidth,
    int bufferHeight,
    int centerX,
    int centerY,
    int radius,
    uint16_t color
)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if ((x * x + y * y) <= (radius * radius))
            {
                int px = centerX + x;
                int py = centerY + y;

                if (px >= 0 &&
                    px < bufferWidth &&
                    py >= 0 &&
                    py < bufferHeight)
                {
                    buffer[py * bufferWidth + px] = color;
                }
            }
        }
    }
}

// =========================================================
// STATIC MEASUREMENT SCREEN
// =========================================================

// =========================================================
// STATIC MEASUREMENT SCREEN
// =========================================================

static void drawStaticScreen()
{
    if (lcd == nullptr)
        return;

    const int width = EXAMPLE_LCD_WIDTH;
    const int height = EXAMPLE_LCD_HEIGHT;

    // -----------------------------------------------------
    // Allocate full screen buffer
    // -----------------------------------------------------

    if (staticBuffer == nullptr)
    {
        staticBuffer = (uint8_t *)heap_caps_malloc(
            width * height * sizeof(uint16_t),
            MALLOC_CAP_SPIRAM
        );

        if (staticBuffer == nullptr)
        {
            Serial0.println(
                "ERROR: Static buffer allocation failed!"
            );
            return;
        }
    }

    uint16_t *pixels = (uint16_t *)staticBuffer;

    // Clear screen
    memset(
        staticBuffer,
        0,
        width * height * sizeof(uint16_t)
    );


    // =====================================================
    // COMPANY NAME
    // =====================================================

    drawTextToBuffer(
        pixels,
        width,
        height,
        COMPANY_NAME,
        COMPANY_X,
        COMPANY_Y,
        COMPANY_SCALE
    );


    // =====================================================
    // VERNIER LABEL
    // =====================================================

    drawTextToBuffer(
        pixels,
        width,
        height,
        "VERNIER",
        VERNIER_X,
        VERNIER_Y,
        6
    );


    // =====================================================
    // MM
    // =====================================================

    drawTextToBuffer(
        pixels,
        width,
        height,
        "MM",
        MM_X,
        MM_Y,
        6
    );


    // =====================================================
    // WEIGHT LABEL
    // =====================================================

    drawTextToBuffer(
        pixels,
        width,
        height,
        "WEIGHT",
        WEIGHT_X,
        WEIGHT_Y,
        6
    );


    // =====================================================
    // GM
    // =====================================================

    drawTextToBuffer(
        pixels,
        width,
        height,
        "GM",
        GM_X,
        GM_Y,
        6
    );


    // =====================================================
    // GREEN VERNIER LED
    // =====================================================

    drawFilledCircle(
        pixels,
        width,
        height,
        VERNIER_LED_X,
        VERNIER_LED_Y,
        LED_RADIUS,
        GREEN_LED
    );


    // =====================================================
    // RED WEIGHT LED
    // =====================================================

    drawFilledCircle(
        pixels,
        width,
        height,
        WEIGHT_LED_X,
        WEIGHT_LED_Y,
        LED_RADIUS,
        RED_LED
    );


    // =====================================================
    // SEND STATIC SCREEN TO LCD
    // =====================================================

    lcdDrawBusy = true;

    lcd->drawBitmap(
        0,
        0,
        width,
        height,
        staticBuffer
    );

    staticScreenDrawn = true;

    Serial0.println(
        "Measurement screen drawn."
    );
}
/* ---------------------------------------------------------
   Draw ONLY the changing number
   --------------------------------------------------------- */

static void drawVernierNumber(float value)
{
    if (lcd == nullptr)
        return;

    if (lcdDrawBusy)
        return;

    if (vernierNumberBuffer == nullptr)
    {
        vernierNumberBuffer = (uint8_t *)heap_caps_malloc(
            VERNIER_NUMBER_WIDTH *
            VERNIER_NUMBER_HEIGHT *
            sizeof(uint16_t),
            MALLOC_CAP_SPIRAM
        );

        if (vernierNumberBuffer == nullptr)
        {
            Serial0.println(
                "ERROR: Vernier buffer allocation failed!"
            );
            return;
        }
    }

    uint16_t *pixels =
        (uint16_t *)vernierNumberBuffer;

    memset(
        vernierNumberBuffer,
        0,
        VERNIER_NUMBER_WIDTH *
        VERNIER_NUMBER_HEIGHT *
        sizeof(uint16_t)
    );

    char numberText[20];

    snprintf(
        numberText,
        sizeof(numberText),
        "%.2f",
        value
    );

    drawTextToBuffer(
        pixels,
        VERNIER_NUMBER_WIDTH,
        VERNIER_NUMBER_HEIGHT,
        numberText,
        0,
        0,
        6
    );

    lcdDrawBusy = true;

    lcd->drawBitmap(
        VERNIER_NUMBER_X,
        VERNIER_NUMBER_Y,
        VERNIER_NUMBER_WIDTH,
        VERNIER_NUMBER_HEIGHT,
        vernierNumberBuffer
    );
}
static void drawWeightNumber(float value)
{
    if (lcd == nullptr)
        return;

    if (lcdDrawBusy)
        return;

    if (weightNumberBuffer == nullptr)
    {
        weightNumberBuffer = (uint8_t *)heap_caps_malloc(
            WEIGHT_NUMBER_WIDTH *
            WEIGHT_NUMBER_HEIGHT *
            sizeof(uint16_t),
            MALLOC_CAP_SPIRAM
        );

        if (weightNumberBuffer == nullptr)
        {
            Serial0.println(
                "ERROR: Weight buffer allocation failed!"
            );
            return;
        }
    }

    uint16_t *pixels =
        (uint16_t *)weightNumberBuffer;

    memset(
        weightNumberBuffer,
        0,
        WEIGHT_NUMBER_WIDTH *
        WEIGHT_NUMBER_HEIGHT *
        sizeof(uint16_t)
    );

    char numberText[20];

    snprintf(
        numberText,
        sizeof(numberText),
        "%.0f",
        value
    );

    drawTextToBuffer(
        pixels,
        WEIGHT_NUMBER_WIDTH,
        WEIGHT_NUMBER_HEIGHT,
        numberText,
        0,
        0,
        6
    );

    lcdDrawBusy = true;

    lcd->drawBitmap(
        WEIGHT_NUMBER_X,
        WEIGHT_NUMBER_Y,
        WEIGHT_NUMBER_WIDTH,
        WEIGHT_NUMBER_HEIGHT,
        weightNumberBuffer
    );
}

/* =========================================================
                        LCD PRINT TEXT
   ========================================================= */
void lcd_print_text(const char *text)
{
    // Kept for compatibility with waveshare_lcd_port.h
    // The new receiver will update the LCD directly.

    if (lcd == nullptr)
    {
        Serial0.println("ERROR: LCD is not initialized!");
        return;
    }

    Serial0.print("LCD text: ");
    Serial0.println(text);
}
/* =========================================================
   UPDATE BOTH VERNIER AND WEIGHT
   ========================================================= */

/* =========================================================
   UPDATE BOTH VERNIER AND WEIGHT
   ========================================================= */

void lcd_update_measurements(
    float vernierMM,
    float weightGM
)
{
    if (lcd == nullptr)
    {
        Serial0.println(
            "ERROR: LCD is not initialized!"
        );
        return;
    }

    // -----------------------------------------------------
    // Draw fixed screen once
    // -----------------------------------------------------

    if (!staticScreenDrawn)
    {
        drawStaticScreen();

        // Wait for full-screen transfer
        while (lcdDrawBusy)
        {
            delay(1);
        }
    }


    // -----------------------------------------------------
    // Wait for LCD to be free
    // -----------------------------------------------------

    while (lcdDrawBusy)
    {
        delay(1);
    }


    // -----------------------------------------------------
    // Update Vernier
    // -----------------------------------------------------

    drawVernierNumber(vernierMM);

    // Wait until Vernier transfer finishes
    while (lcdDrawBusy)
    {
        delay(1);
    }


    // -----------------------------------------------------
    // Update Weight
    // -----------------------------------------------------

    drawWeightNumber(weightGM);

    // Wait until Weight transfer finishes
    while (lcdDrawBusy)
    {
        delay(1);
    }
}