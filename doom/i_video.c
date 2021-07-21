// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "z_zone.h"
#include <math.h>

#include "tables.h"
#include "doomkeys.h"

#include "doomgeneric.h"

#include <stdbool.h>
#include <stdlib.h>

#include <fcntl.h>

#include <stdarg.h>

#include <sys/types.h>

//#define CMAP256

struct FB_BitField
{
	uint32_t offset;			/* beginning of bitfield	*/
	uint32_t length;			/* length of bitfield		*/
};

struct FB_ScreenInfo
{
	uint32_t xres;			/* visible resolution		*/
	uint32_t yres;
	uint32_t xres_virtual;		/* virtual resolution		*/
	uint32_t yres_virtual;

	uint32_t bits_per_pixel;		/* guess what			*/

							/* >1 = FOURCC			*/
	struct FB_BitField red;		/* bitfield in s_Fb mem if true color, */
	struct FB_BitField green;	/* else only length is significant */
	struct FB_BitField blue;
	struct FB_BitField transp;	/* transparency			*/
};

static struct FB_ScreenInfo s_Fb;
int fb_scaling = 1;
int usemouse = 0;

struct color {
    uint32_t b:8;
    uint32_t g:8;
    uint32_t r:8;
    uint32_t a:8;
};

static struct color colors[256];

void I_GetEvent(void);

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

// Palette converted to RGB565

static uint16_t rgb565_palette[256];

void cmap_to_rgb565(uint16_t * out, uint8_t * in, int in_pixels)
{
    int i, j;
    struct color c;
    uint16_t r, g, b;

    for (i = 0; i < in_pixels; i++)
    {
        c = colors[*in];
        r = ((uint16_t)(c.r >> 3)) << 11;
        g = ((uint16_t)(c.g >> 2)) << 5;
        b = ((uint16_t)(c.b >> 3)) << 0;
        *out = (r | g | b);

        in++;
        for (j = 0; j < fb_scaling; j++) {
            out++;
        }
    }
}

void cmap_to_fb(uint8_t * out, uint8_t * in, int in_pixels)
{
    int i, j, k;
    struct color c;
    uint32_t pix;
    uint16_t r, g, b;

    for (i = 0; i < in_pixels; i++)
    {
        c = colors[*in];  /* R:8 G:8 B:8 format! */
        r = (uint16_t)(c.r >> (8 - s_Fb.red.length));
        g = (uint16_t)(c.g >> (8 - s_Fb.green.length));
        b = (uint16_t)(c.b >> (8 - s_Fb.blue.length));
        pix = r << s_Fb.red.offset;
        pix |= g << s_Fb.green.offset;
        pix |= b << s_Fb.blue.offset;

        for (k = 0; k < fb_scaling; k++) {
            for (j = 0; j < s_Fb.bits_per_pixel/8; j++) {
                *out = (pix >> (j*8));
                out++;
            }
        }
        in++;
    }
}

void I_InitGraphics (void)
{
    int i;

	memset(&s_Fb, 0, sizeof(struct FB_ScreenInfo));
	s_Fb.xres = DOOMGENERIC_RESX;
	s_Fb.yres = DOOMGENERIC_RESY;
	s_Fb.xres_virtual = s_Fb.xres;
	s_Fb.yres_virtual = s_Fb.yres;
	s_Fb.bits_per_pixel = 32;

	s_Fb.blue.length = 8;
	s_Fb.green.length = 8;
	s_Fb.red.length = 8;
	s_Fb.transp.length = 8;

	s_Fb.blue.offset = 0;
	s_Fb.green.offset = 8;
	s_Fb.red.offset = 16;
	s_Fb.transp.offset = 24;


    printf("I_InitGraphics: framebuffer: x_res: %d, y_res: %d, x_virtual: %d, y_virtual: %d, bpp: %d\n",
            s_Fb.xres, s_Fb.yres, s_Fb.xres_virtual, s_Fb.yres_virtual, s_Fb.bits_per_pixel);

    printf("I_InitGraphics: framebuffer: RGBA: %d%d%d%d, red_off: %d, green_off: %d, blue_off: %d, transp_off: %d\n",
            s_Fb.red.length, s_Fb.green.length, s_Fb.blue.length, s_Fb.transp.length, s_Fb.red.offset, s_Fb.green.offset, s_Fb.blue.offset, s_Fb.transp.offset);

    printf("I_InitGraphics: DOOM screen size: w x h: %d x %d\n", SCREENWIDTH, SCREENHEIGHT);


    i = M_CheckParmWithArgs("-scaling", 1);
    if (i > 0) {
        i = atoi(myargv[i + 1]);
        fb_scaling = i;
        printf("I_InitGraphics: Scaling factor: %d\n", fb_scaling);
    } else {
        fb_scaling = s_Fb.xres / SCREENWIDTH;
        if (s_Fb.yres / SCREENHEIGHT < fb_scaling)
            fb_scaling = s_Fb.yres / SCREENHEIGHT;
        printf("I_InitGraphics: Auto-scaling factor: %d\n", fb_scaling);
    }


    /* Allocate screen to draw to */
	I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);  // For DOOM to draw on

	screenvisible = true;

    extern int I_InitInput(void);
    I_InitInput();
}

void I_ShutdownGraphics (void)
{
	Z_Free (I_VideoBuffer);
}

void I_StartFrame (void)
{

}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

double clamp(double x, double upper, double lower)
{
    if(x < lower) x = lower;
    if(x > upper) x = upper;
    return x;
}

void map_color_from_doom_buffer_scale(int x, int y) {
    float xf = (float)x/ (float)(DOOMGENERIC_RESX - 1);
    float doom_buffer_xf = xf * (float)SCREENWIDTH;
    int doom_buffer_x = (int)floor(doom_buffer_xf);

    float yf = (float)y / (float)(DOOMGENERIC_RESY - 1);
    float doom_buffer_yf = yf * (float)SCREENHEIGHT;
    int doom_buffer_y = (int)floor(doom_buffer_yf);

    int doom_buffer_offset = doom_buffer_x + doom_buffer_y * SCREENWIDTH;

    struct color c = colors[I_VideoBuffer[doom_buffer_offset]];

    uint8_t final_r = (uint8_t)clamp(c.r, 255, 0);
    uint8_t final_g = (uint8_t)clamp(c.g, 255, 0);
    uint8_t final_b = (uint8_t)clamp(c.b, 255, 0);

    DG_SetPixel(x, y, final_r, final_g, final_b);
}

void map_color_from_doom_buffer_interp(int x, int y) {
    float xf = (float)x/ (float)(DOOMGENERIC_RESX - 1);
    float doom_buffer_xf = xf * (float)SCREENWIDTH;
    int doom_buffer_x1 = (int)floor(doom_buffer_xf);
    int doom_buffer_x2 = (int)ceil(doom_buffer_xf);
    float x1_ratio = doom_buffer_xf - (float)doom_buffer_x1;
    float x2_ratio = (float)doom_buffer_x2 - doom_buffer_xf;

    float yf = (float)y / (float)(DOOMGENERIC_RESY - 1);
    float doom_buffer_yf = yf * (float)SCREENHEIGHT;
    int doom_buffer_y1 = (int)floor(doom_buffer_yf);
    int doom_buffer_y2 = (int)ceil(doom_buffer_yf);
    float y1_ratio = doom_buffer_yf - (float)doom_buffer_y1;
    float y2_ratio = (float)doom_buffer_y2 - doom_buffer_yf;

    int x1y1_offset = doom_buffer_x1 + doom_buffer_y1 * SCREENWIDTH;
    int x2y1_offset = doom_buffer_x2 + doom_buffer_y1 * SCREENWIDTH;
    int x1y2_offset = doom_buffer_x1 + doom_buffer_y2 * SCREENWIDTH;
    int x2y2_offset = doom_buffer_x2 + doom_buffer_y2 * SCREENWIDTH;

    float x1y1_weight = (x1_ratio + y1_ratio) / 2;
    float x2y1_weight = (x2_ratio + y1_ratio) / 2;
    float x1y2_weight = (x1_ratio + y2_ratio) / 2;
    float x2y2_weight = (x2_ratio + y2_ratio) / 2;

    struct color x1y1 = colors[I_VideoBuffer[x1y1_offset]];
    struct color x2y1 = colors[I_VideoBuffer[x2y1_offset]];
    struct color x1y2 = colors[I_VideoBuffer[x1y2_offset]];
    struct color x2y2 = colors[I_VideoBuffer[x2y2_offset]];

    int weighted_r = (int)clamp((float)x1y1.r * x1y1_weight
        + (float)x2y1.r * x2y1_weight
        + (float)x1y2.r * x1y2_weight
        + (float)x2y2.r * x2y2_weight, 255, 0);

    int weighted_g = (int)clamp((float)x1y1.g * x1y1_weight
        + (float)x2y1.g * x2y1_weight
        + (float)x1y2.g * x1y2_weight
        + (float)x2y2.g * x2y2_weight, 255, 0);

    int weighted_b = (int)clamp((float)x1y1.b * x1y1_weight
        + (float)x2y1.b * x2y1_weight
        + (float)x1y2.b * x1y2_weight
        + (float)x2y2.b * x2y2_weight, 255, 0);

    DG_SetPixel(x, y, weighted_r, weighted_g, weighted_b);
}

//
// I_FinishUpdate
//

void I_FinishUpdate (void)
{
    // lower-performance, scale video to screen size
    // this isn't recommended because it performs
    // very poorly

    // int x, y;
    // for (y = 0; y < DOOMGENERIC_RESY; y++) {
    //     for(x = 0; x < DOOMGENERIC_RESX; x++) {
    //         map_color_from_doom_buffer_scale(x, y);
    //     }
    // }

    // doom screen is 320x200 and polycom
    // screen is 480x272, so we add
    // offsets to screen coordinates to
    // center doom on the screen.
    // This is the highest performance since
    // it doesn't interpolate pixel values
    // to fill the whole screen
    int x_offset = 80;
    int y_offset = 36;
    int x, y;
    for (y = 0; y < SCREENHEIGHT; y++) {
        for (x = 0; x < SCREENWIDTH; x++) {
            int vb_coord = x + y * SCREENWIDTH;

            int dg_coordx = x + x_offset;
            int dg_coordy = y + y_offset;

            struct color c = colors[I_VideoBuffer[vb_coord]];
            DG_SetPixel(dg_coordx, dg_coordy, c.r, c.g, c.b);
        }
    }

	DG_DrawFrame();
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
#define GFX_RGB565(r, g, b)			((((r & 0xF8) >> 3) << 11) | (((g & 0xFC) >> 2) << 5) | ((b & 0xF8) >> 3))
#define GFX_RGB565_R(color)			((0xF800 & color) >> 11)
#define GFX_RGB565_G(color)			((0x07E0 & color) >> 5)
#define GFX_RGB565_B(color)			(0x001F & color)

void I_SetPalette (byte* palette)
{
	int i;
	//col_t* c;

	//for (i = 0; i < 256; i++)
	//{
	//	c = (col_t*)palette;

	//	rgb565_palette[i] = GFX_RGB565(gammatable[usegamma][c->r],
	//								   gammatable[usegamma][c->g],
	//								   gammatable[usegamma][c->b]);

	//	palette += 3;
	//}


    /* performance boost:
     * map to the right pixel format over here! */

    for (i=0; i<256; ++i ) {
        colors[i].a = 0;
        colors[i].r = gammatable[usegamma][*palette++];
        colors[i].g = gammatable[usegamma][*palette++];
        colors[i].b = gammatable[usegamma][*palette++];
    }
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    printf("I_GetPaletteIndex\n");

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = GFX_RGB565_R(rgb565_palette[i]);
    	color.g = GFX_RGB565_G(rgb565_palette[i]);
    	color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
	DG_SetWindowTitle(title);
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk(void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
