#pragma once
#ifndef _UI_CONTROL_H_
#define _UI_CONTROL_H_
#include <gtk/gtk.h>

void radio_datatype(GtkWidget *widget, gpointer data);
void radio_bayerpattern(GtkWidget *widget, gpointer data);

void hscale_exposure_up(GtkRange *widget);
void hscale_gain_up(GtkRange *widget);
void enable_ae(GtkToggleButton *toggle_button);

void toggled_addr_length(GtkWidget* widget, gpointer data);
int addr_width_for_rw(int address_width_flag);

void register_write(GtkWidget *widget);
void register_read(GtkWidget *widget);

void capture_bmp(GtkWidget *widget);
void capture_raw(GtkWidget *widget);

void send_trigger(GtkWidget *widget);

int init_control_gui(int argc, char* argv[]);
#endif