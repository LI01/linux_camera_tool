/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for the camera tool
  control GUI using Gtk3. Gtk3 asnd Gtk2 don't live together paceful. If you 
  have problem running Gtk3 with your current compiled openCV, please refer to
  README.md guide to rebuild your opencv for supporting Gtk3.
  
  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#pragma once
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


/****************************************************************************
**                      	Global data 
*****************************************************************************/
// Hold init data for GTK signals
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


/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
int gui_attach_gtk3_menu(GtkWidget *parent);

void open_config_dialog(GtkWidget *widget, gpointer window);
void config_profile_clicked (GtkWidget *item);
void fw_update_clicked (GtkWidget *item);

void radio_datatype(GtkWidget *widget, gpointer data);
void radio_bayerpattern(GtkWidget *widget, gpointer data);

void hscale_exposure_up(GtkRange *widget);
void hscale_gain_up(GtkRange *widget);

void enable_ae(GtkToggleButton *toggle_button);
void enable_awb(GtkToggleButton *toggle_button);
void enable_abc(GtkToggleButton *toggle_button);

void toggled_addr_length(GtkWidget *widget, gpointer data);
void toggled_val_length(GtkWidget *widget, gpointer data);
int addr_width_for_rw(int address_width_flag);
int val_width_for_rw(int value_width_flag);

void register_write(GtkWidget *widget);
void register_read(GtkWidget *widget);

void capture_bmp(GtkWidget *widget);
void capture_raw(GtkWidget *widget);

void gamma_correction(GtkWidget *widget);
void black_level_correction(GtkWidget *widget);

void send_trigger(GtkWidget *widget);
void enable_trig(GtkWidget *widget);

void exit_loop(GtkWidget *widget);
gboolean check_escape(GtkWidget *widget, GdkEventKey *event);

/*****************************************************************************
**                      	GUI Layout Setup, DON'T CHANGE
*****************************************************************************/

void iterate_def_elements(
    def_element *definitions, size_t members);

void list_all_def_elements();

void iterate_grid_elements(
    grid_elements *elements, size_t members);
void list_all_grid_elements();

void iterate_element_cb(element_callback *callbacks,
                        size_t members);
void list_all_element_callbacks();

void iterate_window_signals(GtkWidget *widget,
                            window_signal *signals, size_t members);
void list_all_window_signals(GtkWidget *window);

void init_all_widgets();
/*****************************************************************************
**                      	Main GUI
*****************************************************************************/
int gui_init();
void grid_setup();
void menu_bar_setup();
void gui_run();
void ctrl_gui_main();
