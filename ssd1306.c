#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "i2c.h"
#include "spi.h"
#include "ssd1306.h"

#include "font5x7.h"
#include "font6x14.h"

// display buffer array
uint8_t display_buffer[(SSD1306_OLED_HEIGHT_MAX / 8)] [SSD1306_OLED_WIDTH_MAX] = {{0}};

// array of default initialization commands
const uint8_t PROGMEM cmd_tx[] = 
		{
		SSD1306_DISPLAYOFF,
		SSD1306_SETDISPLAYCLOCKDIV, 0x80,
		SSD1306_SETMULTIPLEX, (SSD1306_OLED_HEIGHT_MAX - 1),
		SSD1306_SETDISPLAYOFFSET, 0x00,
		SSD1306_SETSTARTLINE | 0x00,
		SSD1306_CHARGEPUMP, SSD1306_CHARGE_PUMP_ENABLE,
		SSD1306_MEMORYMODE, SSD1306_MODE_HORIZONTAL,
		SSD1306_SEGREMAP | 0x01,
		SSD1306_COMSCANDEC,
		SSD1306_SETCOMPINS, SSD1306_COMPINS_ALT | SSD1306_COMPINS_DIS,
		SSD1306_SETCONTRAST, 0xCF,
		SSD1306_SETPRECHARGE, 0xF1,
		SSD1306_SETVCOMDETECT, 0x40,
		SSD1306_DISPLAYALLON_RESUME,
		SSD1306_NORMALDISPLAY,
//		SSD1306_INVERTDISPLAY,
		SSD1306_DEACTIVATE_SCROLL,
//		SSD1306_COLUMNADDR, 0x00, SSD1306_COL_MAX,
//		SSD1306_PAGEADDR,   0x00, SSD1306_PAGE_MAX,
		SSD1306_SETLOWCOLUMN,
		SSD1306_SETHIGHCOLUMN,
		SSD1306_SETPAGESTART | 0x00,
		SSD1306_DISPLAYON,
		};


//----------------------------------------------------------------------------------------------------
// send to display
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_send(ssd1306_t *dev, uint8_t *data, size_t size, uint8_t dc_flag)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	if (dev->bus_type == SSD1306_BUS_SPI)
		{
		// D/C pin required for spi
		if (dev->dc_pin.valid_flag != PIN_VALID)
			return -1;

		// send via spi bus
		if (dc_flag == SSD1306_DC_DATA)
			pin_state_set(&dev->dc_pin, PIN_OUT_HIGH);                         // data - set D/C pin
		else
			pin_state_set(&dev->dc_pin, PIN_OUT_LOW);                          // command - clear D/C pin
		spi_write(data, size);
		}
	else
		{
		// send via i2c bus
		i2c_master_write(dev->i2c_addr, &dc_flag, 1, I2C_SEQ_START); // send D/C byte
		i2c_master_write(dev->i2c_addr, data, size, I2C_SEQ_STOP);   // send data bytes
		}

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// initialize display
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_init(ssd1306_t *dev, uint8_t width, uint8_t height, uint8_t bus, uint8_t addr, uint8_t reset_pin, uint8_t dc_pin)
	{
	// set device to invalid
	dev->valid_flag = DEV_INVALID;

	// validate bus type
	if ((bus != SSD1306_BUS_I2C) && (bus != SSD1306_BUS_SPI))
		return -1;
	dev->bus_type = bus;

	// validate and save i2c address
	if (dev->bus_type == SSD1306_BUS_I2C)
		{
		if ((addr <= 0x07) || (addr >= 0x78))
			return -1;
		dev->i2c_addr = addr;
		}

	// validate and save size info
	if ((height > SSD1306_OLED_HEIGHT_MAX) || (width > SSD1306_OLED_WIDTH_MAX))
		return -1;
	dev->oled_height   = height;
	dev->oled_width    = width;
	dev->oled_seg_max  = (uint8_t)(dev->oled_width - 1);
	dev->oled_page_max = (uint8_t)((dev->oled_height / 8) - 1);

	// intialize reset and D/C pins
	pin_init_ard(&dev->reset_pin, reset_pin);
	pin_init_ard(&dev->dc_pin, dc_pin);

	// reset ssd1306
	if (dev->reset_pin.valid_flag == PIN_VALID)
		{
		// pull reset low, wait 100 ms, pull reset high
		pin_state_set(&dev->reset_pin, PIN_OUT_LOW);
		_delay_us(100);                // delay 100 us
		pin_state_set(&dev->reset_pin, PIN_OUT_HIGH);
		}

	// D/C pin required for spi device
	if (dev->bus_type == SSD1306_BUS_SPI && dev->dc_pin.valid_flag != PIN_VALID)
			return -1;

	// set device to valid
	dev->valid_flag = DEV_VALID;

	// copy command list from flash 
	uint8_t cmd_array[sizeof cmd_tx];
	memcpy_P(&cmd_array[0], &cmd_tx[0], sizeof cmd_array);

	// send initialize commands to display
	if (ssd1306_send(dev, &cmd_array[0], sizeof cmd_array, SSD1306_DC_CMD))
		return -1;

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// send buffer to display
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_display(ssd1306_t *dev, uint8_t start_page, uint8_t end_page, uint8_t start_seg, uint8_t end_seg)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	// check limits
	if (start_seg  > dev->oled_seg_max ) return -1;
	if (end_seg    > dev->oled_seg_max ) end_seg  = dev->oled_seg_max;
	if (start_page > dev->oled_page_max) return -1;
	if (end_page   > dev->oled_page_max) end_page = dev->oled_page_max;

	// set up display area
	uint8_t ssd_cmd[] = {SSD1306_COLUMNADDR, start_seg, end_seg, SSD1306_PAGEADDR, start_page, end_page};
	if (ssd1306_send(dev, &ssd_cmd[0], sizeof ssd_cmd, SSD1306_DC_CMD))
		return -1;

	// send data to display
	size_t  size = (size_t)((end_seg - start_seg) + 1);
	for (uint8_t i = start_page; i <= end_page; i++)
		if (ssd1306_send(dev, &display_buffer[i][start_seg], size, SSD1306_DC_DATA))
			return -1;

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// clear entire buffer
//----------------------------------------------------------------------------------------------------
void ssd1306_clear_buffer(void)
	{
	// clear display
	memset(&display_buffer[0][0], 0x00, sizeof display_buffer);
	}

//----------------------------------------------------------------------------------------------------
// set pixel at x,y
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_pixel_set(ssd1306_t *dev, uint8_t pixel_x, uint8_t pixel_y, uint8_t pixel_value)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	// check limits
	if ((pixel_x > dev->oled_width) || (pixel_y > dev->oled_height))
		return -1;

	// determine display page and create bit mask
	uint8_t pixel_page = pixel_y / 8;
	uint8_t pixel_pos  = pixel_y % 8;
	uint8_t pixel_bit  = (uint8_t)(1 << pixel_pos);

	// set bit on or off
	if (pixel_value)
		display_buffer[pixel_page][pixel_x] |= pixel_bit;
	else
		display_buffer[pixel_page][pixel_x] &= (uint8_t)~pixel_bit;

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// set pixels in an area
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_area_set(ssd1306_t *dev, uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y, uint8_t pixel_value)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	// check limits
	if (start_x > dev->oled_width-1)  return -1;
	if (end_x   > dev->oled_width-1)  end_x   = (uint8_t)(dev->oled_width-1);
	if (start_y > dev->oled_height-1) return -1;
	if (end_y   > dev->oled_height-1) end_y   = (uint8_t)(dev->oled_height-1);

	// set pixels
	for (uint8_t y = start_y; y <= end_y; y++)
		for (uint8_t x = start_x; x <= end_x; x++)
			if (ssd1306_pixel_set(dev, x, y, pixel_value))
				return -1;

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// map bitmap into display buffer
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_bitmap(ssd1306_t *dev, uint8_t *bitmap, uint8_t *bitmap_mask,
		uint8_t bitmap_seg_size, uint8_t bitmap_page_size, uint8_t start_pixel_x, uint8_t start_pixel_y)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	// loop through bitmap bytes
	for (uint8_t x = 0; x < bitmap_seg_size; x++)
		{
		// calculate x-position
		uint8_t x_pos = (uint8_t)(start_pixel_x + x);
		if (x_pos > (uint8_t)(dev->oled_width-1))
			break;

		// loop through bitmap pages
		for (uint8_t i = 0; i < bitmap_page_size; i++)
			{
			// get bitmap and bitmap_mask bytes
			uint8_t bitmap_byte      = bitmap[x + (i*bitmap_seg_size)];
			uint8_t bitmap_mask_byte = 0xFF;
			if (bitmap_mask != NULL)
				bitmap_mask_byte = bitmap_mask[x + (i*bitmap_seg_size)];

			// loop through bits
			for (uint8_t y = 0; y < 8; y++)
				{
				// calculate y-position of bit
				uint8_t y_pos = (uint8_t)(start_pixel_y + y + (i*8));
				if (y_pos > dev->oled_height-1)
					break;

				// if mask bit set, set pixel from bit
				if ((bitmap_mask_byte & (1 << y)))
					if (ssd1306_pixel_set(dev, x_pos, y_pos, (uint8_t)(bitmap_byte & (1 << y))))
						return -1;
				}
			}
		}

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// map text into display buffer
//----------------------------------------------------------------------------------------------------
int8_t ssd1306_text(ssd1306_t *dev, char *text, uint8_t start_pixel_x, uint8_t start_pixel_y, uint8_t font)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return -1;

	const uint8_t *font_ptr;
	uint8_t font_bytes;
	uint8_t font_segs;
	uint8_t font_pages;

	// set up font values
	if (font == SSD1306_FONT_6X14)
		{
		font_ptr   = &font6x14[0];
		font_bytes = 12;
		font_segs  = 6;
		font_pages = 2;
		}
	else
		{
		font_ptr   = &font5x7[0];
		font_bytes = 5;
		font_segs  = 5;
		font_pages = 1;
		}

	// loop through string characters
	for (char *character = text; (*character != '\0'); character++)
		{
		// get character font bytes from flash
		int font_index = (int)((*character) * font_bytes);
		uint8_t work[font_bytes];
		memcpy_P(&work[0], &font_ptr[font_index], font_bytes);

		// bitmap font into buffer
		if (ssd1306_bitmap(dev, &work[0], &work[0], font_segs, font_pages, start_pixel_x, start_pixel_y))
			return -1;

		// increment to next character display position
		start_pixel_x = (uint8_t)(start_pixel_x + font_segs);
		if (start_pixel_x > (uint8_t)(dev->oled_width-1))
			break;
		}

	return 0;
	}

