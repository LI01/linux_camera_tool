/*****************************************************************************
*  This program is free software; you can redistribute it and/or modify      *
*  it under the terms of the GNU General Public License as published by      *
*  the Free Software Foundation; either version 2 of the License, or         *
*  (at your option) any later version.                                       *
*                                                                            *
*  This program is distributed in the hope that it will be useful,           *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU General Public License for more details.                              *
*                                                                            *
*  You should have received a copy of the GNU General Public License along   *
*  with this program; if not, write to the Free Software Foundation, Inc.,   *
*                                                                            *
*  This is the sample code for Leopard USB3.0 camera, mainly for camera tool *
*  control GUI using Gtk3. Gtk3 and Gtk2 don't live together peaceful. If you* 
*  have problem running Gtk3 with your current compiled openCV, please refer * 
*  to README.md guide to rebuild your OpenCV for supporting Gtk3.            *
*                                                                            *
*  Author: Danyu L                                                           *
*  Last edit: 2019/08                                                        *
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

typedef enum 
{
  _8BIT_FLG = 1,
  _16BIT_FLG = 2
}reg_addr_val_width_flag;

typedef enum 
{
  _OV580_FLG = 0,
  _SCCB0_FLG,
  _SCCB1_FLG
}ov580_flag;

/*****************************************************************************
**                      	Helper functions
*****************************************************************************/
void init_sensitivity();

int hex_or_dec_interpreter_c_string(
  char *in_string);

int entry_formater(
    GtkWidget *entry_val, 
    const char* str,
    const char type,
    int max, 
    int min);

void enable_flg_formater(
    GtkToggleButton *toggle_button, 
    const char* str, 
    void(*func)(int));

int ov580_dev_formater(char* data);
int toggle_length_formater(
  char* data);

void hscale_formater(
    GtkRange *range,
    void(*func)(int));
GtkWidget* gtk_hscale_new(
    int range_low, 
    int range_max);

void menu_item_formater(
    GtkWidget *item,
    GtkWidget *menu,
    GCallback handler);

GtkWidget* gtk_radio_button_new_with_group(
    GtkWidget* group);
/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
void enable_resize_window(
  GtkToggleButton *toggle_button);
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
void radio_bayerpattern(
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
void enable_awb(
  GtkToggleButton *toggle_button);
void enable_clahe(
  GtkToggleButton *toggle_button);
void enable_trig(
  GtkToggleButton *toggle_button);


void register_write();
void register_read();

void capture_bmp();
void capture_raw();

void gamma_correction();
void send_trigger();

void black_level_correction();
/**-------------------------grid2 callbacks-------------------------------*/
void set_rgb_gain_offset();
void set_rgb_matrix();

void enable_soft_ae(
  GtkToggleButton *toggle_button);
void enable_flip(
  GtkToggleButton *toggle_button);
void enable_mirror(
  GtkToggleButton *toggle_button);
void enable_show_edge(
  GtkToggleButton *toggle_button);
void enable_rgb_ir_color(
  GtkToggleButton *toggle_button);
void enable_rgb_ir_ir(
  GtkToggleButton *toggle_button);
void enable_display_dual_stereo(
  GtkToggleButton *toggle_button);
void enable_display_mat_info(
  GtkToggleButton *toggle_button);
void enable_abc(
  GtkToggleButton *toggle_button);

void hscale_alpha_up(
  GtkRange *widget);
void hscale_beta_up(
  GtkRange *widget);
void hscale_sharpness_up(
  GtkRange *widget);
void hscale_edge_thres_up(
  GtkRange *widget);
/**-------------------------grid3 callbacks-------------------------------*/
void update_ov580_dev(
  GtkWidget *widget, 
  gpointer data);
void ov580_register_write();
void ov580_register_read();
// void ov580_register_write(GtkWidget *widget, gpointer data);
// void ov580_register_read(GtkWidget *widget, gpointer data);
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