/*****************************************************************************
 * This file is part of the Linux Camera Tool 
 * Copyright (c) 2020 Leopard Imaging Inc.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*                                                                            
*  This is the sample code for Leopard USB3.0 camera, mainly for camera tool 
*  control GUI using Gtk3. Gtk3 and Gtk2 don't live together peaceful. If you
*  have problem running Gtk3 with your current compiled openCV, please refer 
*  to README.md guide to rebuild your OpenCV for supporting Gtk3.            
*                                                                            
*  Author: Danyu L                                                           
*  Last edit: 2020/01                                                        
*****************************************************************************/
#pragma once
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


/****************************************************************************
**                      	Global data 
*****************************************************************************/

/// Hold init data for GTK signals
typedef struct
{
  const gchar *signal_name;
  const gchar *signal;
  GCallback handler;
  gpointer data;
} window_signal;

typedef enum
{
  GTK_WIDGET_TYPE_LABEL = 0,
  GTK_WIDGET_TYPE_BUTTON,
  GTK_WIDGET_TYPE_VBOX,
  GTK_WIDGET_TYPE_RADIO_BUTTON,
  GTK_WIDGET_TYPE_CHECK_BUTTON,
  GTK_WIDGET_TYPE_HSCALE,
  GTK_WIDGET_TYPE_ENTRY,
  GTK_WIDGET_TYPE_COMBO_BOX,
} widget_type;

typedef struct
{
  GtkWidget *widget;
  widget_type wid_type;
  GtkWidget *parent;
  const gchar *label_str;
} def_element;


typedef struct
{
  GtkWidget *widget;
  int col;
  int row;
  int width;
} grid_elements;

typedef struct 
{
  GtkWidget *widget;
  const gchar *signal;
  GCallback handler;
  gpointer data;
} element_callback;
void update_frame_rate(GtkWidget *widget);
void update_resolution(GtkWidget *widget);

/*****************************************************************************
**                      	Helper functions
*****************************************************************************/
void init_sensitivity();

void menu_item_formater(
    GtkWidget *item,
    GtkWidget *menu,
    GCallback handler);

/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
/**-------------------------menu bar callbacks------------------------------*/
void open_config_dialog(
  GtkWidget *widget, 
  gpointer window);
void fw_update_clicked (
  GtkWidget *item);
void about_info();
void exit_from_help(
  GtkWidget *widget);
/**-------------------------grid1 callbacks---------------------------------*/
void radio_datatype(
  GtkWidget *widget, 
  gpointer data);

void toggled_addr_length(
  GtkWidget *widget, 
  gpointer data);
void toggled_val_length(
  GtkWidget *widget, 
  gpointer data);


void hscale_exposure_up(
  GtkRange *widget);
void hscale_gain_up(
  GtkRange *widget);

void enable_ae(
  GtkToggleButton *toggle_button);

void enable_trig(
  GtkToggleButton *toggle_button);


void register_write();
void register_read();

void gamma_correction();
void send_trigger();

void black_level_correction();
/**-------------------------grid2 callbacks-------------------------------*/
void set_rgb_gain_offset();
void set_rgb_matrix();

void enable_soft_ae(
  GtkToggleButton *toggle_button);

void enable_show_edge(
  GtkToggleButton *toggle_button);

/**-------------------------grid3 callbacks-------------------------------*/
void update_ov580_dev(
  GtkWidget *widget, 
  gpointer data);
void ov580_register_write();
void ov580_register_read();
/**-------------------------micellanous callbacks---------------------------*/
void exit_loop(GtkWidget *widget);
gboolean check_escape(
  GtkWidget *widget, 
  GdkEventKey *event);


/*****************************************************************************
**                      	GUI Layout Setup, DON'T CHANGE
*****************************************************************************/
void init_grid1_widgets();
void init_grid2_widgets();
void init_grid3_widgets();

void iterate_def_elements(
  def_element *definitions, 
  size_t members);

void init_grid1_def_elements();
void init_grid2_def_elements();
void init_grid3_def_elements();

void iterate_grid_elements(
  GtkWidget *grid,
  grid_elements *elements, 
  size_t members);

void list_all_grid1_elements(
  GtkWidget *grid);
void list_all_grid2_elements(
  GtkWidget *grid);
void list_all_grid3_elements(
  GtkWidget *grid);

void iterate_element_cb(
  element_callback *callbacks,
  size_t members);
void list_all_grid1_element_callbacks();
void list_all_grid2_element_callbacks();
void list_all_grid3_element_callbacks();
void universal_ctrl_setup(GtkWidget *maintable);
void iterate_window_signals(
  GtkWidget *widget,
  window_signal *signals, 
  size_t members);
void list_all_window_signals(
  GtkWidget *window);


/*****************************************************************************
**                      	Main GUI
*****************************************************************************/
int gui_init();
void grid_layout_setup(GtkWidget *grid);
void grid1_setup(GtkWidget *grid);
void grid2_setup(GtkWidget *grid);
void grid3_setup(GtkWidget *grid);

void menu_bar_setup(GtkWidget *maintable);
void notebook_setup(GtkWidget *maintable);
void statusbar_setup(GtkWidget *maintable);

void css_setup();

void gui_run();
void ctrl_gui_main(int socket);
//void ctrl_gui_main();