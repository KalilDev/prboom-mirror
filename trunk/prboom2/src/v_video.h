/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 * $Id: v_video.h,v 1.19 2002/11/22 21:57:12 dukope Exp $
 *
 *  PrBoom a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Gamma correction LUT.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"
#include "doomdef.h"
// Needed because we are refering to patches.
#include "r_data.h"
#include "r_draw.h"

//
// VIDEO
//

#define CENTERY     (SCREENHEIGHT/2)

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

// array of pointers to color translation tables
extern const byte *colrngs[];

// symbolic indices into color translation table pointer array
typedef enum
{
  CR_BRICK,   //0
  CR_TAN,     //1
  CR_GRAY,    //2
  CR_GREEN,   //3
  CR_BROWN,   //4
  CR_GOLD,    //5
  CR_RED,     //6
  CR_BLUE,    //7
  CR_ORANGE,  //8
  CR_YELLOW,  //9
  CR_BLUE2,   //10 // proff
  CR_LIMIT    //11 //jff 2/27/98 added for range check
} crange_idx_e;
//jff 1/16/98 end palette color range additions

#define CR_DEFAULT CR_RED   /* default value for out of range colors */

typedef struct {
  void *data; // pointer to the screen content
  boolean not_on_heap; // if set, no malloc or free is preformed and
                       // data never set to NULL. Used i.e. with SDL doublebuffer.
  int width;           // the width of the surface, used when mallocing
  int height;          // the height of the surface, used when mallocing
} TScreenVars;

#define NUM_SCREENS 6
extern TScreenVars screens[NUM_SCREENS];
extern int         usegamma;

//jff 4/24/98 loads color translation lumps
void V_InitColorTranslation(void);

// Allocates buffer screens, call before R_Init.
void V_Init (void);

enum patch_translation_e {
  VPT_NONE    = 0, // Normal
  VPT_FLIP    = 1, // Flip image horizontally
  VPT_TRANS   = 2, // Translate image via a translation table
  VPT_STRETCH = 4, // Stretch to compensate for high-res
};

//---------------------------------------------------------------------------
// Varying bit-depth support -POPE
//---------------------------------------------------------------------------
// For bilinear filtering, each palette color is pre-weighted and put in a
// table for fast blending operations. These macros decide how many weights
// to create for each color. The lower the number, the lower the blend
// accuracy, which can produce very bad artifacts in texture filtering.
#define VID_NUMCOLORWEIGHTS 64
#define VID_COLORWEIGHTMASK (VID_NUMCOLORWEIGHTS-1)
#define VID_COLORWEIGHTBITS 6

//---------------------------------------------------------------------------
// The available bit-depth modes
typedef enum {
  VID_MODE8   = 0,
  VID_MODE16  = 1,
  VID_MODE32  = 2,
  VID_MODEGL  = 3
} TVidMode;

//---------------------------------------------------------------------------
// Vid mode set interface. This should only be called once per program
// execution, since other systems query the mode only once at the start
void vid_initMode(TVidMode vd);

// Vid mode query interface
TVidMode vid_getMode();
int vid_getNumBits();
int vid_getDepth();

// general mode funcs
TVidMode vid_getModeForNumBits(int numBits);
int vid_getModePixelDepth(TVidMode mode);

//---------------------------------------------------------------------------
// Palettes for converting from 8 bit color to 16 and 32 bit. Also
// contains the weighted versions of each palette color for filtering
// operations
//extern int vid_intPalette[256][VID_NUMCOLORWEIGHTS];
//extern short vid_shortPalette[256][VID_NUMCOLORWEIGHTS];
extern int *vid_intPalette;
extern short *vid_shortPalette;

#define VID_INTPAL(color, weight) vid_intPalette[ (color)*VID_NUMCOLORWEIGHTS + (weight) ]
#define VID_SHORTPAL(color, weight) vid_shortPalette[ (color)*VID_NUMCOLORWEIGHTS + (weight) ]

//---------------------------------------------------------------------------
void V_UpdateTrueColorPalette(TVidMode mode);
void V_DestroyTrueColorPalette(TVidMode mode);
void V_DestroyUnusedTrueColorPalettes();

//---------------------------------------------------------------------------
typedef struct {
  int left, top, right, bottom;
} TPlotRect;

//---------------------------------------------------------------------------
// Since we now have many different bit-depth functions to select from,
// We use function pointers to preselect the correct ones for the chosen
// video mode for speed and ease-of-use/compatibility reasons
//---------------------------------------------------------------------------
// V_CopyRect
typedef void (__cdecl *TFunc_V_CopyRect)(int,int,int,int,int,int,int,int, enum patch_translation_e);
extern TFunc_V_CopyRect V_CopyRect;

// V_FillRect
typedef void (__cdecl *TFunc_V_FillRect)(int,int,int,int,int,byte);
extern TFunc_V_FillRect V_FillRect;

// V_DrawMemPatch
typedef void (__cdecl *TFunc_V_DrawMemPatch)(int,int,int,const patch_t *,int,enum patch_translation_e);
extern TFunc_V_DrawMemPatch V_DrawMemPatch;

// V_DrawNumPatch
typedef void (__cdecl *TFunc_V_DrawNumPatch)(int,int,int,int,int,enum patch_translation_e);
extern TFunc_V_DrawNumPatch V_DrawNumPatch;

#define V_DrawNamePatch(x,y,s,n,t,f) V_DrawNumPatch(x,y,s,W_GetNumForName(n),t,f)

// V_DrawBlock
typedef void (__cdecl *TFunc_V_DrawBlock)(int,int,int,int,int,const byte*,enum patch_translation_e);
extern TFunc_V_DrawBlock V_DrawBlock;

// V_DrawBackground
typedef void (__cdecl *TFunc_V_DrawBackground)(const char*,int);
extern TFunc_V_DrawBackground V_DrawBackground;

// V_PlotPixel
typedef void (__cdecl *TFunc_V_PlotPixel)(int,int,int,byte);
extern TFunc_V_PlotPixel V_PlotPixel;

// V_PlotPatch
typedef void (__cdecl *TFunc_V_PlotPatch)(const patch_t*,TPlotRect,const TPlotRect,TRDrawFilterType,const byte*,byte*,int,int);
extern TFunc_V_PlotPatch V_PlotPatch;

// V_PlotPatchNum
typedef void (__cdecl *TFunc_V_PlotPatchNum)(int,TPlotRect,const TPlotRect,TRDrawFilterType,const byte*,byte*,int,int);
extern TFunc_V_PlotPatchNum V_PlotPatchNum;

// V_PlotTextureNum
typedef void (__cdecl *TFunc_V_PlotTextureNum)(int,int,int,int,int,TRDrawFilterType,byte*,int,int);
extern TFunc_V_PlotTextureNum V_PlotTextureNum;

// V_PatchToBlock
byte *V_PatchToBlock(const char* name, int cm, enum patch_translation_e flags, unsigned short* width, unsigned short* height);

//---------------------------------------------------------------------------
// These functions are now bit-depth aware
void V_AllocScreen(TScreenVars *scrn);
void V_AllocScreens();
void V_FreeScreen(TScreenVars *scrn);
void V_FreeScreens();
void V_SetPalette(int pal);
void V_MarkRect(int x, int y, int width, int height);
//---------------------------------------------------------------------------

/* cph -
 * Functions to return width & height of a patch.
 * Doesn't really belong here, but is often used in conjunction with
 * this code.
 */
int V_NumPatchWidth(int lump);
int V_NumPatchHeight(int lump);
#define V_NamePatchWidth(n) V_NumPatchWidth(W_GetNumForName(n))
#define V_NamePatchHeight(n) V_NumPatchHeight(W_GetNumForName(n))

//---------------------------------------------------------------------------
// These functions use the R_DrawColumn* pipelines to plot filtered
// patches and textures to a destination buffer
//---------------------------------------------------------------------------
byte *V_GetPlottedPatch32(
  int patchNum, int plotWidth, int plotHeight, 
  int bufferWidth, int bufferHeight, 
  TRDrawFilterType filter, const byte *colorTranslationTable, int convertToBGRA
);
byte *V_GetPlottedPatch8(
  int patchNum, int plotWidth, int plotHeight, 
  int bufferWidth, int bufferHeight,
  TRDrawFilterType filter, const byte *colorTranslationTable, byte clearColor
);

byte *V_GetPlottedTexture32(
  int textureNum, int plotWidth, int plotHeight, 
  int bufferWidth, int bufferHeight, 
  TRDrawFilterType filter, int convertToBGRA
);
byte *V_GetPlottedTexture8(
  int textureNum, int plotWidth, int plotHeight, 
  int bufferWidth, int bufferHeight, 
  TRDrawFilterType filter, byte clearColor
);

//---------------------------------------------------------------------------
// Font
//---------------------------------------------------------------------------
/* font colours (CR_ colors + 0x80 as char) */
#define FC_BASEVALUE     0x80
#define FC_BRICK        "\x80"
#define FC_TAN          "\x81"
#define FC_GRAY         "\x82"
#define FC_GREEN        "\x83"
#define FC_BROWN        "\x84"
#define FC_GOLD         "\x85"
#define FC_RED          "\x86"
#define FC_BLUE         "\x87"
#define FC_ORANGE       "\x88"
#define FC_YELLOW       "\x89"
#define FC_BLUE2        "\x8a"

void V_WriteText(unsigned char *s, int x, int y, int gap);
void V_WriteTextColoured(unsigned char *s, int colour, int x, int y, int gap);
int V_StringWidth(unsigned char *s, int gap);
int V_StringHeight(unsigned char *s);
//---------------------------------------------------------------------------


#ifdef GL_DOOM
#include "gl_struct.h"
#endif

#endif
