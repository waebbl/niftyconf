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

#include <math.h>
#include <gtk/gtk.h>
#include "ui/ui.h"
#include "ui/ui-log.h"
#include "ui/ui-renderer.h"
#include "ui/ui-setup-tree.h"
#include "ui/ui-setup-props.h"
#include "ui/ui-setup-ledlist.h"
#include "elements/element-setup.h"
#include "renderer/renderer-setup.h"
#include "renderer/renderer-tile.h"
#include "renderer/renderer-chain.h"
#include "renderer/renderer-led.h"
#include "live-preview/live-preview.h"




/** GtkBuilder for this module */
static GtkBuilder *_builder;


/* currently shown elements */
static NiftyconfHardware *current_hw;
static NiftyconfTile *current_tile;
static NiftyconfChain *current_chain;
static NiftyconfLed *current_led;


/******************************************************************************
 ****************************** STATIC FUNCTIONS ******************************
 ******************************************************************************/


static void _widget_set_error_background(GtkWidget * w, gboolean error)
{
        if(error)
        {
                GdkColor color;
                gdk_color_parse("#f96b5f", &color);
                gtk_widget_modify_base(w, GTK_STATE_NORMAL, &color);
        }
        else
        {
                gtk_widget_modify_base(w, GTK_STATE_NORMAL, NULL);
        }
}




/******************************************************************************
 ***************************** CALLBACKS **************************************
 ******************************************************************************/

/** foreach helper to set led's x position */
void _set_x(NiftyconfLed * led, void *u)
{
        Led *l = led_niftyled(led);
        LedFrameCord *new_val = u;
        LedFrameCord x, y;
        led_get_pos(l, &x, &y);

        if(x == *new_val)
                return;

        /* set new value */
        led_set_pos(l, *new_val, y);

        renderer_led_damage(led);
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_led_x_changed(GtkSpinButton * s,
                                                 gpointer u)
{

        /* set x on all selected LEDs */
        LedFrameCord new_val =
                (LedFrameCord) gtk_spin_button_get_value_as_int(s);
        ui_setup_ledlist_do_foreach_selected_element(_set_x, &new_val);

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        ui_renderer_all_queue_draw();
}


/** foreach helper to set led's y position */
void _set_y(NiftyconfLed * led, void *u)
{
        Led *l = led_niftyled(led);
        LedFrameCord *new_val = u;
        LedFrameCord x, y;
        led_get_pos(led_niftyled(led), &x, &y);

        if(y == *new_val)
                return;

        /* set new value */
        led_set_pos(l, x, *new_val);

        renderer_led_damage(led);
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_led_y_changed(GtkSpinButton * s,
                                                 gpointer u)
{
        /* set y on all selected LEDs */
        LedFrameCord new_val =
                (LedFrameCord) gtk_spin_button_get_value_as_int(s);
        ui_setup_ledlist_do_foreach_selected_element(_set_y, &new_val);

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        ui_renderer_all_queue_draw();
}


/** foreach helper to set led's component */
void _set_component(NiftyconfLed * led, void *u)
{
        Led *l = led_niftyled(led);
        LedFrameComponent *new_val = u;

        if(led_get_component(l) == *new_val)
                return;

        led_set_component(l, *new_val);
        renderer_led_damage(led);
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_led_component_changed(GtkSpinButton * s,
                                                         gpointer u)
{

        /* set component on all selected LEDs */
        LedFrameComponent new_val =
                (LedFrameComponent) gtk_spin_button_get_value_as_int(s);
        ui_setup_ledlist_do_foreach_selected_element(_set_component,
                                                     &new_val);

        /* redraw */
        ui_renderer_all_queue_draw();
}


/** foreach helper to set gain */
void _set_gain(NiftyconfLed * led, void *u)
{
        /* get currently selected LED */
        Led *l = led_niftyled(led);

        /* value really changed? */
        LedGain *new_val = u;
        if(led_get_gain(l) == *new_val)
                return;

        led_set_gain(l, *new_val);

        /* reflect new gain on hardware */
        live_preview_highlight_led(led);
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_led_gain_changed(GtkSpinButton * s,
                                                    gpointer u)
{
        /* walk all currently selected LEDs */
        LedGain gain = gtk_spin_button_get_value(s);
        ui_setup_ledlist_do_foreach_selected_element(_set_gain, &gain);

        /* get hardware these LEDs belong to */
        LedHardware *h;
        LedChain *c = chain_niftyled(led_get_chain(current_led));
        if(led_chain_parent_is_hardware(c))
        {
                h = led_chain_get_parent_hardware(c);
        }
        else
        {
                LedTile *t;
                for(t = led_chain_get_parent_tile(c);
                    t && !led_tile_get_parent_hardware(t);
                    t = led_tile_get_parent_tile(t));

                h = led_tile_get_parent_hardware(t);
        }


        /* update gain */
        led_hardware_refresh_gain(h);

        live_preview_show();
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_chain_ledcount_changed(GtkSpinButton * s,
                                                          gpointer u)
{
        /* get current chain */
        LedChain *chain = chain_niftyled(current_chain);

        /* get new ledcount */
        LedCount ledcount = (LedCount) gtk_spin_button_get_value_as_int(s);

        /* ledcount really changed? */
        if(led_chain_get_ledcount(chain) == ledcount)
                return;

        /* remove all current LEDs */
        ui_setup_ledlist_clear();

        /* unregister all LEDs from chain */
        chain_unregister_leds_from_gui(current_chain);

        /* is this the chain of a hardware element? */
        LedHardware *h;
        if((h = led_chain_get_parent_hardware(chain)))
        {
                /* set new ledcount */
                if(!led_hardware_set_ledcount(h, ledcount))
                        /* error background color */
                {
                        _widget_set_error_background(GTK_WIDGET(s), true);
                        return;
                }
                /* normal background color */
                else
                {
                        _widget_set_error_background(GTK_WIDGET(s), false);
                }
        }
        /* this is an ordinary chain of a tile element */
        {
                /* set new ledcount */
                if(!led_chain_set_ledcount(chain, ledcount))
                        /* error background color */
                {
                        _widget_set_error_background(GTK_WIDGET(s), true);
                        return;
                }
                /* normal background color */
                else
                {
                        _widget_set_error_background(GTK_WIDGET(s), false);
                }
        }

        /* re-register (less or more) LEDs in chain */
        chain_register_leds_to_gui(current_chain);

        /* refresh tree */
        ui_setup_tree_refresh();
        ui_setup_ledlist_refresh(current_chain);

        /* redraw */
        renderer_chain_damage(current_chain);
        ui_renderer_all_queue_draw();
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_tile_x_changed(GtkSpinButton * s,
                                                  gpointer u)
{
        /* get current tile */
        LedTile *tile = tile_niftyled(current_tile);

        /* get position */
        LedFrameCord x, y;
        led_tile_get_pos(tile, &x, &y);

        /* value really changed? */
        int new_val = gtk_spin_button_get_value_as_int(s);
        if(x == new_val)
                return;

        /* set new value */
        if(!led_tile_set_pos(tile, new_val, y))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        renderer_tile_damage(current_tile);
        ui_renderer_all_queue_draw();

}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_tile_y_changed(GtkSpinButton * s,
                                                  gpointer u)
{
        /* get current tile */
        LedTile *tile = tile_niftyled(current_tile);

        /* get position */
        LedFrameCord x, y;
        led_tile_get_pos(tile, &x, &y);

        /* value really changed? */
        int new_val = gtk_spin_button_get_value_as_int(s);
        if(y == new_val)
                return;

        /* set new value */
        if(!led_tile_set_pos(tile, x, new_val))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        renderer_tile_damage(current_tile);
        ui_renderer_all_queue_draw();

}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_tile_rotation_changed(GtkSpinButton * s,
                                                         gpointer u)
{
        /* get current tile */
        LedTile *tile = tile_niftyled(current_tile);

        /* value really changed? */
        double new_val = (gtk_spin_button_get_value(s) * M_PI) / 180;
        if(led_tile_get_rotation(tile) == new_val)
                return;

        /* set new value */
        if(!led_tile_set_rotation(tile, new_val))
        {
                /* error background color */
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        else
        {
                /* normal background color */
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

        /* refresh view */
        ui_setup_props_tile_show(current_tile);

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        renderer_tile_damage(current_tile);
        ui_renderer_all_queue_draw();
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_tile_pivot_x_changed(GtkSpinButton * s,
                                                        gpointer u)
{
        /* get current tile */
        LedTile *tile = tile_niftyled(current_tile);

        /* get pivot */
        double x, y;
        led_tile_get_pivot(tile, &x, &y);

        /* value really changed? */
        double new_val = gtk_spin_button_get_value(s);
        if(x == new_val)
                return;

        /* set new value */
        if(!led_tile_set_pivot(tile, new_val, y))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

        /* refresh view */
        ui_setup_props_tile_show(current_tile);

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        renderer_tile_damage(current_tile);
        ui_renderer_all_queue_draw();
}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_tile_pivot_y_changed(GtkSpinButton * s,
                                                        gpointer u)
{
        /* get current tile */
        LedTile *tile = tile_niftyled(current_tile);

        /* get pivot */
        double x, y;
        led_tile_get_pivot(tile, &x, &y);

        /* value really changed? */
        double new_val = gtk_spin_button_get_value(s);
        if(y == new_val)
                return;

        /* set new value */
        if(!led_tile_set_pivot(tile, x, new_val))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

        /* refresh view */
        ui_setup_props_tile_show(current_tile);

        /* refresh tree */
        ui_setup_tree_refresh();

        /* redraw */
        renderer_tile_damage(current_tile);
        ui_renderer_all_queue_draw();
}


/** entry text changed */
G_MODULE_EXPORT void on_entry_hardware_name_changed(GtkEditable * e,
                                                    gpointer u)
{
        /* get currently selected hardware */
        LedHardware *h = hardware_niftyled(current_hw);

        /* set value */
        if(!led_hardware_set_name(h, gtk_entry_get_text(GTK_ENTRY(e))))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(e), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(e), false);
        }

        /* refresh view */
        ui_setup_tree_refresh();

}


/** entry text changed */
G_MODULE_EXPORT void on_entry_hardware_id_changed(GtkEditable * e, gpointer u)
{
        /* get currently selected hardware */
        LedHardware *h = hardware_niftyled(current_hw);

        /* set value */
        if(!led_hardware_set_id(h, gtk_entry_get_text(GTK_ENTRY(e))))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(e), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(e), false);
        }

}


/** spinbutton value changed */
G_MODULE_EXPORT void on_spinbutton_hardware_stride_changed(GtkSpinButton * s,
                                                           gpointer u)
{
        /* get currently selected hardware */
        LedHardware *h = hardware_niftyled(current_hw);

        /* set value */
        if(!led_hardware_set_stride
           (h, (LedCount) gtk_spin_button_get_value_as_int(s)))
                /* error background color */
        {
                _widget_set_error_background(GTK_WIDGET(s), true);
        }
        /* normal background color */
        else
        {
                _widget_set_error_background(GTK_WIDGET(s), false);
        }

}


/** togglebutton toggled */
G_MODULE_EXPORT void on_togglebutton_hardware_init_toggled(GtkToggleButton *
                                                           b, gpointer u)
{
        LedHardware *h = hardware_niftyled(current_hw);

        /* initialize */
        if(gtk_toggle_button_get_active(b))
        {
                /* initialize hardware */
                LedChain *c = led_hardware_get_chain(h);
                const gchar *id =
                        gtk_entry_get_text(GTK_ENTRY(UI("entry_hw_id")));
                LedCount ledcount = led_chain_get_ledcount(c);
                const char *pixelformat =
                        led_pixel_format_to_string(led_chain_get_format(c));

                if(!(led_hardware_init(h, id, ledcount, pixelformat)))
                {
                        ui_log_alert_show("Failed to initialize hardware.");
                        return;
                }

                /* show correct image */
                ui_setup_props_hardware_initialized_image(true);

                /* update ID (it might have changed) */
                gtk_entry_set_text(GTK_ENTRY(UI("entry_hw_id")),
                                   led_hardware_get_id(h));
        }
        /* deinitialize */
        else
        {
                /* deinitialize hardware */
                led_hardware_deinit(h);

                /* show correct image */
                ui_setup_props_hardware_initialized_image(false);
        }
}


/** different custom hw property selected */
G_MODULE_EXPORT void on_combobox_hw_props_changed(GtkComboBoxText * b,
                                                  gpointer u)
{
        const char *propname;

        if(!(propname = gtk_combo_box_text_get_active_text(b)))
                return;

        LedHardware *h = hardware_niftyled(current_hw);
        LedPluginCustomProp *prop;
        if(!(prop = led_hardware_plugin_prop_find(h, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to get plugin property \"%s\"",
                        gtk_combo_box_text_get_active_text(b));
        }


        gtk_widget_set_visible(GTK_WIDGET(UI("label_hw_prop_none")), false);
        gtk_widget_set_visible(GTK_WIDGET(UI("spinbutton_hw_prop_int")),
                               false);
        gtk_widget_set_visible(GTK_WIDGET(UI("spinbutton_hw_prop_float")),
                               false);
        gtk_widget_set_visible(GTK_WIDGET(UI("entry_hw_prop_string")), false);
        gtk_widget_set_visible(GTK_WIDGET(UI("checkbutton_hw_prop_bool")),
                               false);

        switch (led_hardware_plugin_prop_get_type(prop))
        {
                case LED_HW_CUSTOM_PROP_FLOAT:
                {
                        float floatval;
                        led_hardware_plugin_prop_get_float(h, propname,
                                                           &floatval);
                        gtk_widget_set_visible(GTK_WIDGET
                                               (UI
                                                ("spinbotton_hw_prop_float")),
                                               true);
                        gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                                  (UI
                                                   ("spinbutton_hw_prop_float")),
                                                  (gdouble) floatval);
                        gtk_spin_button_set_digits(GTK_SPIN_BUTTON
                                                   (UI
                                                    ("spinbutton_hw_prop_float")),
                                                   5);
                        break;
                }

                case LED_HW_CUSTOM_PROP_INT:
                {
                        int intval;
                        led_hardware_plugin_prop_get_int(h, propname,
                                                         &intval);
                        gtk_widget_set_visible(GTK_WIDGET
                                               (UI("spinbutton_hw_prop_int")),
                                               true);
                        gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                                  (UI
                                                   ("spinbutton_hw_prop_int")),
                                                  (gdouble) intval);
                        gtk_spin_button_set_digits(GTK_SPIN_BUTTON
                                                   (UI
                                                    ("spinbutton_hw_prop_int")),
                                                   0);
                        break;
                }

                case LED_HW_CUSTOM_PROP_STRING:
                {
                        char *stringval;
                        led_hardware_plugin_prop_get_string(h, propname,
                                                            &stringval);
                        gtk_widget_set_visible(GTK_WIDGET
                                               (UI("entry_hw_prop_string")),
                                               true);
                        gtk_entry_set_text(GTK_ENTRY
                                           (UI("entry_hw_prop_string")),
                                           stringval);
                        break;
                }

                default:
                {
                        gtk_widget_set_visible(GTK_WIDGET
                                               (UI("label_hw_prop_none")),
                                               true);
                        break;
                }
        }
}


/** float hardware property spin-button value changed */
G_MODULE_EXPORT void on_spinbutton_hw_prop_float_change_value(GtkSpinButton *
                                                              b, gpointer u)
{
        const char *propname =
                gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT
                                                   (UI("combobox_hw_props")));
        LedHardware *h = hardware_niftyled(current_hw);
        LedPluginCustomProp *prop =
                led_hardware_plugin_prop_find(h, propname);

        if(led_hardware_plugin_prop_get_type(prop) !=
           LED_HW_CUSTOM_PROP_FLOAT)
        {
                NFT_LOG(L_ERROR,
                        "expected float hardware property. This is a bug.");
                return;
        }

        float floatval = gtk_spin_button_get_value_as_float(b);
        led_hardware_plugin_prop_set_float(h, propname, floatval);

}


/** integer hardware property spin-button value changed */
G_MODULE_EXPORT void on_spinbutton_hw_prop_int_change_value(GtkSpinButton * b,
                                                            gpointer u)
{
        const char *propname =
                gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT
                                                   (UI("combobox_hw_props")));
        LedHardware *h = hardware_niftyled(current_hw);
        LedPluginCustomProp *prop =
                led_hardware_plugin_prop_find(h, propname);

        if(led_hardware_plugin_prop_get_type(prop) != LED_HW_CUSTOM_PROP_INT)
        {
                NFT_LOG(L_ERROR,
                        "expected integer hardware property. This is a bug.");
                return;
        }

        int intval = gtk_spin_button_get_value_as_int(b);
        led_hardware_plugin_prop_set_int(h, propname, intval);

}


/** hardware plugin custom text property changed */
G_MODULE_EXPORT void on_entry_hw_prop_string_changed(GtkEditable * e,
                                                     gpointer u)
{
        const char *propname =
                gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT
                                                   (UI("combobox_hw_props")));
        LedHardware *h = hardware_niftyled(current_hw);

        led_hardware_plugin_prop_set_string(h, propname,
                                            gtk_entry_get_text(GTK_ENTRY(e)));
}



/******************************************************************************
 ******************************************************************************/

/**
 * getter for widget
 */
GtkWidget *ui_setup_props_get_widget()
{
        return GTK_WIDGET(UI("box_props"));
}


/** show hardware props */
void ui_setup_props_hardware_show(NiftyconfHardware * h)
{
        current_hw = h;

        gtk_widget_show(GTK_WIDGET(UI("frame_hardware")));

        if(!h)
                return;


        LedHardware *hardware = hardware_niftyled(h);

        /* name */
        gtk_entry_set_text(GTK_ENTRY(UI("entry_hw_name")),
                           led_hardware_get_name(hardware));
        /* plugin family */
        gtk_entry_set_text(GTK_ENTRY(UI("entry_hw_plugin")),
                           led_hardware_plugin_get_family(hardware));
        /* id */
        gtk_entry_set_text(GTK_ENTRY(UI("entry_hw_id")),
                           led_hardware_get_id(hardware));
        gtk_widget_set_tooltip_text(GTK_WIDGET(UI("entry_hw_id")),
                                    led_hardware_plugin_get_id_example
                                    (hardware));
        /* stride */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                  (UI("spinbutton_hw_stride")),
                                  (gdouble)
                                  led_hardware_get_stride(hardware));

        /* set button to correct toggled state */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
                                     (UI("togglebutton_initialized")),
                                     led_hardware_is_initialized(hardware));

        /* custom properties */
        gtk_widget_set_visible(GTK_WIDGET(UI("label_hw_prop_none")), false);
        gtk_widget_set_visible(GTK_WIDGET(UI("spinbutton_hw_prop_int")),
                               false);
        gtk_widget_set_visible(GTK_WIDGET(UI("spinbutton_hw_prop_float")),
                               false);
        gtk_widget_set_visible(GTK_WIDGET(UI("entry_hw_prop_string")), false);
        gtk_widget_set_visible(GTK_WIDGET(UI("checkbutton_hw_prop_bool")),
                               false);

        if(led_hardware_is_initialized(hardware))
        {
                /* clear property name combobox */
                gtk_list_store_clear(GTK_LIST_STORE
                                     (gtk_combo_box_get_model
                                      (GTK_COMBO_BOX
                                       (UI("combobox_hw_props")))));

                /* set property names to combobox */
                LedPluginCustomProp *prop;
                for(prop =
                    led_hardware_plugin_prop_get_nth(hardware, 0);
                    prop; prop = led_hardware_plugin_prop_get_next(prop))
                {
                        gtk_combo_box_text_append_text
                                (GTK_COMBO_BOX_TEXT
                                 (UI("combobox_hw_props")),
                                 led_hardware_plugin_prop_get_name(prop));
                }

                gtk_combo_box_set_active(GTK_COMBO_BOX
                                         (UI("combobox_hw_props")), 0);

        }

}


/** show tile props */
void ui_setup_props_tile_show(NiftyconfTile * t)
{
        current_tile = t;

        if(t)
        {
                /* position */
                LedTile *tile = tile_niftyled(t);
                LedFrameCord x, y;
                led_tile_get_pos(tile, &x, &y);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_x")),
                                          (gdouble) x);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_y")),
                                          (gdouble) y);

                /* dimension */
                LedFrameCord w, h;
                led_tile_get_transformed_dim(tile, &w, &h);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_width")),
                                          (gdouble) w);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_height")),
                                          (gdouble) h);

                /* transformation */
                double pX, pY;
                led_tile_get_pivot(tile, &pX, &pY);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_rotation")),
                                          (gdouble)
                                          led_tile_get_rotation(tile) * 180 /
                                          M_PI);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_pivot_x")),
                                          (gdouble) pX);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_tile_pivot_y")),
                                          (gdouble) pY);
        }

        gtk_widget_show(GTK_WIDGET(UI("frame_tile")));
}


/** show chain props */
void ui_setup_props_chain_show(NiftyconfChain * c)
{
        current_chain = c;

        if(c)
        {
                LedChain *chain = chain_niftyled(c);
                gtk_entry_set_text(GTK_ENTRY(UI("entry_chain_format")),
                                   led_pixel_format_to_string
                                   (led_chain_get_format(chain)));
                /* set minimum for "ledcount" spinbutton according to format */
                size_t minimum =
                        led_pixel_format_get_n_components(led_chain_get_format
                                                          (chain));
                gtk_adjustment_set_lower(GTK_ADJUSTMENT
                                         (UI("adjustment_chain_ledcount")),
                                         (gdouble) minimum);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON
                                          (UI("spinbutton_chain_ledcount")),
                                          (gdouble)
                                          led_chain_get_ledcount(chain));
        }

        gtk_widget_show(GTK_WIDGET(UI("frame_chain")));
}


/** show led props */
void ui_setup_props_led_show(NiftyconfLed * l)
{
        current_led = l;

#define SPIN_SET(a,b,c) \
	g_signal_handlers_block_by_func(GTK_SPIN_BUTTON(UI(a)), c, NULL); \
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(UI(a)), b); \
	g_signal_handlers_unblock_by_func(GTK_SPIN_BUTTON(UI(a)), c, NULL);

        if(l)
        {


                Led *led = led_niftyled(l);
                LedFrameCord x, y;
                led_get_pos(led, &x, &y);

                SPIN_SET("spinbutton_led_x", (gdouble) x,
                         on_spinbutton_led_x_changed);
                SPIN_SET("spinbutton_led_y", (gdouble) y,
                         on_spinbutton_led_y_changed);
                SPIN_SET("spinbutton_led_component",
                         (gdouble) led_get_component(led),
                         on_spinbutton_led_component_changed);
                SPIN_SET("spinbutton_led_gain", (gdouble) led_get_gain(led),
                         on_spinbutton_led_gain_changed);
        }

        gtk_widget_show(GTK_WIDGET(UI("frame_led")));
}


/** hide all props */
void ui_setup_props_hide()
{
        gtk_widget_hide(GTK_WIDGET(UI("frame_hardware")));
        gtk_widget_hide(GTK_WIDGET(UI("frame_tile")));
        gtk_widget_hide(GTK_WIDGET(UI("frame_chain")));
        gtk_widget_hide(GTK_WIDGET(UI("frame_led")));
}


/** initialize this module */
gboolean ui_setup_props_init()
{
        _builder = ui_builder("niftyconf-setup-props.ui");

        /* set platform specific stuff */
        gtk_adjustment_set_lower(GTK_ADJUSTMENT(UI("adjustment_hw_prop_int")),
                                 (gdouble) (INT_MIN));
        gtk_adjustment_set_upper(GTK_ADJUSTMENT(UI("adjustment_hw_prop_int")),
                                 (gdouble) (INT_MAX));
        gtk_adjustment_set_lower(GTK_ADJUSTMENT
                                 (UI("adjustment_hw_prop_float")),
                                 (gdouble) (FLT_MIN));
        gtk_adjustment_set_upper(GTK_ADJUSTMENT
                                 (UI("adjustment_hw_prop_float")),
                                 (gdouble) (FLT_MAX));

        return true;
}


/** deinitialize this module */
void ui_setup_props_deinit()
{
        g_object_unref(_builder);
}

/** show "hardware initialized" image */
void ui_setup_props_hardware_initialized_image(gboolean is_initialized)
{
        /* show correct image */
        gtk_widget_set_visible(GTK_WIDGET(UI("image_initialized")),
                               is_initialized);
        gtk_widget_set_visible(GTK_WIDGET(UI("image_deinitialized")),
                               !is_initialized);
}
