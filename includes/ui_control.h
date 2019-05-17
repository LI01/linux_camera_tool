/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for the camera tool
  control GUI using Gtk3. Gtk3 asnd Gtk2 don't live together paceful. If you 
  have problem running Gtk3 with your current compiled openCV, please refer to
  README.md guide to rebuild your opencv for supporting Gtk3.
  
  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/
#pragma once
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
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

void basic_device_info_row();
void datatype_choice_row();
void bayer_pattern_choice_row();
void three_a_ctrl_row();
void gain_exposure_ctrl_row();
void i2c_addr_row();
void reg_addr_width_row();
void reg_val_width_row();
void i2c_reg_addr_row();
void i2c_reg_val_row();
void captures_row();
void gamma_correction_row();
void triggering_row();
void black_level_correction_row();

void exit_loop(GtkWidget *widget);

void init_control_gui();