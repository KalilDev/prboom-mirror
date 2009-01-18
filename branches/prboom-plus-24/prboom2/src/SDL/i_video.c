/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
 *  DOOM graphics stuff for SDL
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "SDL.h"
//e6y
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#include "m_argv.h"
#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_plane.h"
#include "f_wipe.h"
#include "d_main.h"
#include "d_event.h"
#include "i_joy.h"
#include "i_video.h"
#include "z_zone.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "lprintf.h"

#include "i_simd.h"
#include "r_screenmultiply.h"
#include "e6y.h"//e6y

//e6y: new mouse code
static SDL_Cursor* cursors[2] = {NULL, NULL};

boolean window_focused;

static void ActivateMouse(void);
static void DeactivateMouse(void);
//static int AccelerateMouse(int val);
static void CenterMouse(void);
static void I_ReadMouse(void);
static boolean MouseShouldBeGrabbed();
static void UpdateFocus(void);

int gl_colorbuffer_bits=16;
int gl_depthbuffer_bits=16;

extern void M_QuitDOOM(int choice);
#ifdef DISABLE_DOUBLEBUFFER
int use_doublebuffer = 0;
#else
int use_doublebuffer = 1; // Included not to break m_misc, but not relevant to SDL
#endif
int use_fullscreen;
int desired_fullscreen;
static SDL_Surface *screen;

////////////////////////////////////////////////////////////////////////////
// Input code
int             leds_always_off = 0; // Expected by m_misc, not relevant

// Mouse handling
extern int     usemouse;        // config file var
static boolean mouse_enabled; // usemouse, but can be overriden by -nomouse

int I_GetModeFromString(const char *modestr);

/////////////////////////////////////////////////////////////////////////////////
// Keyboard handling

//
//  Translates the key currently in key
//

static int I_TranslateKey(SDL_keysym* key)
{
  int rc = 0;

  switch (key->sym) {
  case SDLK_LEFT: rc = KEYD_LEFTARROW;  break;
  case SDLK_RIGHT:  rc = KEYD_RIGHTARROW; break;
  case SDLK_DOWN: rc = KEYD_DOWNARROW;  break;
  case SDLK_UP:   rc = KEYD_UPARROW;  break;
  case SDLK_ESCAPE: rc = KEYD_ESCAPE; break;
  case SDLK_RETURN: rc = KEYD_ENTER;  break;
  case SDLK_TAB:  rc = KEYD_TAB;    break;
  case SDLK_F1:   rc = KEYD_F1;   break;
  case SDLK_F2:   rc = KEYD_F2;   break;
  case SDLK_F3:   rc = KEYD_F3;   break;
  case SDLK_F4:   rc = KEYD_F4;   break;
  case SDLK_F5:   rc = KEYD_F5;   break;
  case SDLK_F6:   rc = KEYD_F6;   break;
  case SDLK_F7:   rc = KEYD_F7;   break;
  case SDLK_F8:   rc = KEYD_F8;   break;
  case SDLK_F9:   rc = KEYD_F9;   break;
  case SDLK_F10:  rc = KEYD_F10;    break;
  case SDLK_F11:  rc = KEYD_F11;    break;
  case SDLK_F12:  rc = KEYD_F12;    break;
  case SDLK_BACKSPACE:  rc = KEYD_BACKSPACE;  break;
  case SDLK_DELETE: rc = KEYD_DEL;  break;
  case SDLK_INSERT: rc = KEYD_INSERT; break;
  case SDLK_PAGEUP: rc = KEYD_PAGEUP; break;
  case SDLK_PAGEDOWN: rc = KEYD_PAGEDOWN; break;
  case SDLK_HOME: rc = KEYD_HOME; break;
  case SDLK_END:  rc = KEYD_END;  break;
  case SDLK_PAUSE:  rc = KEYD_PAUSE;  break;
  case SDLK_EQUALS: rc = KEYD_EQUALS; break;
  case SDLK_MINUS:  rc = KEYD_MINUS;  break;
  case SDLK_KP0:  rc = KEYD_KEYPAD0;  break;
  case SDLK_KP1:  rc = KEYD_KEYPAD1;  break;
  case SDLK_KP2:  rc = KEYD_KEYPAD2;  break;
  case SDLK_KP3:  rc = KEYD_KEYPAD3;  break;
  case SDLK_KP4:  rc = KEYD_KEYPAD4;  break;
  case SDLK_KP5:  rc = KEYD_KEYPAD5;  break;
  case SDLK_KP6:  rc = KEYD_KEYPAD6;  break;
  case SDLK_KP7:  rc = KEYD_KEYPAD7;  break;
  case SDLK_KP8:  rc = KEYD_KEYPAD8;  break;
  case SDLK_KP9:  rc = KEYD_KEYPAD9;  break;
  case SDLK_KP_PLUS:  rc = KEYD_KEYPADPLUS; break;
  case SDLK_KP_MINUS: rc = KEYD_KEYPADMINUS;  break;
  case SDLK_KP_DIVIDE:  rc = KEYD_KEYPADDIVIDE; break;
  case SDLK_KP_MULTIPLY: rc = KEYD_KEYPADMULTIPLY; break;
  case SDLK_KP_ENTER: rc = KEYD_KEYPADENTER;  break;
  case SDLK_KP_PERIOD:  rc = KEYD_KEYPADPERIOD; break;
  case SDLK_LSHIFT:
  case SDLK_RSHIFT: rc = KEYD_RSHIFT; break;
  case SDLK_LCTRL:
  case SDLK_RCTRL:  rc = KEYD_RCTRL;  break;
  case SDLK_LALT:
  case SDLK_LMETA:
  case SDLK_RALT:
  case SDLK_RMETA:  rc = KEYD_RALT;   break;
  case SDLK_CAPSLOCK: rc = KEYD_CAPSLOCK; break;
  default:    rc = key->sym;    break;
  }

  return rc;

}

/////////////////////////////////////////////////////////////////////////////////
// Main input code

/* cph - pulled out common button code logic */
//e6y static 
int I_SDLtoDoomMouseState(Uint8 buttonstate)
{
  return 0
      | (buttonstate & SDL_BUTTON(1) ? 1 : 0)
      | (buttonstate & SDL_BUTTON(2) ? 2 : 0)
      | (buttonstate & SDL_BUTTON(3) ? 4 : 0);
}

static void I_GetEvent(SDL_Event *Event)
{
  event_t event;

  switch (Event->type) {
  case SDL_KEYDOWN:
    // e6y
    // Eating of Alt-Tab event to prevent switching to the automap
    // after task switching on Windows
#ifdef _WIN32
    if ((Event->key.keysym.mod & KMOD_LALT) && Event->key.keysym.sym == SDLK_TAB)
      break;
#endif
    event.type = ev_keydown;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
    break;

  case SDL_KEYUP:
  {
    event.type = ev_keyup;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
  }
  break;

  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP:
  if (mouse_enabled && window_focused)
  {
    event.type = ev_mouse;
    event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
  }
  break;

  //e6y: new mouse code
  case SDL_ACTIVEEVENT:
    UpdateFocus();
    break;

  case SDL_QUIT:
    S_StartSound(NULL, sfx_swtchn);
    M_QuitDOOM(0);

  default:
    break;
  }
}


//
// I_StartTic
//

void I_StartTic (void)
{
  SDL_Event Event;

  while ( SDL_PollEvent(&Event) )
    I_GetEvent(&Event);

  //e6y
  I_ReadMouse();

  I_PollJoystick();
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

//
// I_InitInputs
//

static void I_InitInputs(void)
{
  int nomouse_parm = M_CheckParm("-nomouse");

  // check if the user wants to use the mouse
  mouse_enabled = usemouse && !nomouse_parm;
  
  SDL_PumpEvents();

  //e6y
  if (mouse_enabled)
  {
    Uint8 data[1] = {0x00};
    cursors[0] = SDL_GetCursor();
    cursors[1] = SDL_CreateCursor(data, data, 8, 1, 0, 0);

    CenterMouse();

    MouseAccelChanging();
  }

  I_InitJoystick();
}
/////////////////////////////////////////////////////////////////////////////

// I_SkipFrame
//
// Returns true if it thinks we can afford to skip this frame

inline static boolean I_SkipFrame(void)
{
  static int frameno;

  frameno++;
  switch (gamestate) {
  case GS_LEVEL:
    if (!paused)
      return false;
  default:
    // Skip odd frames
    return (frameno & 1) ? true : false;
  }
}

///////////////////////////////////////////////////////////
// Palette stuff.
//
static void I_UploadNewPalette(int pal)
{
  // This is used to replace the current 256 colour cmap with a new one
  // Used by 256 colour PseudoColor modes

  // Array of SDL_Color structs used for setting the 256-colour palette
  static SDL_Color* colours;
  static int cachedgamma;
  static size_t num_pals;

  if (V_GetMode() == VID_MODEGL)
    return;

  if ((colours == NULL) || (cachedgamma != usegamma)) {
    int pplump = W_GetNumForName("PLAYPAL");
    int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
    register const byte * palette = W_CacheLumpNum(pplump);
    register const byte * const gtable = (const byte *)W_CacheLumpNum(gtlump) + 256*(cachedgamma = usegamma);
    register int i;

    num_pals = W_LumpLength(pplump) / (3*256);
    num_pals *= 256;

    if (!colours) {
      // First call - allocate and prepare colour array
      colours = malloc(sizeof(*colours)*num_pals);
    }

    // set the colormap entries
    for (i=0 ; (size_t)i<num_pals ; i++) {
      colours[i].r = gtable[palette[0]];
      colours[i].g = gtable[palette[1]];
      colours[i].b = gtable[palette[2]];
      palette += 3;
    }

    W_UnlockLumpNum(pplump);
    W_UnlockLumpNum(gtlump);
    num_pals/=256;
  }

#ifdef RANGECHECK
  if ((size_t)pal >= num_pals)
    I_Error("I_UploadNewPalette: Palette number out of range (%d>=%d)",
      pal, num_pals);
#endif

  // store the colors to the current display
  // SDL_SetColors(SDL_GetVideoSurface(), colours+256*pal, 0, 256);
  SDL_SetPalette(SDL_GetVideoSurface(),SDL_PHYSPAL,colours+256*pal, 0, 256);
}

//////////////////////////////////////////////////////////////////////////////
// Graphics API

void I_ShutdownGraphics(void)
{
  SDL_FreeCursor(cursors[1]);
  DeactivateMouse();
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
}

//
// I_FinishUpdate
//
static int newpal = 0;
#define NO_PALETTE_CHANGE 1000

void I_FinishUpdate (void)
{
  //e6y: new mouse code
  UpdateGrab();

  if (I_SkipFrame()) return;

#ifdef MONITOR_VISIBILITY
  if (!(SDL_GetAppState()&SDL_APPACTIVE)) {
    return;
  }
#endif

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    // proff 04/05/2000: swap OpenGL buffers
    gld_Finish();
    return;
  }
#endif

  if ((screen_multiply > 1) || SDL_MUSTLOCK(screen)) {
      int h;
      byte *src;
      byte *dest;

      if (SDL_LockSurface(screen) < 0) {
        lprintf(LO_INFO,"I_FinishUpdate: %s\n", SDL_GetError());
        return;
      }

      // e6y: processing of screen_multiply
      if (screen_multiply > 1)
      {
        R_ProcessScreenMultiply(screens[0].data, screen->pixels,
          V_GetPixelDepth(), screens[0].byte_pitch, screen->pitch);
      }
      else
      {
        dest=screen->pixels;
        src=screens[0].data;
        h=screen->h;
        for (; h>0; h--)
        {
          memcpy_fast(dest,src,SCREENWIDTH*V_GetPixelDepth()); //e6y
          dest+=screen->pitch;
          src+=screens[0].byte_pitch;
        }
      }

      SDL_UnlockSurface(screen);
  }

  /* Update the display buffer (flipping video pages if supported)
   * If we need to change palette, that implicitely does a flip */
  if (newpal != NO_PALETTE_CHANGE) {
    I_UploadNewPalette(newpal);
    newpal = NO_PALETTE_CHANGE;
  }
  SDL_Flip(screen);
}

//
// I_ReadScreen
//
void I_ReadScreen (screeninfo_t *dest)
{
  int h;
  boolean locked = false;
  byte *srcofs;
  byte *dstofs;
  int width, height;
  if (SDL_MUSTLOCK(screen))
  {
    if (SDL_LockSurface(screen) < 0) {
      lprintf(LO_INFO,"I_ReadScreen: %s\n", SDL_GetError());
      return;
    }
    locked = true;
  }
  // e6y: processing of screen_multiply
  // screen->pixels instead of screens[0].data should be used
  srcofs = screen->pixels;
  dstofs = dest->data;
  width = MIN(screen->w, dest->width);
  height = MIN(screen->h, dest->height);
  for (h=height; h>0; h--) {
    memcpy(dstofs, srcofs, width);
    srcofs += screens[0].byte_pitch;
    dstofs += dest->byte_pitch;
  }
  if (locked)
  {
    SDL_UnlockSurface(screen);
  }
}

//
// I_SetPalette
//
void I_SetPalette (int pal)
{
  newpal = pal;
}

// I_PreInitGraphics

static void I_ShutdownSDL(void)
{
  SDL_Quit();
  return;
}

void I_PreInitGraphics(void)
{
  int p;

  // Initialize SDL
  unsigned int flags = 0;
  if (!(M_CheckParm("-nodraw") && M_CheckParm("-nosound")))
    flags = SDL_INIT_VIDEO;
#ifdef _DEBUG
  flags |= SDL_INIT_NOPARACHUTE;
#endif

  // e6y: Forcing "directx" video driver for Win9x.
  // The "windib" video driver is the default for SDL > 1.2.9, 
  // to prevent problems with certain laptops, 64-bit Windows, and Windows Vista.  
  // The DirectX driver is still available, and can be selected by setting 
  // the environment variable SDL_VIDEODRIVER to "directx".

  if ((p = M_CheckParm("-videodriver")) && (p < myargc - 1))
  {
    free(sdl_videodriver);
    sdl_videodriver = strdup(myargv[p + 1]);
  }

  if (strcasecmp(sdl_videodriver, "default"))
  {
    // videodriver != default
    char buf[80];
    strcpy(buf, "SDL_VIDEODRIVER=");
    strncat(buf, sdl_videodriver, sizeof(buf) - sizeof(buf[0]) - strlen(buf));
    putenv(buf);
  }
  else
  {
    // videodriver == default
#ifdef _WIN32
    if ((int)GetVersion() < 0 && V_GetMode() != VID_MODEGL ) // win9x
      putenv("SDL_VIDEODRIVER=directx");
#endif
  }

  p = SDL_Init(flags);

  if (p < 0 && strcasecmp(sdl_videodriver, "default"))
  {
    //e6y: wrong videodriver?
    lprintf(LO_ERROR, "Could not initialize SDL with SDL_VIDEODRIVER=%s [%s]\n", sdl_videodriver, SDL_GetError());

    putenv("SDL_VIDEODRIVER=");
    free(sdl_videodriver);
    sdl_videodriver = strdup("default");
    p = SDL_Init(flags);
  }

  if (p < 0)
  {
    I_Error("Could not initialize SDL [%s]", SDL_GetError());
  }

  atexit(I_ShutdownSDL);
}

// e6y: resolution limitation is removed
void I_InitBuffersRes(void)
{
  R_InitMeltRes();
  R_InitSpritesRes();
  R_InitBuffersRes();
  R_InitPlanesRes();
}

// e6y
// GLBoom use this function for trying to set the closest supported resolution if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It should be used only for fullscreen modes.
static void I_ClosestResolution (int *width, int *height, int flags)
{
  SDL_Rect **modes;
  int twidth, theight;
  int cwidth = 0, cheight = 0;
//  int iteration;
  int i;
  unsigned int closest = UINT_MAX;
  unsigned int dist;

  modes = SDL_ListModes(NULL, flags);

  //for (iteration = 0; iteration < 2; iteration++)
  {
    for(i=0; modes[i]; ++i)
    {
      twidth = modes[i]->w;
      theight = modes[i]->h;

      if (twidth == *width && theight == *height)
        return;

      //if (iteration == 0 && (twidth < *width || theight < *height))
      //  continue;

      dist = (twidth - *width) * (twidth - *width) + 
             (theight - *height) * (theight - *height);

      if (dist < closest)
      {
        closest = dist;
        cwidth = twidth;
        cheight = theight;
      }
    }
    if (closest != 4294967295u)
    {
      *width = cwidth;
      *height = cheight;
      return;
    }
  }
}  

int process_affinity_mask;
int try_to_reduce_cpu_cache_misses;

// e6y
// It is a simple test of CPU cache misses.
unsigned int I_TestCPUCacheMisses(int width, int height)
{
  int i, k;
  char *s, *d, *ps, *pd;
  unsigned int tickStart, tickEnd;
  
  s = malloc(width * height);
  d = malloc(width * height);

  tickStart = SDL_GetTicks();
  for(k = 0; k < 1000; k++)
  {
    ps = s;
    pd = d;
    for(i = 0; i < height; i++)
    {
      pd[0] = ps[0];
      pd += width;
      ps += width;
    }
  }
  tickEnd = SDL_GetTicks();

  free(d);
  free(s);

  return (tickEnd - tickStart);
}

// CPhipps -
// I_CalculateRes
// Calculates the screen resolution, possibly using the supplied guide
void I_CalculateRes(unsigned int width, unsigned int height)
{
  // e6y: how about 1680x1050?
  /*
  SCREENWIDTH = (width+3) & ~3;
  SCREENHEIGHT = (height+3) & ~3;
  */

// e6y
// GLBoom will try to set the closest supported resolution 
// if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It affects only fullscreen modes.
  if (V_GetMode() == VID_MODEGL) {
    if ( desired_fullscreen )
    {
      I_ClosestResolution(&width, &height, SDL_OPENGL|SDL_FULLSCREEN);
    }
    SCREENWIDTH = width;
    SCREENHEIGHT = height;
    SCREENPITCH = SCREENWIDTH;
  } else {
    unsigned int t1, t2;
    int pitch1, pitch2;

    SCREENWIDTH = (width+15) & ~15;
    SCREENHEIGHT = height;

    // e6y
    // Trying to optimise screen pitch for reducing of CPU cache misses.
    // It is extremally important for wiping in software.
    // I have ~20x improvement in speed with using 1056 instead of 1024 on Pentium4
    // and only ~10% for Core2Duo
    if (try_to_reduce_cpu_cache_misses)
    {
      pitch1 = SCREENWIDTH * V_GetPixelDepth();
      pitch2 = SCREENWIDTH * V_GetPixelDepth() + 32;

      t1 = I_TestCPUCacheMisses(pitch1, SCREENHEIGHT);
      t2 = I_TestCPUCacheMisses(pitch2, SCREENHEIGHT);

      lprintf(LO_INFO, "I_CalculateRes: trying to optimise screen pitch\n");
      lprintf(LO_INFO, " test case for pitch=%d is %d msec\n", pitch1, t1);
      lprintf(LO_INFO, " test case for pitch=%d is %d msec\n", pitch2, t2);

      SCREENPITCH = (t1 <= t2 ? pitch1 : pitch2);

      lprintf(LO_INFO, " optimised screen pitch is %d\n", SCREENPITCH);
    }
    else
    {
      SCREENPITCH = SCREENWIDTH * V_GetPixelDepth();
    }
  }

  // e6y: processing of screen_multiply
  {
    int factor = ((V_GetMode() == VID_MODEGL) ? 1 : render_screen_multiply);
    REAL_SCREENWIDTH = SCREENWIDTH * factor;
    REAL_SCREENHEIGHT = SCREENHEIGHT * factor;
    REAL_SCREENPITCH = SCREENPITCH * factor;
  }
}

// CPhipps -
// I_InitScreenResolution
// Sets the screen resolution
// e6y: processing of screen_multiply
void I_InitScreenResolution(void)
{
  int i, p, w, h;
  char c;
  video_mode_t mode;

  // Video stuff
  if ((p = M_CheckParm("-width")))
    if (myargv[p+1])
      desired_screenwidth = atoi(myargv[p+1]);

  if ((p = M_CheckParm("-height")))
    if (myargv[p+1])
      desired_screenheight = atoi(myargv[p+1]);

  if ((p = M_CheckParm("-fullscreen")))
      use_fullscreen = 1;

  if ((p = M_CheckParm("-nofullscreen")))
      use_fullscreen = 0;

  // e6y
  // New command-line options for setting a window (-window) 
  // or fullscreen (-nowindow) mode temporarily which is not saved in cfg.
  // It works like "-geom" switch
  desired_fullscreen = use_fullscreen;
  if ((p = M_CheckParm("-window")))
      desired_fullscreen = 0;

  if ((p = M_CheckParm("-nowindow")))
      desired_fullscreen = 1;

  // e6y
  // change the screen size for the current session only
  // syntax: -geom WidthxHeight[w|f]
  // examples: -geom 320x200f, -geom 640x480w, -geom 1024x768
  w = desired_screenwidth;
  h = desired_screenheight;

  if (!(p = M_CheckParm("-geom")))
    p = M_CheckParm("-geometry");

  if (p && p + 1 < myargc)
  {
    int count = sscanf(myargv[p+1], "%dx%d%c", &w, &h, &c);

    // at least width and height must be specified
    // restoring original values if not
    if (count < 2)
    {
      w = desired_screenwidth;
      h = desired_screenheight;
    }

    if (count >= 3)
    {
      if (tolower(c) == 'w')
        desired_fullscreen = 0;
      if (tolower(c) == 'f')
        desired_fullscreen = 1;
    }
  }

  lprintf(LO_INFO, "I_InitScreenResolution: %dx%d (%s)\n", SCREENWIDTH, SCREENHEIGHT, desired_fullscreen ? "fullscreen" : "nofullscreen");

  mode = I_GetModeFromString(default_videomode);
  if ((i=M_CheckParm("-vidmode")) && i<myargc-1)
  {
    mode = I_GetModeFromString(myargv[i+1]);
  }
  
  V_InitMode(mode);

  I_CalculateRes(w, h);
  V_DestroyUnusedTrueColorPalettes();
  V_FreeScreens();

  // set first three to standard values
  for (i=0; i<3; i++) {
    screens[i].width = REAL_SCREENWIDTH;
    screens[i].height = REAL_SCREENHEIGHT;
    screens[i].byte_pitch = REAL_SCREENPITCH;
    screens[i].short_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
    screens[i].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
  }

  // statusbar
  screens[4].width = REAL_SCREENWIDTH;
  screens[4].height = (ST_SCALED_HEIGHT+1);
  screens[4].byte_pitch = REAL_SCREENPITCH;
  screens[4].short_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
  screens[4].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);

  I_InitBuffersRes();

  lprintf(LO_INFO,"I_InitScreenResolution: Using resolution %dx%d\n", REAL_SCREENWIDTH, REAL_SCREENHEIGHT);
}

// 
// Set the window caption
//

void I_SetWindowCaption(void)
{
  char *buf;

  buf = malloc(strlen(PACKAGE_TITLE) + strlen(VERSION) + 10);

  sprintf(buf, "%s %s", PACKAGE_TITLE, VERSION);

  SDL_WM_SetCaption(buf, NULL);

  free(buf);
}

// 
// Set the application icon
// 

#include "icon.c"

void I_SetWindowIcon(void)
{
    SDL_Surface *surface;
    Uint8 *mask;
    int i;

    // Generate the mask
  
    mask = malloc(icon_w * icon_h / 8);
    memset(mask, 0, icon_w * icon_h / 8);

    for (i=0; i<icon_w * icon_h; ++i) 
    {
        if (icon_data[i * 3] != 0x00
         || icon_data[i * 3 + 1] != 0x00
         || icon_data[i * 3 + 2] != 0x00)
        {
            mask[i / 8] |= 1 << (7 - i % 8);
        }
    }

    surface = SDL_CreateRGBSurfaceFrom(icon_data,
                                       icon_w,
                                       icon_h,
                                       24,
                                       icon_w * 3,
                                       0xff << 0,
                                       0xff << 8,
                                       0xff << 16,
                                       0);

    SDL_WM_SetIcon(surface, mask);
    SDL_FreeSurface(surface);
    free(mask);
}

void I_InitGraphics(void)
{
  static int    firsttime=1;

  if (firsttime)
  {
    firsttime = 0;

    // e6y: initialisation of screen_multiply
    screen_multiply = render_screen_multiply;

    atexit(I_ShutdownGraphics);
    lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

    /* Set the video mode */
    I_UpdateVideoMode();

    //e6y: setup the window title
    I_SetWindowCaption();

    //e6y: set the application icon
    I_SetWindowIcon();

    /* Initialize the input system */
    I_InitInputs();

    //e6y: new mouse code
    UpdateFocus();
    UpdateGrab();
  }
}

int I_GetModeFromString(const char *modestr)
{
  video_mode_t mode;

  if (!stricmp(modestr,"15")) {
    mode = VID_MODE15;
  } else if (!stricmp(modestr,"15bit")) {
    mode = VID_MODE15;
  } else if (!stricmp(modestr,"16")) {
    mode = VID_MODE16;
  } else if (!stricmp(modestr,"16bit")) {
    mode = VID_MODE16;
  } else if (!stricmp(modestr,"32")) {
    mode = VID_MODE32;
  } else if (!stricmp(modestr,"32bit")) {
    mode = VID_MODE32;
  } else if (!stricmp(modestr,"gl")) {
    mode = VID_MODEGL;
  } else if (!stricmp(modestr,"OpenGL")) {
    mode = VID_MODEGL;
  } else {
    mode = VID_MODE8;
  }
  return mode;
}

void I_UpdateVideoMode(void)
{
  int init_flags;

  // Initialize SDL with this graphics mode
  if (V_GetMode() == VID_MODEGL) {
    init_flags = SDL_OPENGL;
  } else {
    if (use_doublebuffer)
      init_flags = SDL_DOUBLEBUF;
    else
      init_flags = SDL_SWSURFACE;
#ifndef _DEBUG
    init_flags |= SDL_HWPALETTE;
#endif
  }

  if ( desired_fullscreen )
    init_flags |= SDL_FULLSCREEN;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, gl_colorbuffer_bits );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, gl_depthbuffer_bits );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    //e6y: vertical sync
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (gl_vsync ? 1 : 0));

    //e6y: anti-aliasing
    e6y_MultisamplingSet();

    screen = SDL_SetVideoMode(REAL_SCREENWIDTH, REAL_SCREENHEIGHT, gl_colorbuffer_bits, init_flags);
    gld_CheckHardwareGamma();
  }
  else
#endif
  {
    // e6y: processing of screen_multiply
    screen = SDL_SetVideoMode(REAL_SCREENWIDTH, REAL_SCREENHEIGHT, V_GetNumPixelBits(), init_flags);
  }

  if(screen == NULL) {
    I_Error("Couldn't set %dx%d video mode [%s]", REAL_SCREENWIDTH, REAL_SCREENHEIGHT, SDL_GetError());
  }
  if (V_GetMode() == VID_MODEGL)
    e6y_MultisamplingCheck();//e6y

  lprintf(LO_INFO, "I_UpdateVideoMode: 0x%x, %s, %s\n", init_flags, screen->pixels ? "SDL buffer" : "own buffer", SDL_MUSTLOCK(screen) ? "lock-and-copy": "direct access");

  // Get the info needed to render to the display
  if (screen_multiply==1 && !SDL_MUSTLOCK(screen))
  {
    screens[0].not_on_heap = true;
    screens[0].data = (unsigned char *) (screen->pixels);
    screens[0].byte_pitch = screen->pitch;
    screens[0].short_pitch = screen->pitch / V_GetModePixelDepth(VID_MODE16);
    screens[0].int_pitch = screen->pitch / V_GetModePixelDepth(VID_MODE32);
  }
  else
  {
    screens[0].not_on_heap = false;
  }

  V_AllocScreens();

  R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);

  if (V_GetMode() == VID_MODEGL) {
    int temp;
    lprintf(LO_INFO,"SDL OpenGL PixelFormat:\n");
    SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_STENCIL_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_ALPHA_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &temp );
    lprintf(LO_INFO,"    SDL_GL_DOUBLEBUFFER: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BUFFER_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BUFFER_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_DEPTH_SIZE: %i\n",temp);
#ifdef GL_DOOM
    SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &temp );
    lprintf(LO_INFO,"    SDL_GL_MULTISAMPLESAMPLES: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &temp );
    lprintf(LO_INFO,"    SDL_GL_MULTISAMPLEBUFFERS: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_STENCIL_SIZE: %i\n",temp);

    gld_Init(SCREENWIDTH, SCREENHEIGHT);
#endif
  }
  I_AfterUpdateVideoMode();//e6y
}

static void ActivateMouse(void)
{
  SDL_SetCursor(cursors[1]);
  SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_ShowCursor(1);
}

static void DeactivateMouse(void)
{
  SDL_SetCursor(cursors[0]);
  SDL_WM_GrabInput(SDL_GRAB_OFF);
  SDL_ShowCursor(1);
}

// Warp the mouse back to the middle of the screen
static void CenterMouse(void)
{
  // Warp the the screen center
  SDL_WarpMouse((unsigned short)(REAL_SCREENWIDTH/2), (unsigned short)(REAL_SCREENHEIGHT/2));

  // Clear any relative movement caused by warping
  SDL_PumpEvents();
  SDL_GetRelativeMouseState(NULL, NULL);
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.
static void I_ReadMouse(void)
{
  int x, y;
  event_t ev;

  if (!usemouse)
    return;

  if (!MouseShouldBeGrabbed())
    return;

  SDL_GetRelativeMouseState(&x, &y);

  if (x != 0 || y != 0) 
  {
    ev.type = ev_mouse;
    ev.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    ev.data2 = x << 5;
    ev.data3 = (-y) << 5;

    D_PostEvent(&ev);
  }

  CenterMouse();
}

static boolean MouseShouldBeGrabbed()
{
  // never grab the mouse when in screensaver mode

  //if (screensaver_mode)
  //    return false;

  // if the window doesnt have focus, never grab it
  if (!window_focused)
    return false;

  // always grab the mouse when full screen (dont want to 
  // see the mouse pointer)
  if (desired_fullscreen)
    return true;

  // if we specify not to grab the mouse, never grab
  if (!mouse_enabled)
    return false;

  // always grab the mouse in camera mode when playing levels 
  // and menu is not active
  if (walkcamera.type)
    return (demoplayback && gamestate == GS_LEVEL && !menuactive);

  // when menu is active or game is paused, release the mouse 
  if (menuactive || paused)
    return false;

  // only grab mouse when playing levels (but not demos)
  return (gamestate == GS_LEVEL) && !demoplayback;
}

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.
static void UpdateFocus(void)
{
  Uint8 state;

  state = SDL_GetAppState();

  // We should have input (keyboard) focus and be visible 
  // (not minimised)
  window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

  // e6y
  // Reuse of a current palette to avoid black screen at software fullscreen modes
  // after switching to OS and back
  if (desired_fullscreen && window_focused)
  {
    V_SetPalette(st_palette);
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    if (gl_lightmode == gl_lightmode_gzdoom)
    {
      if (!window_focused)
      {
        // e6y: Restore of startup gamma if window loses focus
        gld_SetGammaRamp(-1);
      }
      else
      {
        gld_SetGammaRamp(useglgamma);
      }
    }
  }
#endif

  // Should the screen be grabbed?
  //    screenvisible = (state & SDL_APPACTIVE) != 0;
}

void UpdateGrab(void)
{
  static boolean currently_grabbed = false;
  boolean grab;

  grab = MouseShouldBeGrabbed();

  if (grab && !currently_grabbed)
  {
    ActivateMouse();
  }

  if (!grab && currently_grabbed)
  {
    DeactivateMouse();
  }

  currently_grabbed = grab;
}