#ifndef SSD1306_H_
#define SSD1306_H_

#include <avr/pgmspace.h>
#include <stdint.h>

#include "pin.h"

// built-in fonts
extern const uint8_t PROGMEM font5x7[];
extern const uint8_t PROGMEM font6x14[];

// display sizes
#define SSD1306_OLED_WIDTH_MAX    128
#define SSD1306_OLED_HEIGHT_MAX   64
#define SSD1306_OLED_WIDTH_128    128
#define SSD1306_OLED_HEIGHT_64    64
#define SSD1306_OLED_HEIGHT_32    32

// bus types
#define SSD1306_BUS_I2C 0
#define SSD1306_BUS_SPI 1

// valid flag values
#define DEV_VALID   0xFF
#define DEV_INVALID 0xFE

// display device structure
typedef struct ssd1306
	{
	uint8_t valid_flag;
	uint8_t bus_type;
	uint8_t i2c_addr;
	pin_t   reset_pin;
	pin_t   dc_pin;
	uint8_t oled_width;
	uint8_t oled_height;
	uint8_t oled_seg_max;
	uint8_t oled_page_max;
	} ssd1306_t;

// display buffer array
extern uint8_t display_buffer[(SSD1306_OLED_HEIGHT_MAX / 8)] [SSD1306_OLED_WIDTH_MAX];

// data/command select
#define SSD1306_DC_CMD     0x00
#define SSD1306_DC_DATA    0x40

// included fonts
#define SSD1306_FONT_5X7   0x01
#define SSD1306_FONT_6X14  0x02

// prototypes
int8_t ssd1306_send(ssd1306_t *dev, uint8_t *data, size_t size, uint8_t dc_flag);
int8_t ssd1306_init(ssd1306_t *dev, uint8_t width, uint8_t height, uint8_t bus, uint8_t addr, uint8_t reset_pin, uint8_t dc_pin);
int8_t ssd1306_display(ssd1306_t *dev, uint8_t start_page, uint8_t end_page, uint8_t start_seg, uint8_t end_seg);

void   ssd1306_clear_buffer(void);
int8_t ssd1306_pixel_set(ssd1306_t *dev, uint8_t pixel_x, uint8_t pixel_y, uint8_t pixel_value);
int8_t ssd1306_area_set(ssd1306_t *dev, uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y, uint8_t pixel_value);
int8_t ssd1306_bitmap(ssd1306_t *dev, uint8_t *bitmap, uint8_t *bitmap_mask,
		uint8_t bitmap_seg_size, uint8_t bitmap_page_size, uint8_t start_pixel_x, uint8_t start_pixel_y);
int8_t ssd1306_text(ssd1306_t *dev, char *text, uint8_t start_pixel_x, uint8_t start_pixel_y, uint8_t font);

// ssd1306 commands

// fundamental comands
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF

// timing commands
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9

// display mapping commands
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_SETPAGESTART        0xB0

// addressing commands
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_MODE_HORIZONTAL     0x00
#define SSD1306_MODE_VERTICAL       0x01
#define SSD1306_MODE_PAGE           0x02
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22

// hardware configuration commands
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_COMPINS_SEQ         0x02
#define SSD1306_COMPINS_ALT         0x12
#define SSD1306_COMPINS_DIS         0x02
#define SSD1306_COMPINS_EN          0x22
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_CHARGE_PUMP_ENABLE  0x14
#define SSD1306_CHARGE_PUMP_DISABLE 0x10

// scroll commands
#define SSD1306_ACTIVATE_SCROLL                      0x2F
#define SSD1306_DEACTIVATE_SCROLL                    0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A

#endif // SSD1306_H_
