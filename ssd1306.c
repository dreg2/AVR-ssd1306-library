#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "uart.h"
#include "i2c.h"
#include "spi.h"
#include "ssd1306.h"

#include "font5x7.h"
#include "font6x14.h"

// display buffer array
union ssd1306_buffer display_buffer = {{0}};
size_t display_buffer_size = ARRAY_SIZE(display_buffer.dim_one);

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
void ssd1306_send(ssd1306_t *dev, uint8_t *data, size_t size, uint8_t dc_flag)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return;

	if (dev->bus_type == SSD1306_BUS_SPI)
		{
		// D/C pin required for spi
		if (dev->dc_pin.valid_flag != PIN_VALID)
			return;

		// send via spi bus
		if (dc_flag == SSD1306_DC_DATA)
			_SFR_IO8(dev->dc_pin.port_reg) |= dev->dc_pin.pin_mask;            // data - set D/C pin
		else
			_SFR_IO8(dev->dc_pin.port_reg) &= (uint8_t)~dev->dc_pin.pin_mask;  // command - clear D/C pin
		spi_write(data, size);
		}
	else
		{
		// send via i2c bus
		i2c_master_write(dev->i2c_addr, &dc_flag, 1, I2C_SEQ_START); // send D/C byte
		i2c_master_write(dev->i2c_addr, data, size, I2C_SEQ_STOP);   // send data bytes
		}
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
		// set to output
		_SFR_IO8(dev->reset_pin.ddr_reg)  |= dev->reset_pin.pin_mask;

		// pull reset low, wait 100 ms, pull reset high
		_SFR_IO8(dev->reset_pin.port_reg) &= (uint8_t)~dev->reset_pin.pin_mask;
		_delay_us(100);                // delay 100 us
		_SFR_IO8(dev->reset_pin.port_reg) |= dev->reset_pin.pin_mask;
		}

	// initialize spi device
	if (dev->bus_type == SSD1306_BUS_SPI)
		{
		// D/C pin required for spi device
		if (dev->dc_pin.valid_flag != PIN_VALID)
			return -1;

		// set d/c pin to output
		_SFR_IO8(dev->dc_pin.ddr_reg) |= dev->dc_pin.pin_mask;
		}

	// set device to valid
	dev->valid_flag = DEV_VALID;

	// copy command list from flash 
	uint8_t cmd_array[ARRAY_SIZE(cmd_tx)];
	memcpy_P(cmd_array, &cmd_tx[0], ARRAY_SIZE(cmd_tx));

	// send initialize commands to display
	ssd1306_send(dev, &cmd_array[0], ARRAY_SIZE(cmd_tx), SSD1306_DC_CMD);

	return 0;
	}

//----------------------------------------------------------------------------------------------------
// send buffer to display
//----------------------------------------------------------------------------------------------------
void ssd1306_display(ssd1306_t *dev, uint8_t start_page, uint8_t end_page, uint8_t start_seg, uint8_t end_seg)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return;

	// check limits
	if (start_seg  > dev->oled_seg_max ) return;
	if (end_seg    > dev->oled_seg_max ) end_seg  = dev->oled_seg_max;
	if (start_page > dev->oled_page_max) return;
	if (end_page   > dev->oled_page_max) end_page = dev->oled_page_max;

	// set up display area
	uint8_t ssd_cmd[] = {SSD1306_COLUMNADDR, start_seg, end_seg, SSD1306_PAGEADDR, start_page, end_page};
	ssd1306_send(dev, &ssd_cmd[0], ARRAY_SIZE(ssd_cmd), SSD1306_DC_CMD);

	// send data to display
	size_t  size = (size_t)((end_seg - start_seg) + 1);
	for (uint8_t i = start_page; i <= end_page; i++)
		ssd1306_send(dev, &display_buffer.dim_two[i][start_seg], size, SSD1306_DC_DATA);
	}

//----------------------------------------------------------------------------------------------------
// set pixel at x,y
//----------------------------------------------------------------------------------------------------
void ssd1306_pixel_set(ssd1306_t *dev, uint8_t pixel_x, uint8_t pixel_y, uint8_t pixel_value)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return;

	// check limits
	if ((pixel_x > dev->oled_width) || (pixel_y > dev->oled_height))
		return;

	// determine display page and create bit mask
	uint8_t pixel_page = pixel_y / 8;
	uint8_t pixel_pos  = pixel_y % 8;
	uint8_t pixel_bit  = (uint8_t)(1 << pixel_pos);

	// set bit on or off
	if (pixel_value)
		display_buffer.dim_two[pixel_page][pixel_x] |= pixel_bit;
	else
		display_buffer.dim_two[pixel_page][pixel_x] &= (uint8_t)~pixel_bit;
	}

//----------------------------------------------------------------------------------------------------
// clear entire buffer
//----------------------------------------------------------------------------------------------------
void ssd1306_clear_all(void)
	{
	// clear display
	memset(&display_buffer.dim_one[0], 0x00, display_buffer_size);
	}

//----------------------------------------------------------------------------------------------------
// set pixels in an area
//----------------------------------------------------------------------------------------------------
void ssd1306_area_set(ssd1306_t *dev, uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y, uint8_t pixel_value)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return;

	// check limits
	if (start_x > dev->oled_width-1)  return;
	if (end_x   > dev->oled_width-1)  end_x   = (uint8_t)(dev->oled_width-1);
	if (start_y > dev->oled_height-1) return;
	if (end_y   > dev->oled_height-1) end_y   = (uint8_t)(dev->oled_height-1);

	// set pixels
	for (uint8_t y = start_y; y <= end_y; y++)
		for (uint8_t x = start_x; x <= end_x; x++)
			ssd1306_pixel_set(dev, x, y, pixel_value);
	}

//----------------------------------------------------------------------------------------------------
// map text into display buffer
//----------------------------------------------------------------------------------------------------
void ssd1306_text(ssd1306_t *dev, char *text, uint8_t pixel_x, uint8_t pixel_y, uint8_t font)
	{
	// check for valid device
	if (dev->valid_flag != DEV_VALID)
		return;

	const uint8_t *font_ptr;
	uint8_t font_bytes;
	uint8_t font_width;
	uint8_t font_rows;

	// set up font values
	if (font == SSD1306_FONT_6X14)
		{
		font_ptr = &font6x14[0];
		font_bytes  = 12;
		font_width  = 6;
		font_rows   = 2;
		}
	else
		{
		font_ptr = &font5x7[0];
		font_bytes  = 5;
		font_width  = 5;
		font_rows   = 1;
		}

	// loop through string characters
	for (char *character = text; (*character != '\0'); character++)
		{
		int font_index = (int)((*character) * font_bytes);
		uint8_t work[font_bytes];
		memcpy_P(work, &font_ptr[font_index], font_bytes);

		// loop through character font bytes
		for (uint8_t x = 0; x < font_width; x++)
			{
			uint8_t x_pos = (uint8_t)(pixel_x + x);

			if (x_pos > (uint8_t)(dev->oled_width-1))
				break;

			// loop through font rows
			for (uint8_t i = 0; i < font_rows; i++)
				{
				uint8_t font_byte = work[x + (i*font_width)];

				// loop through bits
				for (uint8_t y = 0; y < 8; y++)
					{
					uint8_t y_pos = (uint8_t)(pixel_y + y + (i*8));

					if (y_pos > dev->oled_height-1)
						break;

					// set pixel from bit
					ssd1306_pixel_set(dev, x_pos, y_pos, (uint8_t)(font_byte & (1 << y)));
					}
				}
			}

		// increment to next character display position
		pixel_x = (uint8_t)(pixel_x + font_width);
		if (pixel_x > (uint8_t)(dev->oled_width-1))
			break;
		}
	}

