
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "doomgeneric.h"
#include "doomkeys.h"
#include <unistd.h>
#include <sys/time.h>
#include "m_config.h"
#include <sys/poll.h>

// In-memory framebuffer, we write pixel
// data to this and once all pixels are
// written this buffer is flushed into /dev/fb0
uint8_t PlcmBuffer[BUFFER_SIZE];

// Time DOOM starts, used to track game ticks
struct timeval start_time;

// File pointer for /dev/fb0 frame buffer
FILE *fb0_file;

// Initial position of /dev/fb0 stream (so we always flush the
// buffer to the beginning of the file
fpos_t fb0_initial_pos;

// Init script is run once during DOOM startup
void DG_Init()
{
	gettimeofday(&start_time, 0);

	fb0_file = fopen("/dev/fb0", "w");
	fgetpos(fb0_file, &fb0_initial_pos);
}

struct Rgb565 rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
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

	uint8_t g6_l3 = g6 & 0b00000111;		// green lower3 bits
	uint8_t g6_h3 = (g6 >> 3) & 0b00000111; // green higher3 bits

	struct Rgb565 rgb565 = {};
	rgb565.b2 = (r5 << 3) | g6_h3;
	rgb565.b1 = (g6_l3 << 5) | b5;
	return rgb565;
}

// Sets the pixel at coordinates `x`, `y`
// to the color defined by r, g, b
// This only modifies the in-memory buffer
// and is flushed to the framebuffer by DG_DrawFrame
void DG_SetPixel(int x, int y, uint32_t r, uint32_t g, uint32_t b)
{
	if (x >= DOOMGENERIC_RESX || x < 0 || y >= DOOMGENERIC_RESY || y < 0)
	{
		printf("Error!!\n");
	}

	struct Rgb565 pixel = rgb_to_rgb565(r, g, b);
	int array_offset = x * 2 + (y * 2 * DOOMGENERIC_RESX);
	PlcmBuffer[array_offset] = pixel.b1;
	PlcmBuffer[array_offset + 1] = pixel.b2;
	PlcmBuffer[BUFFER_SIZE / 2 + array_offset] = pixel.b1;
	PlcmBuffer[BUFFER_SIZE / 2 + array_offset + 1] = pixel.b2;
}

// Writes the in-memory PlcmBuffer
// to /dev/fb0, called at the end
// of the frame after all pixels
// are written to the in-memory buffer
void DG_DrawFrame()
{
	fsetpos(fb0_file, &fb0_initial_pos);
	fwrite(PlcmBuffer, sizeof(PlcmBuffer), 1, fb0_file);
	fflush(fb0_file);
}

// Sleep for the specified amount of time
void DG_SleepMs(uint32_t ms)
{
	usleep(1000 * ms);
}

// Helper method to compare milliseconds between two timevals
float timedifference_msec(struct timeval t0, struct timeval t1)
{
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

// Return the total number of ticks (1-tick = 1 millisecond)
// since DOOM started
uint32_t DG_GetTicksMs()
{
	struct timeval now;
	gettimeofday(&now, 0);

	float elapsed = timedifference_msec(start_time, now);
	return (uint32_t)elapsed;
}

// Maps a polycom keypad driver scancode to DOOM key
static unsigned char convertToDoomKey(unsigned char scancode)
{
	switch (scancode)
	{
	case 0x3c: // Portable headset
		return KEY_ENTER;
	case 0x29: // Home key
		return KEY_ESCAPE;
	case 0x2a: // 7 key
		return KEY_LEFTARROW;
	case 0x38: // 9 key
		return KEY_RIGHTARROW;
	case 0x6f: // 5 key
		return KEY_UPARROW;
	case 0x39: // 8 key
		return KEY_DOWNARROW;
	case 0x37: // Mute key
		return KEY_FIRE;
	case 0x3e: // Speaker phone
		return KEY_USE;
		// case 0xxx: // Not bound
		// 	return KEY_RSHIFT;
	}

	return 0;
}

// Provides input to DOOM game engine
// If any keys have been pressed or released since the last frame, this method
// should return 1. If no key states changed since last frame, this method should
// return 0. This method is called multiple times per frame until all input changes
// have been processed.
// int *pressed -> assign the value in this pointer to 1 if the key was pressed, or 0 if the key was released
// unsigned char *key -> assign the value in this pointer to the doomkey that was pressed
int DG_GetKey(int *pressed, unsigned char *key)
{
	struct pollfd fds;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;
	int ret = poll(&fds, 1, 0);

	if (ret == 1)
	{
		char ch[16];
		read(STDIN_FILENO, &ch, 16);
		char scancode = ch[10];		// Key scancode
		char pressed_down = ch[12]; // 1 if key is pressed down, 0 if key is released
		char doomKey = convertToDoomKey(scancode);
		if (doomKey != 0)
		{
			*pressed = pressed_down;
			*key = doomKey;
			return 1;
		}
	}

	return 0;
}

// Not relevant for doom on the polycom phone.
// Useful for window APIs like X11 to title
// the window doom runs inside
void DG_SetWindowTitle(const char *title)
{
}
