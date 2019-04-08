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

void radio_datatype(GtkWidget *widget, gpointer data);
void radio_bayerpattern(GtkWidget *widget, gpointer data);

void hscale_exposure_up(GtkRange *widget);
void hscale_gain_up(GtkRange *widget);

void enable_ae(GtkToggleButton *toggle_button);
void enable_awb(GtkToggleButton *toggle_button);
void enable_ag(GtkToggleButton *toggle_button);

void toggled_addr_length(GtkWidget* widget, gpointer data);
int addr_width_for_rw(int address_width_flag);

void register_write(GtkWidget *widget);
void register_read(GtkWidget *widget);

void capture_bmp(GtkWidget *widget);
void capture_raw(GtkWidget *widget);

void send_trigger(GtkWidget *widget);
void enable_trig(GtkWidget *widget);
int init_control_gui(int argc, char* argv[]);
