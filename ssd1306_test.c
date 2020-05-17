#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart.h"

#if !defined (SSD1306_I2C) && !defined (SSD1306_SPI)
        #define SSD1306_I2C
#endif

#include "i2c.h"
#include "spi.h"

#include "ssd1306.h"

#define SSD1306_SLAVE_ADDR          0x3C

uint8_t bitmap_test[][32] =
	{
        {
	0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF,
        0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF,
	},
        {
	0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00,
        0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00,
	},
	};


int main(void)
	{
	__attribute__((unused))int option;

        uart_init_baud(); // initialize uart
	printf("UART initialized\n");
	option = getchar();

#ifdef SSD1306_I2C
	// initialize i2c bus
	i2c_conf_bus(TWI_FREQ_100K, TWI_PUR_ON); // initialize i2c
	printf("i2c initialized\n");
	option = getchar();

	// initialize i2c device
	ssd1306_t dev_i2c;
	if (ssd1306_init(&dev_i2c, SSD1306_OLED_WIDTH_128, SSD1306_OLED_HEIGHT_64, SSD1306_BUS_I2C, SSD1306_SLAVE_ADDR, PIN_NOT_USED, PIN_NOT_USED) < 0)
		printf("i2c initialize failed\n");
	ssd1306_clear_buffer();
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
	printf("ssd1306 i2c initialized\n");
	option = getchar();
#endif

#ifdef SSD1306_SPI
	// initialize spi bus
	spi_init();
	printf("spi initialized\n");
	option = getchar();

	// initialize spi device
	ssd1306_t dev_spi;
	if (ssd1306_init(&dev_spi, SSD1306_OLED_WIDTH_128, SSD1306_OLED_HEIGHT_64, SSD1306_BUS_SPI, 0x00, PIN_B1_ARD, PIN_B0_ARD) < 0)
		printf("spi initialize failed\n");
	ssd1306_clear_buffer();
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
	printf("ssd1306 spi initialized\n");
	option = getchar();
#endif


	// display checkerboard pattern
	printf("checkerboard\n");
	ssd1306_clear_buffer();
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 128; j++)
			if (((i%2) && ((j%16)/8)) || (!(i%2) && !((j%16)/8)))
				display_buffer[i][j]  = 0xFF;
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	// ascii char set 5x7
	printf("\n5x7 char set\n");
	ssd1306_clear_buffer();
	for (int i = 0; i < 128; i++)
		{
		int row_index  = (i / 16);
		int col_index  = (i % 16) * 8;
		int font_index = (i + 32) * 5; // start at space (32)

		memcpy_P(&display_buffer[row_index][col_index], &font5x7[font_index], 5);
		}

#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();


	// ascii char set 6x14
	printf("\n6x14 char set\n");
	ssd1306_clear_buffer();
	for (int i = 0; i < 64; i++)
		{
		int row_index  = (i / 16) * 2;
		int col_index  = (i % 16) * 8;
		int font_index = (i + 32) * 12; // start at space (32)

		memcpy_P(&display_buffer[row_index]  [col_index], &font6x14[font_index],   6);
		memcpy_P(&display_buffer[row_index+1][col_index], &font6x14[font_index+6], 6);
		}

#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	// pixel set test
	printf("\npixel test\n");
	ssd1306_clear_buffer();
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	for (uint8_t i = 10; i < 54; i++)
		{
		ssd1306_pixel_set(&dev_i2c, i, i, 1);
		ssd1306_pixel_set(&dev_i2c, (uint8_t)(dev_i2c.oled_seg_max-i), i, 1);
		}
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();
	for (uint8_t i = 10; i < 54; i++)
		{
		ssd1306_pixel_set(&dev_i2c, i, i, 0);
		ssd1306_pixel_set(&dev_i2c, (uint8_t)(dev_i2c.oled_seg_max-i), i, 0);
		}
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	// bitmap test
	printf("\nbitmap test 1\n");
	ssd1306_clear_buffer();
	ssd1306_bitmap(&dev_i2c, bitmap_test[0], bitmap_test[0], 16, 2, 64, 32);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();
	ssd1306_bitmap(&dev_i2c, bitmap_test[1], bitmap_test[1], 16, 2, 64, 32);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();
	ssd1306_bitmap(&dev_i2c, bitmap_test[0], NULL, 16, 2, 64, 32);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	char text1[16];
	ssd1306_clear_buffer();
	printf("\ntext test 1\n");
	snprintf(text1, 16, "Test 1.2.3.4.5.6.7.8");
	ssd1306_text(&dev_i2c, text1,  0, 0, SSD1306_FONT_5X7);
	ssd1306_text(&dev_i2c, text1, 64, 8, SSD1306_FONT_6X14);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, 3, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, 3, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	printf("\ntext test 2\n");
	snprintf(text1, 16, "Test 6.5.4.3.2.1");
	ssd1306_text(&dev_i2c, text1, 64, 40, SSD1306_FONT_5X7);
	ssd1306_text(&dev_i2c, text1,  0, 48, SSD1306_FONT_6X14);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 4, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 4, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	printf("\narea test\n");
	ssd1306_clear_buffer();
	ssd1306_area_set(&dev_i2c, 32, 96, 16, 48, 1);
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	option = getchar();

	printf("\nend program\n");
	ssd1306_clear_buffer();
#ifdef SSD1306_SPI
	ssd1306_display(&dev_spi, 0, dev_spi.oled_page_max, 0, dev_spi.oled_seg_max);
#endif
#ifdef SSD1306_I2C
	ssd1306_display(&dev_i2c, 0, dev_i2c.oled_page_max, 0, dev_i2c.oled_seg_max);
#endif
	return 0;
	}
