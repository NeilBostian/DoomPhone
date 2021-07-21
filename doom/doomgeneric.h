#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

#define DOOMGENERIC_RESX 480
#define DOOMGENERIC_RESY 272
#define BUFFER_SIZE DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4

struct Rgb565 {
	uint8_t b1;
	uint8_t b2;
};

struct Rgb565 rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
void DG_Init();
void DG_SetPixel(int x, int y, uint32_t r, uint32_t g, uint32_t b);
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

#endif //DOOM_GENERIC
