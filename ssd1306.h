#ifndef SSD1306_H_
#define SSD1306_H_

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stddef.h>

#include "pin.h"

extern const uint8_t PROGMEM font5x7[];
extern const uint8_t PROGMEM font6x14[];

// bus types
#define SSD1306_BUS_I2C 0
#define SSD1306_BUS_SPI 1

// display structure
typedef struct ssd1306
	{
	uint8_t bus_type;
	uint8_t i2c_addr;
	pin_t   spi_reset;
	pin_t   spi_dc;

//	int oled_width;
//	int oled_height;
//	int oled_pages;
//	int oled_seg_max;
//	int oled_page_max;
	} ssd1306_t;

// oled size
#define SSD1306_OLED_WIDTH    128
#define SSD1306_OLED_HEIGHT    64
#define SSD1306_OLED_PAGES    (SSD1306_OLED_HEIGHT / 8)
#define SSD1306_SEG_MAX       (SSD1306_OLED_WIDTH - 1)
#define SSD1306_PAGE_MAX      (SSD1306_OLED_PAGES - 1)

// display buffer array
union ssd1306_buffer
	{
	uint8_t dim_one[SSD1306_OLED_PAGES * SSD1306_OLED_WIDTH];       // dim_one dimensional buffer
	uint8_t dim_two[SSD1306_OLED_PAGES] [SSD1306_OLED_WIDTH];       // dim_two dimensional buffer (row x col)
	};

extern union ssd1306_buffer display_buffer;
extern size_t display_buffer_size;


// data/command select
#define SSD1306_DC_CMD              0x00
#define SSD1306_DC_DATA             0x40

// included fonts
#define SSD1306_FONT_5X7   0x01
#define SSD1306_FONT_6X14  0x02

// prototypes
void ssd1306_send(ssd1306_t *dev, uint8_t *data, size_t size, uint8_t dc_flag);
void ssd1306_init(ssd1306_t *dev);
void ssd1306_display(ssd1306_t *dev, uint8_t start_page, uint8_t end_page, uint8_t start_seg, uint8_t end_seg);

void ssd1306_clear_all(void);
void ssd1306_area_set(uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y, uint8_t pixel_value);
void ssd1306_pixel_set(uint8_t pixel_x, uint8_t pixel_y, uint8_t pixel_value);
void ssd1306_text(char *text, uint8_t pixel_x, uint8_t pixel_y, uint8_t font);

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
