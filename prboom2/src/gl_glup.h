/* $Id: gl_glup.h,v 1.2 2002/11/16 23:02:19 proff_fs Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * Copyright (C) 1995-2000  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * This file allows the GLU code to be compiled either with the Mesa
 * headers or with the real OpenGL headers.
 */


#ifndef GLUP_H
#define GLUP_H

#include <string.h>
#include "gl_dyn.h"

#  define GLAPIENTRY
#  define GLCALLBACK
#  define GLAPI

#ifndef GLU_TESS_ERROR9
   /* If we're using the real OpenGL header files... */
#  define GLU_TESS_ERROR9	100159
#endif

#define GLU_NO_ERROR		GL_NO_ERROR

#ifndef NULL
#  define NULL 0
#endif


#endif
