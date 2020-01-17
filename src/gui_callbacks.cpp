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
#include "../includes/gui_callbacks.h"
#include "../includes/extend_cam_ctrl.h"
#include "string.h"
/*****************************************************************************
**                      	Helper functions
*****************************************************************************/
/** interpret c string for both hex and decimal */
int hex_or_dec_interpreter_c_string(
    char *in_string)
{
    int out_val;
    char *hex_prefix = (char*)"0x";

    char *pch =strstr(in_string, hex_prefix);
    if (pch)
        out_val = strtol(in_string, NULL, 16);
    else
        out_val = strtol(in_string, NULL, 10);
    return out_val;
    
}

/**
 * format the entry input between hex and decimal input
 * print a warning message if it is out of a set range,
 * and set the value to a given max/min value
 * args:    
 *      GtkWidget *entry_val - entry
 *      str - string message that will be added in print out
 *      type - tell the function whether it is hex or decimal
 *      max, min - max, min value for that entry range
 */
int entry_formater(
    GtkWidget *entry_val, 
    const char* str,
    const char type,
    int max, 
    int min)
{
    int val = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_val)));
    if (type == 'x' && (val > max || val < min)) // hex
    {
        g_print("Please enter a valid number for %-20s,"
            " within range (0x%x,0x%x)\r\n", 
            str, min, max);
    }
    else if (type == 'd' && (val > max || val < min))// int
    {
        g_print("Please enter a valid number for %-12s,"
            " within range (%d,%d)\r\n", 
            str, min, max);
    }
    return set_limit(val, max, min);
}

/** 
 * a toggle button enable/disable wrapper
 * args:
 *      GtkToggleButton *toggle_button - toggle_button,
 *      str - string message that will be added in print out
 *      *func - function pointer for that enable/disable function
 */
void enable_flg_formater(
    GtkToggleButton *toggle_button, 
    const char* str, 
    void(*func)(int))
{
     if (gtk_toggle_button_get_active(toggle_button))
     {
         g_print("%s enable\r\n", str);
         func(1);
     }
     else 
     {
         g_print("%s disable\r\n", str);
         func(0);
     }
}

/** a wapper for set address and value length */
int toggle_length_formater(char* data)
{
    if (strcmp(data, "1") == 0)
        return _8BIT_FLG;
    if (strcmp(data, "2") == 0)
        return _16BIT_FLG;
    return _8BIT_FLG;
}

/** a hscale wrapper to save some type */
void hscale_formater(
    GtkRange *range,
    void(*func)(int))
{
    func((int)gtk_range_get_value(range));
}

/** a wrapper for a horizontal scale initialization */
GtkWidget* gtk_hscale_new(
    int range_low, 
    int range_max)
{
    GtkWidget* widget = 
        gtk_scale_new_with_range(
            GTK_ORIENTATION_HORIZONTAL,
            (double)range_low, 
            (double)range_max, 
            1.0);
    return widget;
                    
}

/** add radio button in their corresponding group */
GtkWidget* gtk_radio_button_new_with_group(
    GtkWidget* group)
{
  GtkWidget* new_widget = 
    gtk_radio_button_new(gtk_radio_button_get_group
    (GTK_RADIO_BUTTON(group)));
  return new_widget;
}

/** OV580 device flag */
int ov580_dev_formater(char* data)
{
    if (strcmp(data, "0") == 0)
        return _OV580_FLG;
    if (strcmp(data, "1") == 0)
        return _SCCB0_FLG;
    if (strcmp(data, "2") == 0)
        return _SCCB1_FLG;
    return _OV580_FLG;
}

/**-------------------------grid1 callbacks---------------------------------*/
// /** callback for enabling/disabling resize window in grid3 */
void enable_resize_window(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "resize window", 
        &resize_window_enable);
}

/** callback for enabling/disabling display camera info */
void enable_display_mat_info(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Stream info display enable", 
        &display_info_enable);
}
/** callback for bayer pattern choice updates*/
void radio_bayerpattern(GtkWidget *widget, gpointer data)
{
    (void)widget;
    change_bayerpattern(data);
}

/** callback for enabling/disabling auto white balance */
void enable_awb(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "awb", 
        &awb_enable);
}

/** callback for enabling/disabling CLAHE optimization */
void enable_clahe(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "contrast limited adaptive histogram equalization", 
        &clahe_enable);
}

/** callback for capturing bmp */
void capture_bmp()
{
    video_capture_save_bmp();
}

/** callback for capturing raw */
void capture_raw()
{
    video_capture_save_raw();
}

/**-------------------------grid2 callbacks-------------------------------*/
/** callback for enabling/disabling image flip */
void enable_flip(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Flip", 
        &flip_enable);
}
/** callback for enabling/disabling image mirror */
void enable_mirror(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Mirror", 
        &mirror_enable);
}

/** 
 * callback for histogram
 */
void enable_histogram(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "show histogram", 
        &histogram_enable);
}
/** callback for motion detector*/
void enable_motion_detect(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "motion detector", 
        &motion_detector_enable);
}

/** callback for enabling/disabling separate show dual stereo cam */
void enable_display_dual_stereo(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Separate display dual stereo cam", 
        &separate_dual_enable);
}

/** callback for enabling/disabling auto white balance */
void enable_abc(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Auto brightness and contrast enable", 
        &abc_enable);
}
/** callback for update alpha value */
void hscale_alpha_up(GtkRange *widget)
{
    hscale_formater(widget, &add_alpha_val);
}
/** callback for update beta value */
void hscale_beta_up(GtkRange *widget)
{
    hscale_formater(widget, &add_beta_val);
}
/** callback for update sharpness value */
void hscale_sharpness_up(GtkRange *widget)
{
    hscale_formater(widget, &add_sharpness_val);
}
/** callback for update edge threashold value */
void hscale_edge_thres_up(GtkRange *widget)
{
    hscale_formater(widget, &add_edge_thres_val);
}

/** 
 * callback for enabling/disabling displaying RGB IR color
 * channel after color correction
 */
void enable_rgb_ir_color(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "RGB-IR color correction", 
        &rgb_ir_correction_enable);
}

/** allback for enabling/disabling displaying RGB IR IR channel */
void enable_rgb_ir_ir(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "RGB-IR IR display", 
        &rgb_ir_ir_display_enable);
}

