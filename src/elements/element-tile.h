/*
 * niftyconf - niftyled GUI
 * Copyright (C) 2011-2014 Daniel Hiepler <daniel@niftylight.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _NIFTYCONF_TILE_H
#define _NIFTYCONF_TILE_H

#include <niftyled.h>
#include "renderer/renderer.h"
#include "elements/element-hardware.h"


typedef struct _NiftyconfTile   NiftyconfTile;



/* GUI model functions */
gboolean                        tile_init();
void                            tile_deinit();
NiftyconfTile                  *tile_register_to_gui(LedTile * t);
void                            tile_unregister_from_gui(NiftyconfTile * t);
gboolean                        tile_of_hardware_new(NiftyconfHardware * parent);
gboolean                        tile_of_tile_new(NiftyconfTile * parent);
void                            tile_destroy(NiftyconfTile * tile);


/* GUI functions */
gboolean                        tile_get_collapsed(NiftyconfTile * t);
gboolean                        tile_get_highlighted(NiftyconfTile * t);
void                            tile_set_collapsed(NiftyconfTile * t, gboolean is_collapsed);
void                            tile_set_highlighted(NiftyconfTile * t, gboolean is_highlighted);


/* model functions */
gboolean                        tile_calc_render_offset(NiftyconfTile * t, double screenWidth, double screenHeight, double *xOff, double *yOff);
NiftyconfRenderer              *tile_get_renderer(NiftyconfTile * t);
LedTile                        *tile_niftyled(NiftyconfTile * t);
char                           *tile_dump(NiftyconfTile * tile, gboolean encapsulation);


#endif /* _NIFTYCONF_TILE_H */
