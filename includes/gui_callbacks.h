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
 *  control GUI GTK3 callbacks                                                
 *                                                                            
 *  Author: Danyu L                                                           
 *  Last edit: 2020/01                                                        
*****************************************************************************/
#pragma once
#include <gtk/gtk.h>
/*****************************************************************************
**                      	Helper functions
*****************************************************************************/
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
GtkWidget* gtk_radio_button_new_with_group(
    GtkWidget* group);

void enable_resize_window(GtkToggleButton *toggle_button);
void enable_display_mat_info(GtkToggleButton *toggle_button);
/**-------------------------grid1 callbacks-------------------------------*/
void radio_bayerpattern(GtkWidget *widget, gpointer data);
void enable_awb(GtkToggleButton *toggle_button);
void enable_clahe(GtkToggleButton *toggle_button);

void capture_bmp();
void capture_raw();
/**-------------------------grid2 callbacks-------------------------------*/
void enable_flip(GtkToggleButton *toggle_button);
void enable_mirror(GtkToggleButton *toggle_button);
void enable_histogram(GtkToggleButton *toggle_button);
void enable_motion_detect(GtkToggleButton *toggle_button);
void enable_display_dual_stereo(GtkToggleButton *toggle_button);
void enable_abc(GtkToggleButton *toggle_button);

void hscale_alpha_up(
  GtkRange *widget);
void hscale_beta_up(
  GtkRange *widget);
void hscale_sharpness_up(
  GtkRange *widget);
void hscale_edge_thres_up(
  GtkRange *widget);

/**-------------------------grid3 callbacks-------------------------------*/
void enable_rgb_ir_color(
  GtkToggleButton *toggle_button);
void enable_rgb_ir_ir(
  GtkToggleButton *toggle_button);

  