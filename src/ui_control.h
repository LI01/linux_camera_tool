#pragma once
#ifndef _UI_CONTROL_H_
#define _UI_CONTROL_H_
#include <gtk/gtk.h>

void reg_update(GtkWidget *widget, gpointer data);
//void trim_leading_0x(char *hex_val);
void toggled_addr_length(GtkWidget* widget, gpointer data);

void hscale_exposure_up(GtkRange *widget, gpointer data);
void hscale_gain_up(GtkRange *widget, gpointer data);
void radio_datatype(GtkWidget* widget, gpointer data);

int init_control_gui(int argc, char* argv[]);
#endif