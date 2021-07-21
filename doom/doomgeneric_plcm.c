
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "doomgeneric.h"
#include "doomkeys.h"
#include <unistd.h>
#include <sys/time.h>
#include "m_config.h"
#include <pthread.h>
#include <sys/poll.h>

uint8_t PlcmBuffer[BUFFER_SIZE];

struct timeval start_time;
//pthread_t thread;
FILE *fb0_file;
fpos_t fb0_initial_pos;

void *buffer_copy(void *ptr);

void PrintDebug()
{
	printf("  PlcmBuffer Init:, fb0_file=%i, BUFFER_SIZE=%i, PlcmBuffer=%i, sizeof(PlcmBuffer)=%i\n", (uint32_t)fb0_file, BUFFER_SIZE, (uint32_t)PlcmBuffer, sizeof(PlcmBuffer));
}

void DG_Init()
{
	gettimeofday(&start_time, 0);

	fb0_file = fopen("/dev/fb0", "w");
	fgetpos(fb0_file, &fb0_initial_pos);

	printf("------------------------------------------------------------------------------\n");
	PrintDebug();
	printf("------------------------------------------------------------------------------\n");

	//pthread_create(&thread, NULL, buffer_copy, NULL);
}

void flush_buffer()
{
	fsetpos(fb0_file, &fb0_initial_pos);
	fwrite(PlcmBuffer, sizeof(PlcmBuffer), 1, fb0_file);
	fflush(fb0_file);
}

void *buffer_copy(void *ptr)
{
	while (1)
	{
		flush_buffer();
	}
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
	//printf("DEBUG %i, %i, %i, %i, %i\n", r5, g6, b5, mask5, mask6);

	uint8_t g6_l3 = g6 & 0b00000111;		// green lower3 bits
	uint8_t g6_h3 = (g6 >> 3) & 0b00000111; // green higher3 bits

	struct Rgb565 rgb565 = {};
	rgb565.b2 = (r5 << 3) | g6_h3;
	rgb565.b1 = (g6_l3 << 5) | b5;
	return rgb565;
}

void DG_SetPixel(int x, int y, uint32_t r, uint32_t g, uint32_t b)
{
	if (x >= DOOMGENERIC_RESX || x < 0 || y >= DOOMGENERIC_RESY || y < 0)
	{
		printf("Error!!\n");
	}

	int array_offset = x * 2 + (y * 2 * DOOMGENERIC_RESX);
	uint8_t prev_b1 = PlcmBuffer[array_offset];
	uint8_t prev_b2 = PlcmBuffer[array_offset + 1];
	struct Rgb565 pixel = rgb_to_rgb565(r, g, b);

	PlcmBuffer[array_offset] = pixel.b1;
	PlcmBuffer[array_offset + 1] = pixel.b2;
	PlcmBuffer[BUFFER_SIZE / 2 + array_offset] = pixel.b1;
	PlcmBuffer[BUFFER_SIZE / 2 + array_offset + 1] = pixel.b2;

	//printf("Set pixel%i(%i, %i) = rgb(%i, %i, %i) = Rgb565(0x%02x, 0x%02x), PrevRgb565(0x%02x, 0x%02x)\n", array_offset, x, y, r, g, b, PlcmBuffer[array_offset].b1, PlcmBuffer[array_offset].b2, prev_b1, prev_b2);
}

void DG_DrawFrame()
{
	flush_buffer();
}

void DG_SleepMs(uint32_t ms)
{
	usleep(1000 * ms);
}

float timedifference_msec(struct timeval t0, struct timeval t1)
{
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

uint32_t DG_GetTicksMs()
{
	struct timeval now;
	gettimeofday(&now, 0);

	float elapsed = timedifference_msec(start_time, now);
	return (uint32_t)elapsed;
}

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

void DG_SetWindowTitle(const char *title)
{
}
