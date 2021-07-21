#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define SCREEN_HEIGHT 272
#define SCREEN_WIDTH 480
#define BUFFER_SIZE SCREEN_HEIGHT *SCREEN_WIDTH * 4

// In memory buffer, is copied to /dev/fb0 on draw
static uint8_t buffer[BUFFER_SIZE];

struct Rgb565
{
	uint8_t b1; // High-order byte of rgb565 format
	uint8_t b2; // Low-order byte of rgb565 format
};

// ROYGBIV used to make a pattern in our buffer
struct Rgb565 colors[7];

static struct Rgb565 rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	// store r/g/b as a ratio from 0 to 1
	float r_r, r_g, r_b;
	r_r = (float)r / 255.0;
	r_g = (float)g / 255.0;
	r_b = (float)b / 255.0;

	uint8_t lim5 = 32;
	uint8_t lim6 = 64;
	uint8_t mask5 = lim5 - 1;
	uint8_t mask6 = lim6 - 1;

	uint8_t r5, g6, b5;
	r5 = (uint8_t)(r_r * lim5);
	r5 = r5 > mask5 ? mask5 : r5 & mask5;

	g6 = (uint8_t)(r_g * lim6);
	g6 = g6 > mask6 ? mask6 : g6 & mask6;

	b5 = (uint8_t)(r_b * lim5);
	b5 = b5 > mask5 ? mask5 : b5 & mask5;
	//printf("DEBUG %i, %i, %i, %i, %i\n", r5, g6, b5, mask5, mask6);

	uint8_t g6_l3 = g6 & 0b00000111;		// green lower3 bits
	uint8_t g6_h3 = (g6 >> 3) & 0b00000111; // green higher3 bits

	struct Rgb565 rgb565 = {};
	rgb565.b2 = (r5 << 3) | g6_h3;
	rgb565.b1 = (g6_l3 << 5) | b5;
	return rgb565;
}

void test_rgb_conversion_item(char r, char g, char b, char expected_b1, char expected_b2)
{
	struct Rgb565 rgb;
	rgb = rgb_to_rgb565(r, g, b);
	if (rgb.b1 == expected_b1 && rgb.b2 == expected_b2)
	{
		printf("PASS rgb(%i, %i, %i) -> rgb565(0x%02x, 0x%02x)\n", r, g, b, expected_b1, expected_b2);
	}
	else
	{
		printf("FAIL rgb(%i, %i, %i) -> expected rgb565(0x%02x, 0x%02x) but got rgb565(0x%02x, 0x%02x)\n", r, g, b, expected_b1, expected_b2, rgb.b1, rgb.b2);
	}
}

void test_rgb_conversion()
{
	test_rgb_conversion_item(255, 255, 255, 0xFF, 0xFF);
	test_rgb_conversion_item(0, 0, 0, 0x00, 0x00);
	test_rgb_conversion_item(225, 225, 225, 0x1C, 0xE7);
	test_rgb_conversion_item(8, 12, 8, 0x61, 0x08);
	test_rgb_conversion_item(255, 0, 0, 0x00, 0xF8);
	fflush(stdout);
}

void set_pixel(uint32_t x, uint32_t y, struct Rgb565 color)
{
	uint32_t array_offset = x * 2 + (y * 2 * SCREEN_WIDTH);
	buffer[array_offset] = color.b1;
	buffer[array_offset + 1] = color.b2;
	buffer[BUFFER_SIZE / 2 + array_offset] = color.b1;
	buffer[BUFFER_SIZE / 2 + array_offset + 1] = color.b2;
}

void update_buffer(uint32_t loop)
{
	for (uint32_t y = 0; y < SCREEN_HEIGHT; y++)
	{
		for (uint32_t x = 0; x < SCREEN_WIDTH; x++)
		{
			uint32_t speed = 2;
			uint32_t column_width = (SCREEN_WIDTH / 7);
			uint32_t x_offset = (x + loop * speed) % SCREEN_WIDTH;
			int colorIndex = (int)floor(x_offset / column_width) % 7;
			struct Rgb565 color = colors[colorIndex];
			set_pixel(x, y, color);
		}
	}
}

int main()
{
	test_rgb_conversion();

	colors[0] = rgb_to_rgb565(255, 0, 0);	  // Red
	colors[1] = rgb_to_rgb565(255, 165, 0);	  // Orange
	colors[2] = rgb_to_rgb565(255, 255, 0);	  // Yellow
	colors[3] = rgb_to_rgb565(0, 128, 0);	  // Green
	colors[4] = rgb_to_rgb565(0, 0, 255);	  // Blue
	colors[5] = rgb_to_rgb565(75, 0, 130);	  // Indigo
	colors[6] = rgb_to_rgb565(238, 130, 238); // Violet

	FILE *f = fopen("/dev/fb0", "w");
	fpos_t f_initial_pos;
	fgetpos(f, &f_initial_pos);

	uint32_t loop = 0;
	while (1)
	{
		update_buffer(loop);

		fsetpos(f, &f_initial_pos);
		fwrite(buffer, sizeof(buffer), 1, f);
		fflush(f);

		loop++;
		usleep(100);
	}
}
