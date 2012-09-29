/*
 * niftyconf - niftyled GUI
 * Copyright (C) 2011-2012 Daniel Hiepler <daniel@niftylight.de>
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

#include <gtk/gtk.h>
#include "niftyconf-ui.h"



/** GtkBuilder for this module */
static GtkBuilder *_ui;






/******************************************************************************
 ****************************** STATIC FUNCTIONS ******************************
 ******************************************************************************/



/******************************************************************************
 ******************************************************************************/



/** show/hide window */
void about_set_visible(gboolean visible)
{
        gtk_widget_set_visible(GTK_WIDGET(UI("window")), visible);
        gtk_widget_show(GTK_WIDGET(UI("window")));
}


/** initialize setup tree module */
gboolean  about_init()
{
        if(!(_ui = ui_builder("niftyconf-about.ui")))
                return FALSE;

        return TRUE;
}


/** deinitialize this module */
void about_deinit()
{
	g_object_unref(_ui);
}


/******************************************************************************
 ***************************** CALLBACKS **************************************
 ******************************************************************************/

/** hide dialog */
void  on_about_dialog_closed(GtkDialog *arg0, gpointer   user_data)
{

		gtk_widget_set_visible(GTK_WIDGET(arg0), FALSE);
}


/** hide dialog */
G_MODULE_EXPORT gboolean on_about_dialog_response(GtkDialog *dialog,
                                                        gint       response_id,
                                                        gpointer   user_data)
{
        	gtk_widget_set_visible(GTK_WIDGET(dialog), FALSE);
        	return TRUE;
}
