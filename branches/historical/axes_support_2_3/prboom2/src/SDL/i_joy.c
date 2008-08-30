/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: i_joy.c,v 1.4 2001/07/16 15:41:26 proff_fs Exp $
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
 *   Joystick handling for Linux
 *
 *-----------------------------------------------------------------------------
 */

#ifndef lint
static const char rcsid[] = "$Id: i_joy.c,v 1.4 2001/07/16 15:41:26 proff_fs Exp $";
#endif /* lint */

#include "SDL.h"
#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"
#include "lprintf.h"
#include "m_fixed.h"

int joyleft;
int joyright;
int joyup;
int joydown;

int usejoystick;

static SDL_Joystick *joystick;

void I_EndJoystick(void)
{
  lprintf(LO_DEBUG, "I_EndJoystick : closing joystick\n");
}

void I_PollJoystick(void)
{
  event_t ev;
  Sint16 axis_value;

  if (!usejoystick || (!joystick)) return;
  ev.type = ev_joystick;
  ev.data1 =
    (SDL_JoystickGetButton(joystick, 0)<<0) |
    (SDL_JoystickGetButton(joystick, 1)<<1) |
    (SDL_JoystickGetButton(joystick, 2)<<2) |
    (SDL_JoystickGetButton(joystick, 3)<<3);
  axis_value = SDL_JoystickGetAxis(joystick, 0) / 3000;
  if (D_abs(axis_value)<10) axis_value=0;
  ev.data2 = axis_value;
  axis_value = SDL_JoystickGetAxis(joystick, 1) / 3000;
  if (D_abs(axis_value)<10) axis_value=0;
  ev.data3 = axis_value;

  D_PostEvent(&ev);
}

void I_InitJoystick(void)
{
  const char* fname = "I_InitJoystick : ";
  int num_joysticks;

  if (!usejoystick) return;
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  num_joysticks=SDL_NumJoysticks();
  if (M_CheckParm("-nojoy") || (usejoystick>num_joysticks) || (usejoystick<0)) {
    if ((usejoystick > num_joysticks) || (usejoystick < 0))
      lprintf(LO_WARN, "%sinvalid joystick %d\n", fname, usejoystick);
    else
      lprintf(LO_INFO, "%suser disabled\n", fname);
    return;
  }
  joystick=SDL_JoystickOpen(usejoystick-1);
  if (!joystick)
    lprintf(LO_ERROR, "%serror opening joystick %s\n", fname, SDL_JoystickName(usejoystick-1));
  else {
    atexit(I_EndJoystick);
    lprintf(LO_INFO, "%sopened %s\n", fname, SDL_JoystickName(usejoystick-1));
    joyup = 32767;
    joydown = -32768;
    joyright = 32767;
    joyleft = -32768;
  }
}