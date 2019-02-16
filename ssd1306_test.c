#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "uart.h"
#include "i2c.h"
#include "spi.h"
#include "ssd1306.h"

#define SSD1306_SLAVE_ADDR          0x3C

int main(void)
	{
	__attribute__((unused))int option;

        uart_init_115200(); // initialize uart
	printf("UART initialized\n");
	option = getchar();

	// initialize i2c bus
	i2c_conf_bus(TWI_FREQ_100K, TWI_PUR_ON); // initialize i2c
	printf("i2c initialized\n");
	option = getchar();

	// initialize i2c device
	ssd1306_t dev_i2c;
	dev_i2c.bus_type = SSD1306_BUS_I2C;
	dev_i2c.i2c_addr = SSD1306_SLAVE_ADDR;
	ssd1306_init(&dev_i2c);
	printf("ssd1306 i2c initialized\n");
	option = getchar();

	// initialize spi bus
	spi_init();
	printf("spi initialized\n");
	option = getchar();

	// initialize spi device
	ssd1306_t dev_spi;
	dev_spi.bus_type = SSD1306_BUS_SPI;
//	pin_init(&dev_spi.spi_dc,    PIN_B, PB0);
	pin_init_ard(&dev_spi.spi_dc,    8);
//	pin_init(&dev_spi.spi_reset, PIN_B, PB1);
	pin_init_ard(&dev_spi.spi_reset, 9);
	ssd1306_init(&dev_spi);
	printf("ssd1306 spi initialized\n");
	option = getchar();


	// display checkerboard pattern
	printf("checkerboard\n");
	ssd1306_clear_all();
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 128; j++)
			if (((i%2) && ((j%16)/8)) || (!(i%2) && !((j%16)/8)))
				display_buffer.dim_two[i][j]  = 0xFF;
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	// ascii char set 5x7
	printf("\n5x7 char set\n");
	ssd1306_clear_all();
	for (int i = 0; i < 128; i++)
		{
		int row_index  = (i / 16);
		int col_index  = (i % 16) * 8;
		int font_index = (i + 32) * 5; // start at space (32)

		memcpy_P(&display_buffer.dim_two[row_index][col_index], &font5x7[font_index], 5);
		}

	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();


	// ascii char set 6x14
	printf("\n6x14 char set\n");
	ssd1306_clear_all();
	for (int i = 0; i < 64; i++)
		{
		int row_index  = (i / 16) * 2;
		int col_index  = (i % 16) * 8;
		int font_index = (i + 32) * 12; // start at space (32)

		memcpy_P(&display_buffer.dim_two[row_index]  [col_index], &font6x14[font_index],   6);
		memcpy_P(&display_buffer.dim_two[row_index+1][col_index], &font6x14[font_index+6], 6);
		}

	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	printf("\npixel test\n");
	ssd1306_clear_all();
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	for (uint8_t i = 10; i < 54; i++)
		{
		ssd1306_pixel_set(i, i, 1);
		ssd1306_pixel_set((uint8_t)(SSD1306_SEG_MAX-i), i, 1);
		}
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();
	for (uint8_t i = 10; i < 54; i++)
		{
		ssd1306_pixel_set(i, i, 0);
		ssd1306_pixel_set((uint8_t)(SSD1306_SEG_MAX-i), i, 0);
		}
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	printf("\nmap test\n");
	for (int i = 0; i <= 7; i++)
		{
		display_buffer.dim_two  [i][0]   = 0xFF;
		display_buffer.dim_two  [i][127] = 0xFF;

		if (i == 0)
			for (int j = 1; j <= 126; j++)
				display_buffer.dim_two  [i][j]  = 0x01;
		if (i == 7)
			for (int j = 1; j <= 126; j++)
				display_buffer.dim_two  [i][j]  = 0x80;
				
		if (i >= 3 && i <= 4)
			{
			display_buffer.dim_two  [i][61]  = 0xFF;
			display_buffer.dim_two  [i][62]  = 0x81;
			display_buffer.dim_two  [i][63]  = 0x81;
			display_buffer.dim_two  [i][64]  = 0x81;
			display_buffer.dim_two  [i][65]  = 0x81;
			display_buffer.dim_two  [i][66]  = 0x81;
			display_buffer.dim_two  [i][67]  = 0xFF;
			}
		}

	ssd1306_display(&dev_spi, 3, 4, 61, 67);
	ssd1306_display(&dev_i2c, 3, 4, 61, 67);
	option = getchar();
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	char text1[16];
	ssd1306_clear_all();

	printf("\ntext test 1\n");
	snprintf(text1, 16, "Test 1.2.3.4.5.6.7.8");
	ssd1306_text(text1,  0, 0, SSD1306_FONT_5X7);
	ssd1306_text(text1, 64, 8, SSD1306_FONT_6X14);
	ssd1306_display(&dev_spi, 0, 3, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, 3, 0, SSD1306_SEG_MAX);
	option = getchar();

	printf("\ntext test 2\n");
	snprintf(text1, 16, "Test 6.5.4.3.2.1");
	ssd1306_text(text1, 64, 40, SSD1306_FONT_5X7);
	ssd1306_text(text1,  0, 48, SSD1306_FONT_6X14);
	ssd1306_display(&dev_spi, 4, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 4, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	printf("\narea test\n");
	ssd1306_area_set(16, 112, 8, 56, 0);
	ssd1306_display(&dev_spi, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	ssd1306_display(&dev_i2c, 0, SSD1306_PAGE_MAX, 0, SSD1306_SEG_MAX);
	option = getchar();

	printf("\nend program\n");
	return 0;
	}
