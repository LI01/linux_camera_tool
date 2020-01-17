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

#include "../includes/shortcuts.h"
#include "../includes/gui_callbacks.h"
#include "../includes/ui_control.h"
#include "../includes/v4l2_devices.h"
#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/cam_property.h"
#include "../includes/json_parser.h"
#include "../includes/core_io.h"
#include "../includes/gitversion.h"
#include "../includes/fd_socket.h"
#include "../includes/batch_cmd_parser.h"
#include <iostream>
#include <string>
#include <vector>
/*****************************************************************************
**                      	Global data 
*****************************************************************************/
static GtkWidget *window;
/// top grid elements
static GtkWidget *check_button_resize_window, *button_exit_streaming;
static GtkWidget *check_button_stream_info;
static GtkWidget *separator_top_tab;
/// grid 1 elements
static GtkWidget *label_datatype, *hbox_datatype;
static GtkWidget *radio_raw10, *radio_raw12, *radio_yuyv, *radio_raw8, *radio_mjpeg;
static GtkWidget *label_bayer, *hbox_bayer;
static GtkWidget *radio_bg, *radio_gb, *radio_rg, *radio_gr, *radio_mono;
static GtkWidget *check_button_auto_exp, *check_button_awb, *check_button_clahe;
static GtkWidget *label_exposure, *label_gain;
static GtkWidget *hscale_exposure, *hscale_gain;
static GtkWidget *label_i2c_addr, *entry_i2c_addr;
static GtkWidget *label_addr_width, *hbox_addr_width;
static GtkWidget *radio_8bit_addr, *radio_16bit_addr;
static GtkWidget *label_val_width, *hbox_val_width;
static GtkWidget *radio_8bit_val, *radio_16bit_val;
static GtkWidget *label_reg_addr, *entry_reg_addr;
static GtkWidget *label_reg_val, *entry_reg_val;
static GtkWidget *button_read, *button_write;
static GtkWidget *check_button_just_sensor;
static GtkWidget *label_capture, *button_capture_bmp, *button_capture_raw;
static GtkWidget *label_gamma, *entry_gamma, *button_apply_gamma;
static GtkWidget *label_trig, *check_button_trig_en, *button_trig;
static GtkWidget *label_blc, *entry_blc, *button_apply_blc;
static GtkWidget *separator_tab1_1, *separator_tab1_2;
/// grid2 elements
static GtkWidget *check_button_rgb_gain, *button_update_rgb_gain;
static GtkWidget *label_red_gain, *label_green_gain, *label_blue_gain;
static GtkWidget *entry_red_gain, *entry_green_gain, *entry_blue_gain;
static GtkWidget *label_red_offset, *label_green_offset, *label_blue_offset;
static GtkWidget *entry_red_offset, *entry_green_offset, *entry_blue_offset;
static GtkWidget *separator_tab2_1, *separator_tab2_2;
static GtkWidget *check_button_rgb_matrix, *button_update_rgb_matrix;
static GtkWidget *label_red_hor, *label_green_hor, *label_blue_hor;
static GtkWidget *label_red_ver, *label_green_ver, *label_blue_ver;
static GtkWidget *entry_rr, *entry_rg, *entry_rb; 
static GtkWidget *entry_gr, *entry_gg, *entry_gb; 
static GtkWidget *entry_br, *entry_bg, *entry_bb; 
static GtkWidget *check_button_soft_ae , *check_button_flip;
static GtkWidget *check_button_edge, *check_button_mirror;
static GtkWidget *check_button_histogram, *check_button_motion_detect;
static GtkWidget *check_button_dual_stereo;
static GtkWidget *label_alpha, *label_beta, *label_sharpness;
static GtkWidget *check_button_abc;
static GtkWidget *hscale_alpha, *hscale_beta, *hscale_sharpness;
static GtkWidget *label_edge_low_thres, *hscale_edge_low_thres;
/// grid3 elements
static GtkWidget *label_ov580_device, *hbox_ov580_device;
static GtkWidget *radio_ov580, *radio_sccb0, *radio_sccb1; 
static GtkWidget *label_ov580_i2c_addr, *entry_ov580_i2c_addr;
static GtkWidget *label_ov580_reg_addr, *entry_ov580_reg_addr;
static GtkWidget *label_ov580_reg_val, *entry_ov580_reg_val;
static GtkWidget *button_ov580_read, *button_ov580_write;
static GtkWidget *separator_tab3_1;
static GtkWidget *check_button_rgb_ir_color, *check_button_rgb_ir_ir;
static GtkWidget *label_resolution, *combo_box_resolution;
static GtkWidget *label_frame_rate, *combo_box_frame_rate;

static int ov580_dev_flag;
static int address_width_flag;
static int value_width_flag;

int v4l2_dev; /** global variable, file descriptor for camera device */
extern std::vector<std::string> resolutions;
extern std::vector<int> cur_frame_rates;
/*****************************************************************************
**                      	Helper functions
*****************************************************************************/
void update_resolution(GtkWidget *widget)
{
    g_print("Update Resolution\n");
    char cur_fps[10];
    
    int cmd_index =(int)gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    video_change_res(cmd_index);

    sleep(1);
    int cur_frame_rate = get_frame_rate(v4l2_dev);
    snprintf(cur_fps, sizeof(cur_fps), "%d fps", cur_frame_rate);
   
    /*disable fps combobox signals*/
	g_signal_handlers_block_by_func(
        GTK_COMBO_BOX(combo_box_frame_rate), 
        (gpointer)(update_frame_rate), NULL);
	/* clear out the old fps list... */
	GtkListStore *store = GTK_LIST_STORE(
        gtk_combo_box_get_model (GTK_COMBO_BOX(combo_box_frame_rate)));
	gtk_list_store_clear(store);

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(combo_box_frame_rate), 
        cur_fps);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box_frame_rate),0);
    int cur_fps_size = cur_frame_rates.size();
    for(int i=0; i<cur_fps_size; i++) {
        if (cur_frame_rate != cur_frame_rates[i]) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%d fps", cur_frame_rates[i]);
            gtk_combo_box_text_append_text(
                GTK_COMBO_BOX_TEXT(combo_box_frame_rate), 
                buf);     
        }
    }
  
}
void update_frame_rate(GtkWidget *widget)
{
    g_print("Update Frame Rate\n");
    
    int cur_fps_size = cur_frame_rates.size();
    int cur_frame_rate = get_frame_rate(v4l2_dev);
    for(int i=0; i<cur_fps_size; i++) {
        if (cur_frame_rate != cur_frame_rates[i]) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%d fps", cur_frame_rates[i]);
            gtk_combo_box_text_append_text(
                GTK_COMBO_BOX_TEXT(combo_box_frame_rate), 
                buf);     
        }
    }
    int cmd_index =(int)gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    set_restart_flag(1);
    set_frame_rate(v4l2_dev, cur_frame_rates[cmd_index]);
}
/** a menu_item formater to save some typing */
void menu_item_formater(
    GtkWidget *item,
    GtkWidget *menu,
    GCallback handler)
{
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    g_signal_connect(item, "activate", handler, window);
}


/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
/**-------------------------menu bar callbacks------------------------------*/
/** callback for find and load config files */
void open_config_dialog(GtkWidget *widget, gpointer window)
{
    (void)widget;
    GtkWidget *file_dialog = gtk_file_chooser_dialog_new(
        "Load Config File",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        ("_Open"),
        GTK_RESPONSE_OK,
        ("_Cancel"),
        GTK_RESPONSE_CANCEL,
        NULL);
    /// put this here so no warning
    gtk_window_set_transient_for(GTK_WINDOW(file_dialog), GTK_WINDOW(window));
    gtk_widget_show(file_dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(file_dialog));
    // gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_dialog), "/");
    // gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_dialog), 
    // g_get_home_dir());

    if (resp == GTK_RESPONSE_OK) 
    {
        char *file_name = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(file_dialog));
        load_control_profile(v4l2_dev, file_name);
    }
    else
        g_print("You pressed the cancel\r\n");
    gtk_widget_destroy(file_dialog);
    
}
/** callback for fw update info */
void fw_update_clicked (GtkWidget *item)
{
    if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(item)),
         "Erase Firmware") == 0 )
    {
        /// close the camera first before erase 
        set_loop(0);
        firmware_erase(v4l2_dev);
        gtk_main_quit();
        
    }
    else if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(item)), 
        "Reset Camera") == 0 )
    {
        reboot_camera(v4l2_dev);
    }

}
/** callback for display linux camera tool info */
void about_info()
{

    GtkWidget *dialog_about = gtk_about_dialog_new();
  
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog_about), 
        "Linux Camera Tool");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog_about), gitversion);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog_about),
    "Copyright \xc2\xa9 Leopard Imaging Inc, 2019");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog_about),
        GTK_LICENSE_GPL_3_0);
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog_about), 
        "https://github.com/danyu9394/linux_camera_tool");
    gtk_window_set_transient_for(GTK_WINDOW(dialog_about), GTK_WINDOW(window));
    gtk_widget_show_all(dialog_about);
}
/** callback for exit when click EXIT in help */
void exit_from_help(GtkWidget *widget)
{
    exit_loop(widget);
}

/**-------------------------grid1 callbacks---------------------------------*/

/** callback for sensor datatype updates*/
void radio_datatype(GtkWidget *widget, gpointer data)
{
    (void)widget;
    /// disable a bunch of function if it is YUV,MJPEG camera
    if (strcmp((char *)data, "3") == 0 || strcmp((char *)data, "5") == 0)
    {
        //printf("datatype = %c\r\n", (char *)data);
        gtk_widget_set_sensitive(radio_rg,0);
        gtk_widget_set_sensitive(radio_gr,0);
        gtk_widget_set_sensitive(radio_gb,0);
        gtk_widget_set_sensitive(radio_bg,0);
        gtk_widget_set_sensitive(check_button_awb,0);
        gtk_widget_set_sensitive(check_button_rgb_gain,0);
        gtk_widget_set_sensitive(check_button_rgb_matrix,0);
        gtk_widget_set_sensitive(check_button_rgb_ir_color,0);
        gtk_widget_set_sensitive(check_button_rgb_ir_ir,0);
        gtk_widget_set_sensitive(button_apply_blc,0);
    }
    else 
    {
        gtk_widget_set_sensitive(radio_rg,1);
        gtk_widget_set_sensitive(radio_gr,1);
        gtk_widget_set_sensitive(radio_gb,1);
        gtk_widget_set_sensitive(radio_bg,1);
        gtk_widget_set_sensitive(check_button_awb,1);
        gtk_widget_set_sensitive(check_button_rgb_gain,1);
        gtk_widget_set_sensitive(check_button_rgb_matrix,1);
        gtk_widget_set_sensitive(check_button_rgb_ir_color,1);
        gtk_widget_set_sensitive(check_button_rgb_ir_ir,1);
        gtk_widget_set_sensitive(button_apply_blc,1);
    }
    change_datatype(data);
}

/** callback for updating exposure time line */
void hscale_exposure_up(GtkRange *widget)
{
    int exposure_time = (int)gtk_range_get_value(widget);
    set_exposure_absolute(v4l2_dev, exposure_time);
    g_print("exposure is %d lines\n",get_exposure_absolute(v4l2_dev));
}

/** callback for updating analog gain */
void hscale_gain_up(GtkRange *widget)
{
    int gain = (int)gtk_range_get_value(widget);
    set_gain(v4l2_dev, gain);
    g_print("gain is %d\n", get_gain(v4l2_dev));
}

/** callback for enabling/disabling auto exposure */
void enable_ae(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button)) {
        set_exposure_auto(v4l2_dev, V4L2_EXPOSURE_AUTO);
        gtk_widget_set_sensitive(hscale_exposure, 0);
        gtk_widget_set_sensitive(hscale_gain, 0);
    }
    else {
        set_exposure_auto(v4l2_dev, V4L2_EXPOSURE_MANUAL);
        gtk_widget_set_sensitive(hscale_exposure, 1);
        gtk_widget_set_sensitive(hscale_gain, 1);
    }
}

/** callback for updating register address length 8/16 bits */
void toggled_addr_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    address_width_flag = toggle_length_formater((char *)data);

}

/** callback for updating register value length 8/16 bits */
void toggled_val_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    value_width_flag = toggle_length_formater((char *)data);
}

/** callback for register write */
void register_write()
{
    /** sensor write */
    if (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        gtk_widget_set_sensitive(entry_i2c_addr,0);
        
        int regAddr = entry_formater(
            entry_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        if ((address_width_flag) == _8BIT_FLG)
            regAddr = regAddr & 0xff;
        
        int regVal = entry_formater(
            entry_reg_val, 
            "register value", 
            'x', 
            0xffff, 
            0x00);
        if ((value_width_flag) == _8BIT_FLG)
            regVal = regVal & 0xff;
       
        sensor_reg_write(v4l2_dev, regAddr, regVal);
    }

    /** generic i2c slave write */
    else
    {
        gtk_widget_set_sensitive(entry_i2c_addr,1);
        int slaveAddr = entry_formater(
            entry_i2c_addr, 
            "slave address", 
            'x', 
            0xff, 
            0x1);
        int regAddr = entry_formater(
            entry_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        if ((address_width_flag) == _8BIT_FLG)
            regAddr = regAddr & 0xff;
        int regVal = entry_formater(
            entry_reg_val, 
            "register value", 
            'x', 
            0xffff, 
            0x0);
        if ((value_width_flag) == _8BIT_FLG)
            regVal = regVal & 0xff;
        unsigned char buf[2];
        buf[0] = regVal & 0xff;
        buf[1] = (regVal >> 8) & 0xff;

        /** write 8/16 bit register addr and register data */
        generic_I2C_write(
            v4l2_dev, 
            (GENERIC_REG_WRITE_FLG | address_width_flag),         
            value_width_flag,             
            slaveAddr, 
            regAddr, 
            buf);
    }
}

/** callback for register read */
void register_read()
{
    /** sensor read */
    if (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        int regAddr = entry_formater(
            entry_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        if ((address_width_flag) == _8BIT_FLG)
            regAddr = regAddr & 0xff;

        int regVal = sensor_reg_read(v4l2_dev, regAddr);
        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }

    /** generic i2c slave read */
    else
    {
        int slaveAddr = entry_formater(
            entry_i2c_addr, 
            "slave address", 
            'x', 
            0xff, 
            0x1);
        int regAddr = entry_formater(
            entry_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        if ((address_width_flag) == _8BIT_FLG)
            regAddr = regAddr & 0xff;

        int regVal = generic_I2C_read(
            v4l2_dev, 
            GENERIC_REG_READ_FLG | address_width_flag, 
            value_width_flag, 
            slaveAddr, 
            regAddr);

        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }
}

/** callback for gamma correction */
void gamma_correction()
{
    float gamma = atof((char *)gtk_entry_get_text(
        GTK_ENTRY(entry_gamma)));
    if (isgreater(gamma, 0.0)) 
    {
        add_gamma_val(gamma);
        g_print("gamma = %f\n", gamma);
    }
    else
        g_print("gamma should be greater than 0," 
        "you entered gamma = %f\n", gamma);
}

/** callback for triggering sensor functionality */
void send_trigger()
{
    if (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(check_button_trig_en)))
    {
        g_print("trigger enabled\n");
        soft_trigger(v4l2_dev);
        g_print("send one trigger\n");
    }
    else
    {
        g_print("trigger disabled\n");
    }
}


/** callback for enabling/disalign sensor trigger */
void enable_trig(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        /** positive edge */
        trigger_enable(v4l2_dev, 1, 1);
        // /** negative edge */
        // trigger_enable(v4l2_dev, 1, 0);
    }
    else
    {
        /** disable trigger */
        trigger_enable(v4l2_dev, 0, 0);
    }
}

/** callback for black level correction ctrl */
void black_level_correction()
{
    int blc = entry_formater(
        entry_blc, 
        "black level correction", 
        'd', 
        512, 
        -512);
    add_blc(blc);
}

/**-------------------------grid2 callbacks-------------------------------*/
/** 
 * callback for update rgb gain
 * limit the input on MAX/MIN_RGB macros
 */
void set_rgb_gain_offset()
{
    int red_gain    = entry_formater(
        entry_red_gain,    
        "red gain",    
        'd', 
        MAX_RGB_GAIN, 
        MIN_RGB_GAIN);
    int green_gain  = entry_formater(
        entry_green_gain,  
        "green gain",  
        'd', 
        MAX_RGB_GAIN, 
        MIN_RGB_GAIN);
    int blue_gain   = entry_formater(
        entry_blue_gain,   
        "blue gain",   
        'd', 
        MAX_RGB_GAIN, 
        MIN_RGB_GAIN);
    int red_offset  = entry_formater(
        entry_red_offset,  
        "red offset",  
        'd', 
        MAX_RGB_OFFSET
        , MIN_RGB_OFFSET);
    int green_offset = entry_formater(
        entry_green_offset,
        "green offset",
        'd', 
        MAX_RGB_OFFSET
        , MIN_RGB_OFFSET);
    int blue_offset = entry_formater(
        entry_blue_offset, 
        "blue offset", 
        'd', 
        MAX_RGB_OFFSET
        , MIN_RGB_OFFSET);

    if (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(check_button_rgb_gain))) {
        enable_rgb_gain_offset (
            red_gain,   green_gain,   blue_gain, 
            red_offset, green_offset, blue_offset);
    }
    else
        disable_rgb_gain_offset(); 
}

/**
 *  callback for update rgb2rgb color matrix
 *  limit the input on MAX/MIN_RGB macros
 * 
 *  http://www.imatest.com/docs/colormatrix/
 */
void set_rgb_matrix()
{    

    int red_red     = entry_formater(entry_rr,    "red red", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int red_green   = entry_formater(entry_rg,  "red green", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int red_blue    = entry_formater(entry_rb,   "red blue", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int green_red   = entry_formater(entry_gr,  "green red", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int green_green = entry_formater(entry_gg,"green green", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int green_blue  = entry_formater(entry_gb, "green blue", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int blue_red    = entry_formater(entry_br,   "blue red", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int blue_green  = entry_formater(entry_bg, "blue green", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);
    int blue_blue   = entry_formater(entry_bb,  "blue blue", 'd', 
    MAX_RGB_MATRIX, MIN_RGB_MATRIX);

    if (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(check_button_rgb_matrix))) {
        enable_rgb_matrix (
            red_red,   red_green,   red_blue, 
            green_red, green_green, green_blue,
            blue_red,  blue_green,  blue_blue);
    }
    else
        disable_rgb_matrix();    
    
}

/** callback for enabling/disabling auto white balance */
void enable_soft_ae(GtkToggleButton *toggle_button)
{

    enable_flg_formater(
        toggle_button, 
        "Software AE", 
        &soft_ae_enable);

    if (gtk_toggle_button_get_active(toggle_button))
    {
        /// disable change for exposure and gain
        gtk_widget_set_sensitive(hscale_exposure, 0); 
        gtk_widget_set_sensitive(hscale_gain,0);
    }
    else
    {
        /// enable change for exposure and gain
        gtk_widget_set_sensitive(hscale_exposure, 1); 
        gtk_widget_set_sensitive(hscale_gain,1);    
    }
}

/** callback for enabling/disabling show image edge */
void enable_show_edge(GtkToggleButton *toggle_button)
{
    enable_flg_formater(
        toggle_button, 
        "Show canny edge", 
        &canny_filter_enable);
    if (gtk_toggle_button_get_active(toggle_button))
        gtk_widget_set_sensitive(hscale_edge_low_thres,1);
    else 
        gtk_widget_set_sensitive(hscale_edge_low_thres,0);

}
/**-------------------------grid3 callbacks-------------------------------*/
/** 
 * callback for updating the ov580 device flag 
 * it changes device, clears the entries 
 */
void update_ov580_dev(GtkWidget *widget, gpointer data)
{
    (void)widget;
    static int last_flg = _OV580_FLG;
    ov580_dev_flag = ov580_dev_formater((char *)data);
    if (last_flg != ov580_dev_flag)
    {
        last_flg = ov580_dev_flag;
        gtk_entry_set_text(GTK_ENTRY(entry_ov580_reg_val), "0x");
        gtk_entry_set_text(GTK_ENTRY(entry_ov580_reg_addr), "0x");
        gtk_entry_set_text(GTK_ENTRY(entry_ov580_i2c_addr), "0x");
    }
    if (ov580_dev_flag == _OV580_FLG)
        gtk_widget_set_sensitive(entry_ov580_i2c_addr, 0); 
    else
        gtk_widget_set_sensitive(entry_ov580_i2c_addr, 1); 
}

/** callback for ov580 register write */
void ov580_register_write()
{
    if ((ov580_dev_flag) == _OV580_FLG)
    {
        int regAddr = entry_formater(
            entry_ov580_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        unsigned char regVal = entry_formater(
            entry_ov580_reg_val, 
            "register value", 
            'x', 
            0xff, 
            0x0); 
  
        ov580_write_system_reg(v4l2_dev, regAddr, regVal);

    } else {
        unsigned char slaveAddr = entry_formater(
            entry_ov580_i2c_addr, 
            "slave address", 
            'x', 
            0xff, 
            0x1);
        int regAddr = entry_formater(
            entry_ov580_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        unsigned char regVal = entry_formater(
            entry_ov580_reg_val, 
            "register value", 
            'x', 
            0xff, 
            0x00);
     
        if ((ov580_dev_flag) == _SCCB0_FLG)
            ov580_write_sccb0_reg(v4l2_dev, slaveAddr, regAddr, regVal);
        else if ((ov580_dev_flag) == _SCCB1_FLG)
            ov580_write_sccb1_reg(v4l2_dev, slaveAddr, regAddr, regVal);

    }
}

/** callback for ov580 register read */
void ov580_register_read()
{
    if ((ov580_dev_flag) == _OV580_FLG)
    {
        int regAddr = entry_formater(
            entry_ov580_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        printf("reg addr = 0x%x\r\n", regAddr);
        unsigned char regVal = ov580_read_system_reg(v4l2_dev, regAddr);
        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_ov580_reg_val), buf);
    } else {
        unsigned char slaveAddr = entry_formater(
            entry_ov580_i2c_addr, 
            "slave address", 
            'x', 
            0xff, 
            0x1);
        int regAddr = entry_formater(
            entry_ov580_reg_addr, 
            "register address", 
            'x', 
            0xffff, 
            0x0);
        unsigned char regVal;
        printf("reg addr = 0x%x\r\n", regAddr);
        if ((ov580_dev_flag) == _SCCB0_FLG)
            regVal = ov580_read_sccb0_reg(v4l2_dev, slaveAddr, regAddr);
        else 
            regVal = ov580_read_sccb1_reg(v4l2_dev, slaveAddr, regAddr);
        //g_print("read register 0x%x = 0x%x\r\n", regAddr, regVal);
        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_ov580_reg_val), buf);
    }
}

/**-------------------------micellanous callbacks---------------------------*/
/** exit streaming loop */
void exit_loop(GtkWidget *widget)
{
    (void)widget;
    set_loop(0);
    gtk_main_quit();
    g_print("exit\n");
}

/** escape gui from pressing ESC */
gboolean check_escape(
    GtkWidget *widget, 
    GdkEventKey *event)
{
    (void)widget;
    if (event->keyval == GDK_KEY_Escape)
    {
        gtk_main_quit();
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
**                      	GUI Layout Setup, DON'T CHANGE
*****************************************************************************/
/** 
 * init and declare all the grid1 widgets here
 * since you couldn‘t declare inside switch, sequence doesn't matter
 */
void init_grid1_widgets()
{
      
	/**
	 * This part is to get the gain and exposure maximum value defined in 
     * firmware and update the range in GUI, if MAX_EXPOSURE_TIME is undefined
     * in firmware, it will put a upper limit for exposure time range so the 
     * range won't be as large as 65535 which confuses people
	 * If UNSET_MAX_EXPOSURE_LINE, firmware didn't define MAX_EXPOSURE_TIME...
	 * place holder = 3 * vertical_line#, it should be enough for most cases
	 * if you adjust exposure to maximum, and image is still dark, try to 
     * increase 3 to 4, 5, 6 etc, or ask for a firmware update, firmware 
     * exposure time mapping might be wrong
	 */

    int exposure_max = query_exposure_absolute_max(v4l2_dev);
    int gain_max = query_gain_max(v4l2_dev);
    
    if (exposure_max == 0)
        exposure_max = 999;
    
    if (gain_max == 0)
        gain_max = 13;

    if (exposure_max == UNDEFINED_MAX_EXPOSURE_LINE)
        exposure_max = get_current_height(v4l2_dev) * 3;



    label_datatype   = gtk_label_new(NULL);
    label_bayer      = gtk_label_new(NULL);
    label_exposure   = gtk_label_new(NULL);
    label_gain       = gtk_label_new(NULL);  
    label_i2c_addr   = gtk_label_new(NULL);
    label_addr_width = gtk_label_new(NULL);
    label_val_width  = gtk_label_new(NULL);
    label_reg_addr   = gtk_label_new(NULL);
    label_reg_val    = gtk_label_new(NULL);
    label_capture    = gtk_label_new(NULL);  
    label_gamma      = gtk_label_new(NULL);
    label_trig       = gtk_label_new(NULL);
    label_blc        = gtk_label_new(NULL);

    button_read             = gtk_button_new();
    button_write            = gtk_button_new();
    button_apply_gamma      = gtk_button_new();
    button_trig             = gtk_button_new();
    button_capture_bmp      = gtk_button_new();
    button_capture_raw      = gtk_button_new();
    button_apply_blc        = gtk_button_new();

    check_button_auto_exp       = gtk_check_button_new();
    check_button_awb            = gtk_check_button_new();
    check_button_clahe          = gtk_check_button_new();    
    check_button_trig_en        = gtk_check_button_new();
    check_button_just_sensor    = gtk_check_button_new();

    
    entry_i2c_addr  = gtk_entry_new();
    entry_reg_addr  = gtk_entry_new();
    entry_reg_val   = gtk_entry_new();
    entry_gamma     = gtk_entry_new();
    entry_blc       = gtk_entry_new();

    hscale_exposure = gtk_hscale_new(0, exposure_max);
    hscale_gain     = gtk_hscale_new(0, gain_max);


    hbox_val_width  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_addr_width = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_bayer      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_datatype   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    radio_raw10 = gtk_radio_button_new(NULL);
    radio_raw12 = gtk_radio_button_new_with_group(radio_raw10);
    radio_yuyv  = gtk_radio_button_new_with_group(radio_raw10);
    radio_raw8  = gtk_radio_button_new_with_group(radio_raw10);
    radio_mjpeg  = gtk_radio_button_new_with_group(radio_raw10);

    radio_bg    = gtk_radio_button_new(NULL);
    radio_gb    = gtk_radio_button_new_with_group(radio_bg);
    radio_rg    = gtk_radio_button_new_with_group(radio_bg);
    radio_gr    = gtk_radio_button_new_with_group(radio_bg);
    radio_mono  = gtk_radio_button_new_with_group(radio_bg);

    radio_8bit_addr    = gtk_radio_button_new(NULL);
    radio_16bit_addr   = gtk_radio_button_new_with_group(radio_8bit_addr);

    radio_8bit_val     = gtk_radio_button_new(NULL);
    radio_16bit_val    = gtk_radio_button_new_with_group(radio_8bit_val);


   
    separator_tab1_1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    separator_tab1_2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);


    if (gain_max == 13)
        gtk_widget_set_sensitive(hscale_gain, 0);
    if (exposure_max == 999)
        gtk_widget_set_sensitive(hscale_exposure, 0);

}
/** 
 * init and declare all the grid2 widgets here
 * since you couldn‘t declare inside switch, sequence doesn't matter
 */
void init_grid2_widgets()
{
    check_button_rgb_gain       = gtk_check_button_new();
    check_button_rgb_matrix     = gtk_check_button_new();
    check_button_soft_ae        = gtk_check_button_new();
    check_button_flip           = gtk_check_button_new();
    check_button_mirror         = gtk_check_button_new();
    check_button_edge           = gtk_check_button_new();
    check_button_histogram      = gtk_check_button_new();
    check_button_motion_detect  = gtk_check_button_new();
    check_button_dual_stereo    = gtk_check_button_new();
    check_button_abc            = gtk_check_button_new();

    button_update_rgb_gain      = gtk_button_new();
    button_update_rgb_matrix    = gtk_button_new();
    
    label_red_gain              = gtk_label_new(NULL);
    label_green_gain            = gtk_label_new(NULL);
    label_blue_gain             = gtk_label_new(NULL);
    label_red_offset            = gtk_label_new(NULL);
    label_green_offset          = gtk_label_new(NULL);
    label_blue_offset           = gtk_label_new(NULL);
    label_red_hor               = gtk_label_new(NULL);
    label_green_hor             = gtk_label_new(NULL);
    label_blue_hor              = gtk_label_new(NULL);
    label_red_ver               = gtk_label_new(NULL);
    label_green_ver             = gtk_label_new(NULL);
    label_blue_ver              = gtk_label_new(NULL);
    label_alpha                 = gtk_label_new(NULL);
    label_beta                  = gtk_label_new(NULL);
    label_sharpness             = gtk_label_new(NULL);
    label_edge_low_thres        = gtk_label_new(NULL);
    
    entry_red_gain              = gtk_entry_new();
    entry_green_gain            = gtk_entry_new();
    entry_blue_gain             = gtk_entry_new();
    entry_red_offset            = gtk_entry_new();
    entry_green_offset          = gtk_entry_new();
    entry_blue_offset           = gtk_entry_new();
    entry_rr                    = gtk_entry_new();
    entry_rg                    = gtk_entry_new();
    entry_rb                    = gtk_entry_new();
    entry_gr                    = gtk_entry_new();
    entry_gg                    = gtk_entry_new();
    entry_gb                    = gtk_entry_new();
    entry_br                    = gtk_entry_new();
    entry_bg                    = gtk_entry_new();
    entry_bb                    = gtk_entry_new();

    separator_tab2_1         = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    separator_tab2_2         = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    
 
    hscale_alpha                = gtk_hscale_new(1, ALPHA_MAX);
    hscale_beta                 = gtk_hscale_new(-BETA_MAX, BETA_MAX);
    // gtk_range_set_value(GTK_RANGE(hscale_beta), 0);
    hscale_sharpness            = gtk_hscale_new(1, SHARPNESS_MAX);
    hscale_edge_low_thres       = gtk_hscale_new(0, LOW_THRESHOLD_MAX);
    // gtk_widget_set_sensitive(hscale_edge_low_thres,0);                                         
}
/** 
 * init and declare all the grid3 widgets here
 * since you couldn‘t declare inside switch, sequence doesn't matter
 */
void init_grid3_widgets()
{
    label_ov580_device   = gtk_label_new(NULL);
    label_ov580_i2c_addr = gtk_label_new(NULL);
    label_ov580_reg_addr = gtk_label_new(NULL);
    label_ov580_reg_val  = gtk_label_new(NULL);
    label_resolution     = gtk_label_new(NULL);
    label_frame_rate     = gtk_label_new(NULL);

    hbox_ov580_device    = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    radio_ov580          = gtk_radio_button_new(NULL);
    radio_sccb0          = gtk_radio_button_new_with_group(radio_ov580);
    radio_sccb1          = gtk_radio_button_new_with_group(radio_ov580);

    button_ov580_read             = gtk_button_new();
    button_ov580_write            = gtk_button_new();

    entry_ov580_i2c_addr = gtk_entry_new();
    entry_ov580_reg_addr = gtk_entry_new();
    entry_ov580_reg_val  = gtk_entry_new();

    separator_tab3_1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    check_button_rgb_ir_color   = gtk_check_button_new();
    check_button_rgb_ir_ir      = gtk_check_button_new();
    
    combo_box_resolution = gtk_combo_box_text_new();
    combo_box_frame_rate = gtk_combo_box_text_new();
    gtk_widget_set_halign (combo_box_resolution, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (combo_box_resolution, TRUE);
    gtk_widget_set_halign (combo_box_frame_rate, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (combo_box_frame_rate, TRUE);
    gtk_widget_set_sensitive(entry_ov580_i2c_addr, 0); 
}

/** def_element hold all gtk type widgets initialization setup */
void iterate_def_elements (
    def_element *definitions, size_t members)
{   
    FOREACH_NELEM(definitions, members, def)
    {

        switch (def->wid_type) 
        {
            case GTK_WIDGET_TYPE_LABEL:
                gtk_label_set_text(
                    GTK_LABEL(def->widget), 
                    def->label_str);
                break;
            case GTK_WIDGET_TYPE_BUTTON:
                gtk_button_set_label(
                    GTK_BUTTON(def->widget), 
                    def->label_str);
                break;
            case GTK_WIDGET_TYPE_VBOX:
                break;
            case GTK_WIDGET_TYPE_RADIO_BUTTON:  
                gtk_box_pack_start(
                    GTK_BOX(def->parent), 
                    def->widget, 0, 0, 0);
                gtk_button_set_label(
                    GTK_BUTTON(def->widget), 
                    def->label_str);
                break;
            case GTK_WIDGET_TYPE_CHECK_BUTTON:
                gtk_button_set_label(
                    GTK_BUTTON(def->widget), 
                    def->label_str);
                break;
            case GTK_WIDGET_TYPE_ENTRY:
                gtk_entry_set_text(
                    GTK_ENTRY(def->widget), 
                    def->label_str);
                gtk_entry_set_width_chars(
                    GTK_ENTRY(def->widget), -1);
                break;
            case GTK_WIDGET_TYPE_HSCALE:
                gtk_widget_set_hexpand(def->widget, TRUE);
                break;
            case GTK_WIDGET_TYPE_COMBO_BOX:
                gtk_combo_box_text_append_text(
                    GTK_COMBO_BOX_TEXT(def->widget), 
                    def->label_str);
                gtk_combo_box_set_active(
                    GTK_COMBO_BOX(def->widget),0);
                break;
            default:
                break;

       } 
    }
}

/** iterate over definition element for grid1 widgets setup */
void init_grid1_def_elements ()
{
 
    static def_element definitions[] = {
        {.widget = label_exposure,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Exposure:"},
        {.widget = label_gain,        
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Gain:"},   
        {.widget = label_i2c_addr,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "I2C Addr:"},
        {.widget = label_datatype,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Sensor Datatype:"},
        {.widget = label_bayer,       
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Raw Camera Pixel Format:"},
        {.widget = label_addr_width,  
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Register Addr Width:"},
        {.widget = label_val_width,   
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Register Value Width:"},
        {.widget = label_reg_addr,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Reg Addr:"},     
        {.widget = label_reg_val,     
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Reg Value:"},
        {.widget = label_trig,        
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Trigger Sensor:"},
        {.widget = label_capture,     
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Capture:"},     
        {.widget = label_gamma,       
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Gamma Correction:"},
        {.widget = label_blc,         
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Black Level Correction:"},

        {.widget = button_write,          
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Write"}, 
        {.widget = button_read,           
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Read"}, 
        {.widget = button_capture_bmp,    
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Capture bmp"},  
        {.widget = button_capture_raw,    
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Capture raw"},  
        {.widget = button_trig,           
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Shot 1 Trigger"},
        {.widget = button_apply_gamma ,   
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Apply"},   
        {.widget = button_apply_blc,      
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Apply"},

        {.widget = check_button_auto_exp,      
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Enable auto exposure"},
        {.widget = check_button_awb ,          
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Enable auto white balance"},
        {.widget = check_button_clahe ,    
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Enable CLAHE"},        
        {.widget = check_button_just_sensor,   
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Just sensor read/write"},
        {.widget = check_button_trig_en,       
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Enable"},

        {.widget = hscale_exposure,   
         .wid_type = GTK_WIDGET_TYPE_HSCALE,
         .parent =  NULL,
         .label_str = "2505"},
        {.widget = hscale_gain,       
         .wid_type = GTK_WIDGET_TYPE_HSCALE,
         .parent =  NULL,
         .label_str = "64"},
        {.widget = entry_i2c_addr,    
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},
        {.widget = entry_reg_addr,    
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},
        {.widget = entry_reg_val,     
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},    
        {.widget = entry_gamma,       
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "1"},
        {.widget = entry_blc,         
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},

        {.widget = radio_raw10, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_datatype, 
         .label_str = "RAW10"},
        {.widget = radio_raw12, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_datatype, 
         .label_str = "RAW12"},
        {.widget = radio_yuyv,  
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_datatype, 
         .label_str = "YUYV"},
        {.widget = radio_raw8,  
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_datatype, 
         .label_str = "RAW8"},
        {.widget = radio_mjpeg,  
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_datatype, 
         .label_str = "MJPEG"},

        {.widget = radio_bg,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_bayer, 
         .label_str = "BGGR"},
        {.widget = radio_gb,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_bayer, 
         .label_str = "GBRG"},
        {.widget = radio_rg,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_bayer, 
         .label_str = "RGGB"},
        {.widget = radio_gr,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_bayer, 
         .label_str = "GRBG"},
        {.widget = radio_mono, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_bayer, 
         .label_str = "MONO"},

        {.widget = radio_8bit_addr,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_addr_width,
         .label_str = "8-bit"},
        {.widget = radio_16bit_addr,  
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_addr_width,
         .label_str = "16-bit"},
        {.widget = radio_8bit_val,    
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_val_width, 
         .label_str = "8-bit"},
        {.widget = radio_16bit_val,   
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_val_width, 
         .label_str = "16-bit"},
        

    };
    iterate_def_elements(definitions, SIZE(definitions));
}
/** iterate over definition element for grid2 widgets setup */
void init_grid2_def_elements ()
{
    static def_element definitions[] = {
        {.widget = check_button_rgb_gain, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "RGBGainOffsetEna"},
        {.widget = button_update_rgb_gain, 
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Update"},
        {.widget = label_red_gain, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Red Gain:"},
        {.widget = label_green_gain, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Green Gain:"},
        {.widget = label_blue_gain, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Blue Gain:"},
        {.widget = entry_red_gain, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "512"},
        {.widget = entry_green_gain, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "512"},
        {.widget = entry_blue_gain, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "512"},    
        {.widget = label_red_offset, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Red Offset:"},
        {.widget = label_green_offset, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Green Offset:"},
        {.widget = label_blue_offset, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Blue Offset:"},
        {.widget = entry_red_offset, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_green_offset, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_blue_offset, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},  
        {.widget = check_button_rgb_matrix, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "RGB2RGBMatrixEna"},
        {.widget = button_update_rgb_matrix, 
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Update"},
        {.widget = label_red_hor, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Red:"},
        {.widget = label_green_hor, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Green:"},
        {.widget = label_blue_hor, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Blue:"},
        {.widget = label_red_ver, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Red:"},
        {.widget = label_green_ver, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Green:"},
        {.widget = label_blue_ver, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Blue:"},
        {.widget = entry_rr, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "256"},
        {.widget = entry_rg, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_rb, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_gr, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_gg, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "256"},
        {.widget = entry_gb, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_br, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_bg, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},        
        {.widget = entry_bb, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "256"},         
        {.widget = check_button_soft_ae, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Software AE"},
        {.widget = check_button_flip, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Flip Image"},
        {.widget = check_button_mirror, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Mirror Image"},
        {.widget = check_button_edge, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Show Edge"},
        {.widget = check_button_histogram, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Display Histogram"},
        {.widget = check_button_motion_detect, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Motion Detector"},
        {.widget = check_button_dual_stereo, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Separate Dual Stereo"},
        {.widget = check_button_abc, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Auto bright&contrast"},
        {.widget = label_alpha, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Constrast:"},
        {.widget = label_beta, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Brightness:"},
        {.widget = label_sharpness, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Sharpness:"},
        {.widget = label_edge_low_thres, 
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Edge Low Threshold:"},
    };
    iterate_def_elements(definitions, SIZE(definitions));
}

/** iterate over definition element for grid1 widgets setup */
void init_grid3_def_elements ()
{
    int cur_frame_rate = get_frame_rate(v4l2_dev);
    char fps_buf[10];
    snprintf(fps_buf, sizeof(fps_buf), "%d fps", cur_frame_rate);

    static def_element definitions[] = {
        {.widget = label_ov580_device,
         .wid_type = GTK_WIDGET_TYPE_LABEL,
         .parent = NULL,
         .label_str = "Device: "},
        {.widget = radio_ov580, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_ov580_device,
         .label_str = "OV580"},
        {.widget = radio_sccb0, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_ov580_device,
         .label_str = "SENSOR 0"},
        {.widget = radio_sccb1, 
         .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, 
         .parent = hbox_ov580_device,
         .label_str = "SENSOR 1"},
        
        {.widget = label_ov580_i2c_addr,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Slave Addr:"},
        {.widget = label_ov580_reg_addr,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Reg Addr:"},
        {.widget = label_ov580_reg_val,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Reg Val:"},
        
        {.widget = button_ov580_write,          
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Write"}, 
        {.widget = button_ov580_read,           
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "Read"}, 

        {.widget = entry_ov580_i2c_addr,    
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},
        {.widget = entry_ov580_reg_addr,    
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},
        {.widget = entry_ov580_reg_val,    
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0x"},

        {.widget = check_button_rgb_ir_color, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "RGB-IR ColorCorrectEna"},
        {.widget = check_button_rgb_ir_ir, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Display RGB-IR IR"},

        {.widget = label_resolution,    
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Resolution:"},
        {.widget = label_frame_rate,        
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = "Frame Rate:"},   

        {.widget = combo_box_resolution,    
         .wid_type = GTK_WIDGET_TYPE_COMBO_BOX, 
         .parent = NULL, 
         .label_str = resolutions[0].c_str()},
        {.widget = combo_box_frame_rate,        
         .wid_type = GTK_WIDGET_TYPE_COMBO_BOX, 
         .parent = NULL, 
         .label_str = fps_buf},   

    };

    iterate_def_elements(definitions, SIZE(definitions));

    /// append the remaining resolutions
    int resolution_counts = resolutions.size();
    for (int i=1;i<resolution_counts;i++) {
        gtk_combo_box_text_append_text(
            GTK_COMBO_BOX_TEXT(combo_box_resolution), 
            resolutions[i].c_str());        
    }
    /// append the remaining frame rates
    int cur_fps_size = cur_frame_rates.size();
    //printf("cur_fps_size = %d\r\n",cur_fps_size);
    for(int i=0; i<cur_fps_size; i++) {
        if (cur_frame_rate != cur_frame_rates[i]) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%d fps", cur_frame_rates[i]);
            gtk_combo_box_text_append_text(
                GTK_COMBO_BOX_TEXT(combo_box_frame_rate), 
                buf);     
        }
    }

    /// set the current resolution as active
    int res_index;
    for (res_index=0; res_index< resolution_counts; res_index++) {
        std::vector<std::string> elem = 
			split(resolutions[res_index], 'x');

		int height = stoi(elem[1]);
        if (get_current_height(v4l2_dev) == height) {
            //printf("res index = %d\r\n", res_index);
            gtk_combo_box_set_active(
                GTK_COMBO_BOX(combo_box_resolution),
                res_index);
        }
    }

    
}
    

/** iterate over grid element for GUI layout */
void iterate_grid_elements(
    GtkWidget *grid,
    grid_elements *elements, 
    size_t members)
{
    FOREACH_NELEM(elements, members, ele)
    {
        g_assert(ele->widget != NULL);
        gtk_grid_attach(
            GTK_GRID(grid), 
            ele->widget, 
            ele->col,
            ele->row, 
            ele->width, 
            1);
    }
}

/** grid_elements to hold all grid1 elements layout info */
void list_all_grid1_elements(GtkWidget *grid)
{

    int row;
    int col;
    static grid_elements elements[] = {
        {.widget = label_datatype,           .col =col=0, .row =row=0,   .width =1},
        {.widget = hbox_datatype,            .col =++col, .row =row++, .width =2},
        {.widget = label_bayer,              .col =col=0, .row =row,   .width =1},
        {.widget = hbox_bayer,               .col =++col, .row =row++, .width =2},
        {.widget = check_button_auto_exp,    .col =col=0, .row =row,   .width =1},
        {.widget = check_button_awb,         .col =++col, .row =row,   .width =1},
        {.widget = check_button_clahe,       .col =++col, .row =row++, .width =1},
        {.widget = label_exposure,           .col =col=0, .row =row,   .width =1},
        {.widget = hscale_exposure,          .col =++col, .row =row++, .width =2},
        {.widget = label_gain,               .col =col=0, .row =row,   .width =1},
        {.widget = hscale_gain,              .col =++col, .row =row++, .width =2},
        {.widget = separator_tab1_1,         .col =col=0, .row =row++, .width =3},
        {.widget = label_i2c_addr,           .col =col=0, .row =row,   .width =1},
        {.widget = entry_i2c_addr,           .col =++col, .row =row,   .width =1},
        {.widget = check_button_just_sensor, .col =++col, .row =row++, .width =1},
        {.widget = label_addr_width,         .col =col=0, .row =row,   .width =1},
        {.widget = hbox_addr_width,          .col =++col, .row =row++, .width =1},
        {.widget = label_val_width,          .col =col=0, .row =row,   .width =1},
        {.widget = hbox_val_width,           .col =++col, .row =row++, .width =1},
        {.widget = label_reg_addr,           .col =col=0, .row =row,   .width =1},
        {.widget = entry_reg_addr,           .col =++col, .row =row,   .width =1},
        {.widget = button_read,              .col =++col, .row =row++, .width =1},
        {.widget = label_reg_val,            .col =col=0, .row =row,   .width =1},
        {.widget = entry_reg_val,            .col =++col, .row =row,   .width =1},
        {.widget = button_write,             .col =++col, .row =row++, .width =1},
        {.widget = label_trig,               .col =col=0, .row =row,   .width =1},
        {.widget = check_button_trig_en,     .col =++col, .row =row,   .width =1},
        {.widget = button_trig,              .col =++col, .row =row++, .width =1},
        {.widget = separator_tab1_2,         .col =col=0, .row =row++, .width =3},
        {.widget = label_capture,            .col =col=0, .row =row,   .width =1},
        {.widget = button_capture_bmp,       .col =++col, .row =row,   .width =1},
        {.widget = button_capture_raw,       .col =++col, .row =row++, .width =1},
        {.widget = label_gamma,              .col =col=0, .row =row,   .width =1},
        {.widget = entry_gamma,              .col =++col, .row =row,   .width =1},
        {.widget = button_apply_gamma,       .col =++col, .row =row++, .width =1},
        {.widget = label_blc,                .col =col=0, .row =row,   .width =1},
        {.widget = entry_blc,                .col =++col, .row =row,   .width =1},
        {.widget = button_apply_blc,         .col =++col, .row =row++, .width =1},
    };
    iterate_grid_elements(grid, elements, SIZE(elements));
}
/** grid_elements to hold all grid2 elements layout info */
void list_all_grid2_elements(GtkWidget *grid)
{
    int row;
    int col;
    static grid_elements elements[] = { 
        {.widget = check_button_rgb_gain,    .col =col=0, .row =row=0, .width =2},
        {.widget = button_update_rgb_gain,   .col =col=3, .row =row,   .width =1},
        {.widget = label_red_gain,           .col =col=0, .row =++row, .width =1},
        {.widget = label_green_gain,         .col =col,   .row =++row, .width =1},
        {.widget = label_blue_gain,          .col =col++, .row =++row, .width =1},
        {.widget = entry_red_gain,           .col =col,   .row =row-=2,.width =1},
        {.widget = entry_green_gain,         .col =col,   .row =++row, .width =1},
        {.widget = entry_blue_gain,          .col =col++, .row =++row, .width =1},
        {.widget = label_red_offset,         .col =col,   .row =row=1, .width =1},
        {.widget = label_green_offset,       .col =col,   .row =++row, .width =1},
        {.widget = label_blue_offset,        .col =col++, .row =++row, .width =1},
        {.widget = entry_red_offset,         .col =col,   .row =row-=2,.width =1},
        {.widget = entry_green_offset,       .col =col,   .row =++row, .width =1},
        {.widget = entry_blue_offset,        .col =col++, .row =++row, .width =1},
        {.widget = separator_tab2_1,         .col =col=0, .row =++row, .width =4},

        {.widget = check_button_rgb_matrix,  .col =col=0, .row =++row, .width =2},
        {.widget = button_update_rgb_matrix, .col =col=3, .row =row,   .width =1},
        {.widget = label_red_hor,            .col =col=1, .row =++row, .width =1},
        {.widget = label_green_hor,          .col =++col, .row =row,   .width =1},
        {.widget = label_blue_hor,           .col =++col, .row =row++, .width =1},
        {.widget = label_red_ver,            .col =col=0, .row =row,   .width =1},
        {.widget = label_green_ver,          .col =col,   .row =++row, .width =1},
        {.widget = label_blue_ver,           .col =col++, .row =++row, .width =1},
        {.widget = entry_rr,                 .col =col,   .row =row-=2,.width =1},
        {.widget = entry_rg,                 .col =++col, .row =row,   .width =1},
        {.widget = entry_rb,                 .col =++col, .row =row++, .width =1},
        {.widget = entry_gr,                 .col =col-=2,.row =row,   .width =1},
        {.widget = entry_gg,                 .col =++col, .row =row,   .width =1},
        {.widget = entry_gb,                 .col =++col, .row =row++, .width =1}, 
        {.widget = entry_br,                 .col =col-=2,.row =row,   .width =1},
        {.widget = entry_bg,                 .col =++col, .row =row,   .width =1},
        {.widget = entry_bb,                 .col =++col, .row =row++, .width =1},
        {.widget = separator_tab2_2,         .col =col=0, .row =row++, .width =4},
        {.widget = check_button_soft_ae,     .col =col++, .row =row,   .width =1},
        {.widget = check_button_flip,        .col =col++, .row =row,   .width =1},
        {.widget = check_button_mirror,      .col =col++, .row =row,   .width =1},
        {.widget = check_button_edge,        .col =col++, .row =row++, .width =1},
        {.widget = check_button_histogram,   .col =col=0, .row =row,   .width =1},
        {.widget = check_button_motion_detect,.col =++col, .row =row,   .width =1},
        {.widget = check_button_dual_stereo, .col =++col, .row =row,   .width =1},
        {.widget = check_button_abc,         .col =++col, .row =row++, .width =1},
        {.widget = label_alpha,              .col =col=0, .row =row++, .width =1},
        {.widget = label_beta,               .col =col,   .row =row++, .width =1},
        {.widget = label_sharpness,          .col =col,   .row =row++, .width =1},
        {.widget = label_edge_low_thres,     .col =col++, .row =row++, .width =1},
        {.widget = hscale_alpha,             .col =col,   .row =row-=4,.width =3},
        {.widget = hscale_beta,              .col =col,   .row =++row, .width =3},
        {.widget = hscale_sharpness,         .col =col,   .row =++row, .width =3},
        {.widget = hscale_edge_low_thres,    .col =col,   .row =++row, .width =3},
    };
    iterate_grid_elements(grid, elements, SIZE(elements));
}

/** grid_elements to hold all grid3 elements layout info */
void list_all_grid3_elements(GtkWidget *grid)
{
    int row;
    int col;
    static grid_elements elements[] = { 
        {.widget = label_ov580_device,       .col=col=0, .row=row=0, .width =1},
        {.widget = hbox_ov580_device,        .col=++col, .row=row=0, .width =3},
       
        {.widget = label_ov580_i2c_addr,     .col=col=0, .row=++row, .width =1},
        {.widget = label_ov580_reg_addr,     .col=++col, .row=row,   .width =1},
        {.widget = label_ov580_reg_val,      .col=++col, .row=row,   .width =1},
        {.widget = button_ov580_write,       .col=++col, .row=row,   .width =1},
       
        {.widget = entry_ov580_i2c_addr,     .col=col=0, .row=++row, .width =1},
        {.widget = entry_ov580_reg_addr,     .col=++col, .row=row,   .width =1},
        {.widget = entry_ov580_reg_val,      .col=++col, .row=row,   .width =1},
        {.widget = button_ov580_read,        .col=++col, .row=row,   .width =1},
       
        {.widget = separator_tab3_1,         .col =col=0,.row =++row,.width =4},
       
        {.widget = check_button_rgb_ir_color,.col =col=2,.row =++row,.width =1},
        {.widget = check_button_rgb_ir_ir,   .col =++col,.row =row,  .width =1},
        
        {.widget = label_resolution,         .col =col=0,.row =++row,.width =1},
        {.widget = combo_box_resolution,     .col =++col,.row =row,  .width =1},

        {.widget = label_frame_rate,         .col =++col,.row =row,  .width =1},
        {.widget = combo_box_frame_rate,     .col =++col,.row =row,  .width =1},
    };
    iterate_grid_elements(grid, elements, SIZE(elements));
}

/** iterate over element callbacks */
void iterate_element_cb(element_callback *callbacks, size_t members)
{
    FOREACH_NELEM(callbacks, members, cb) {
        g_assert(cb->widget != NULL);
        g_assert(cb->handler != NULL);
        g_signal_connect (
            cb->widget, 
            cb->signal, 
            cb->handler, 
            cb->data);
    }
}

/** element_callback to hold all grid1 elements callback info */
void list_all_grid1_element_callbacks()
{
    static element_callback callbacks[] = {
        {.widget = radio_raw10,       
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_datatype), 
         .data = (gpointer)"1"},
        {.widget = radio_raw12,       
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_datatype), 
         .data = (gpointer)"2"},
        {.widget = radio_yuyv,        
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_datatype), 
         .data = (gpointer)"3"},
        {.widget = radio_raw8,        
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_datatype), 
         .data = (gpointer)"4"},
        {.widget = radio_mjpeg,        
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_datatype), 
         .data = (gpointer)"5"},

        {.widget = radio_bg,          
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_bayerpattern), 
         .data = (gpointer)"1"},
        {.widget = radio_gb,          
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_bayerpattern), 
         .data = (gpointer)"2"},
        {.widget = radio_rg,          
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_bayerpattern), 
         .data = (gpointer)"3"},
        {.widget = radio_gr,          
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_bayerpattern), 
         .data = (gpointer)"4"},
        {.widget = radio_mono,        
         .signal = "toggled", 
         .handler = G_CALLBACK(radio_bayerpattern), 
         .data = (gpointer)"5"},
 
        {.widget = check_button_auto_exp,      
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_ae),  
         .data = NULL},
        {.widget = check_button_awb,           
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_awb), 
         .data = NULL},
        {.widget = check_button_clahe,     
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_clahe), 
         .data = NULL},
 
        {.widget = hscale_exposure,   
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_exposure_up), 
         .data = NULL},
        {.widget = hscale_gain,       
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_gain_up),     
         .data = NULL},
 
        {.widget = radio_8bit_addr,   
         .signal = "toggled", 
         .handler = G_CALLBACK(toggled_addr_length), 
         .data = (gpointer)"1"},
        {.widget = radio_16bit_addr,  
         .signal = "toggled", 
         .handler = G_CALLBACK(toggled_addr_length), 
         .data = (gpointer)"2"},
        {.widget = radio_8bit_val,    
         .signal = "toggled", 
         .handler = G_CALLBACK(toggled_val_length),  
         .data = (gpointer)"1"},
        {.widget = radio_16bit_val,   
         .signal = "toggled", 
         .handler = G_CALLBACK(toggled_val_length),  
         .data = (gpointer)"2"},
 
        {.widget = button_read,        
         .signal = "clicked", 
         .handler = G_CALLBACK(register_read),       
         .data = NULL},
        {.widget = button_write,       
         .signal = "clicked", 
         .handler = G_CALLBACK(register_write),      
         .data = NULL},
        {.widget = button_capture_bmp, 
         .signal = "clicked", 
         .handler = G_CALLBACK(capture_bmp),         
         .data = NULL},
        {.widget = button_capture_raw, 
         .signal = "clicked", 
         .handler = G_CALLBACK(capture_raw),         
         .data = NULL},
        {.widget = button_apply_gamma, 
         .signal = "clicked", 
         .handler = G_CALLBACK(gamma_correction),    
         .data = NULL},
 
        {.widget = check_button_trig_en,  
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_trig),              
         .data = NULL},
        {.widget = button_trig,           
         .signal = "clicked", 
         .handler = G_CALLBACK(send_trigger),             
         .data = NULL},
        {.widget = button_apply_blc,      
         .signal = "clicked", 
         .handler = G_CALLBACK(black_level_correction),   
         .data = NULL},
    };

    iterate_element_cb(callbacks, SIZE(callbacks));
}
/** element_callback to hold all grid2 elements callback info */
void list_all_grid2_element_callbacks()
{
    static element_callback callbacks[] = {
        {.widget = button_update_rgb_gain, 
         .signal = "clicked", 
         .handler = G_CALLBACK(set_rgb_gain_offset), 
         .data = NULL},
        {.widget = button_update_rgb_matrix, 
         .signal = "clicked", 
         .handler = G_CALLBACK(set_rgb_matrix), 
         .data = NULL},
       {.widget = check_button_soft_ae, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_soft_ae), 
         .data = NULL},
       {.widget = check_button_flip, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_flip), 
         .data = NULL},
        {.widget = check_button_mirror, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_mirror), 
         .data = NULL},
        {.widget = check_button_edge, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_show_edge), 
         .data = NULL},
        {.widget = check_button_histogram, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_histogram), 
         .data = NULL},
        {.widget = check_button_motion_detect, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_motion_detect), 
         .data = NULL},
        {.widget = check_button_dual_stereo, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_display_dual_stereo), 
         .data = NULL},
        {.widget = hscale_alpha, 
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_alpha_up), 
         .data = NULL},
        {.widget = check_button_abc, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_abc), 
         .data = NULL},
        {.widget = hscale_beta, 
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_beta_up), 
         .data = NULL},
        {.widget = hscale_sharpness, 
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_sharpness_up), 
         .data = NULL},
        {.widget = hscale_edge_low_thres, 
         .signal = "value_changed", 
         .handler = G_CALLBACK(hscale_edge_thres_up), 
         .data = NULL},
    };
    iterate_element_cb(callbacks, SIZE(callbacks));
}
/** element_callback to hold all grid3 elements callback info */
void list_all_grid3_element_callbacks()
{
    static element_callback callbacks[] = {
        {.widget = radio_ov580,       
         .signal = "toggled", 
         .handler = G_CALLBACK(update_ov580_dev), 
         .data = (gpointer)"0"},
        {.widget = radio_sccb0,       
         .signal = "toggled", 
         .handler = G_CALLBACK(update_ov580_dev), 
         .data = (gpointer)"1"},
        {.widget = radio_sccb1,        
         .signal = "toggled", 
         .handler = G_CALLBACK(update_ov580_dev), 
         .data = (gpointer)"2"},

        {.widget = button_ov580_write,       
         .signal = "clicked", 
         .handler = G_CALLBACK(ov580_register_write),      
         .data = NULL},
        {.widget = button_ov580_read,        
         .signal = "clicked", 
         .handler = G_CALLBACK(ov580_register_read),       
         .data = NULL},

        {.widget = check_button_rgb_ir_color, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_rgb_ir_color), 
         .data = NULL},
        {.widget = check_button_rgb_ir_ir, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_rgb_ir_ir), 
         .data = NULL},
        {.widget = combo_box_resolution,           
         .signal = "changed", 
         .handler = G_CALLBACK(update_resolution),             
         .data = NULL},
        {.widget = combo_box_frame_rate,           
         .signal = "changed", 
         .handler = G_CALLBACK(update_frame_rate),             
         .data = NULL},
        
    };
    
    iterate_element_cb(callbacks, SIZE(callbacks));

}
/** iterate over windows signals */
void iterate_window_signals(GtkWidget *widget,
                    window_signal *signals, size_t members)
{
    FOREACH_NELEM(signals, members, sig)
    {
        g_signal_connect(
            widget, 
            sig->signal, 
            sig->handler, 
            sig->data);
    }
}
/** signal to hold all window signals info */
void list_all_window_signals(GtkWidget *window)
{
    static window_signal signals[] = {
        {"destroy1", "delete-event", G_CALLBACK(gtk_main_quit), NULL},
        {"destroy2", "key_press_event", G_CALLBACK(check_escape), NULL},
    };
    iterate_window_signals(window, signals, SIZE(signals));
}

/** init the sensitivity of GUI widget */ 
void init_sensitivity()
{

    /** OV580 firmware doesn't have a bunch of fx3 extension unit capabilities */
    if ((strcmp(get_product(), "USB Camera-OV580") == 0) 
    ||(strcmp(get_product(), "USB Cam-OV580-OG01A") == 0)
    ||(strcmp(get_product(), "OV580 STEREO") == 0))
    {
        gtk_widget_set_sensitive(button_read, 0);
        gtk_widget_set_sensitive(button_write, 0);
        gtk_widget_set_sensitive(radio_16bit_addr,0);
        gtk_widget_set_sensitive(radio_16bit_val,0);
        gtk_widget_set_sensitive(radio_8bit_addr,0);
        gtk_widget_set_sensitive(radio_8bit_val,0);
        gtk_widget_set_sensitive(check_button_just_sensor,0);
        gtk_widget_set_sensitive(check_button_trig_en, 0);
        gtk_widget_set_sensitive(button_trig, 0);

    }

    gtk_toggle_button_set_active(
       GTK_TOGGLE_BUTTON(check_button_stream_info), TRUE);  
    gtk_range_set_value(GTK_RANGE(hscale_beta), 0);
    gtk_widget_set_sensitive(hscale_edge_low_thres,0);    

}

/*****************************************************************************
**                      	Main GUI
*****************************************************************************/
/** init GUI, do error checking and name the application */
int gui_init()
{
    if (!gtk_init_check(NULL, NULL))
    {
        fprintf(stderr, "Camera Tool: (GUI) Gtk3 can't open display\n");
        return -1;
    }
    g_set_application_name("Linux Camera Tool");
    return 0;
}

void grid_layout_setup(GtkWidget *grid)
{
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 2);
}

void grid1_setup(GtkWidget *grid)
{
    init_grid1_widgets();
    init_grid1_def_elements ();
    list_all_grid1_elements(grid);
    list_all_grid1_element_callbacks();
    grid_layout_setup(grid);
}

void grid2_setup(GtkWidget *grid)
{
    init_grid2_widgets();
    init_grid2_def_elements ();
    list_all_grid2_elements(grid);
    list_all_grid2_element_callbacks();
    grid_layout_setup(grid);
}

void grid3_setup(GtkWidget *grid)
{
    init_grid3_widgets();
    init_grid3_def_elements();
    list_all_grid3_elements(grid);
    list_all_grid3_element_callbacks();
    grid_layout_setup(grid);

}

void notebook_setup(GtkWidget *maintable)
{
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
    /////////////////////////////////////////////////////
    GtkWidget *icon_image = gtk_image_new_from_icon_name(
        "camera-photo", 
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_show(icon_image);
    
    GtkWidget *label_image = gtk_label_new("General Control");
    gtk_widget_show(label_image);

    GtkWidget *tab1 = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(tab1), icon_image, 0,0,1,1);
    gtk_grid_attach(GTK_GRID(tab1), label_image, 1,0,1,1);

    /////////////////////////////////////////////////////
    GtkWidget *icon_video = gtk_image_new_from_icon_name(
        "video-display", 
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_show(icon_video);

    GtkWidget *label_video = gtk_label_new("ISP Control");
    gtk_widget_show(label_video);

    GtkWidget *tab2 = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(tab2), icon_video, 0,0,1,1);
    gtk_grid_attach(GTK_GRID(tab2), label_video, 1,0,1,1);

    //////////////////////////////////////////////////////
    GtkWidget *icon_ctrl  = gtk_image_new_from_icon_name(
        "media-flash", 
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_show(icon_ctrl);

    GtkWidget *label_flash = gtk_label_new("Flash Control");
    gtk_widget_show(label_flash);

    GtkWidget *tab3 = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(tab3), icon_ctrl, 0,0,1,1);
    gtk_grid_attach(GTK_GRID(tab3), label_flash, 1,0,1,1);

    /////////////////////////////////////////////////////
    GtkWidget *grid1 = gtk_grid_new();
    GtkWidget *grid2 = gtk_grid_new();
    GtkWidget *grid3 = gtk_grid_new();

    grid1_setup(grid1);
    grid2_setup(grid2);
    grid3_setup(grid3);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid1, tab1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, tab2);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid3, tab3);
    init_sensitivity();
    gtk_container_add(GTK_CONTAINER(maintable), notebook);

}

std::string convert_bpp_to_string(int bpp)
{
    switch(bpp)
    {
        case 16: 
            return "YUYV";
        case 8: 
            return "RAW8";
        case 12:    
            return "RAW12";
        case 10:    
            return "RAW10";
        default: 
            return "RAW10";
    }
}

/**
 * menu bar setup
 * the menu bar info will get update depending on the device found
 */
void menu_bar_setup(GtkWidget *maintable)
{

    GtkWidget *menu_bar = gtk_menu_bar_new();
    gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menu_bar), 
        GTK_PACK_DIRECTION_LTR); 
    gtk_widget_set_hexpand(menu_bar, TRUE);
    gtk_widget_set_halign(menu_bar, GTK_ALIGN_FILL);
    /** init all menus */
    GtkWidget *device_menu = gtk_menu_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *fw_update_menu = gtk_menu_new();
    GtkWidget *help_menu = gtk_menu_new();

    /** device info items */
    GtkWidget *device_item = 
        gtk_menu_item_new_with_label("Devices Info");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(device_item), device_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), device_item);
    
    std::vector<std::string> device_info;
    device_info.push_back("Device: " 
        + std::string(get_dev_name()));
    device_info.push_back("Manufacturer: " 
        + std::string(get_manufacturer_name()));
    device_info.push_back("Product: " 
        + std::string(get_product()));
    
    if (!is_ov580_stereo())
    {
        device_info.push_back("Firmware Rev: " 
            + std::to_string(get_fw_rev()));
        char buf[10];
        snprintf(buf, sizeof(buf), "0%x", get_hw_rev());
        device_info.push_back("Hardware Rev: " 
            + std::string(buf));
        device_info.push_back("Datatype: " 
            + convert_bpp_to_string(set_bpp(get_li_datatype())));
    }
    else 
    {
	    int bpp;
        if ((strcmp(get_product(), "USB Camera-OV580") == 0) 
	    || (strcmp(get_product(), "USB Cam-OV580-OG01A") == 0))
            bpp = RAW8_FLG;
        else if (strcmp(get_product(), "OV580 STEREO") == 0)
            bpp = YUYV_FLG;
        device_info.push_back("Datatype: " 
            + convert_bpp_to_string(bpp));
    }

    for (size_t i=0; i < device_info.size(); i++) {
        device_item = 
            gtk_menu_item_new_with_label(device_info[i].c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(device_menu), device_item);
    }

    if (!is_ov580_stereo())
    {
        /** config file items */
        GtkWidget *config_file_item = 
            gtk_menu_item_new_with_label("Config File");
        gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(config_file_item), 
            file_menu);
        gtk_menu_shell_append(
            GTK_MENU_SHELL(menu_bar), 
            config_file_item);

        config_file_item = 
            gtk_menu_item_new_with_label("Load json");
        menu_item_formater(
            config_file_item, 
            file_menu, 
            G_CALLBACK(open_config_dialog));

        config_file_item = 
            gtk_menu_item_new_with_label("Load BatchCmd.txt");
        menu_item_formater(
            config_file_item, 
            file_menu, 
            G_CALLBACK(open_config_dialog));

        config_file_item = 
            gtk_menu_item_new_with_label("Program Flash");
        menu_item_formater(
            config_file_item, 
            file_menu, 
            G_CALLBACK(open_config_dialog));

        config_file_item = 
            gtk_menu_item_new_with_label("Program EEPROM");
        menu_item_formater(
            config_file_item, 
            file_menu, 
            G_CALLBACK(open_config_dialog));
        // gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
        // g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), window);

        /** firmware update items */
        GtkWidget *fw_update_item = 
            gtk_menu_item_new_with_label("FW Update");
        gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(fw_update_item), 
            fw_update_menu);
        gtk_menu_shell_append(
            GTK_MENU_SHELL(menu_bar), 
            fw_update_item);

        fw_update_item = 
            gtk_menu_item_new_with_label("Reset Camera");
        menu_item_formater(
            fw_update_item,
            fw_update_menu, 
            G_CALLBACK(fw_update_clicked));

        fw_update_item = 
            gtk_menu_item_new_with_label("Erase Firmware");
        menu_item_formater(
            fw_update_item, 
            fw_update_menu, 
            G_CALLBACK(fw_update_clicked));
    }

    /** help items */
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(
        GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(
        GTK_MENU_SHELL(menu_bar), help_item);

    help_item = gtk_menu_item_new_with_label("About");
    menu_item_formater(
        help_item, 
        help_menu, 
        G_CALLBACK(about_info));

    help_item = gtk_menu_item_new_with_label("Exit");
    menu_item_formater(
        help_item, 
        help_menu, 
        G_CALLBACK(exit_loop));

    gtk_container_add(GTK_CONTAINER(maintable), menu_bar);
}
/**
 * Display the built info(built from OpenCV CUDA) from status bar
 */
void statusbar_setup(GtkWidget *maintable)
{
    GtkWidget* status_bar = gtk_statusbar_new();
    
    #ifdef HAVE_OPENCV_CUDA_SUPPORT
        gtk_statusbar_push(
            GTK_STATUSBAR (status_bar), 
            0, 
            (gchar *)"Build Status: OpenCV CUDA Support");

    #else
        gtk_statusbar_push(
            GTK_STATUSBAR (status_bar), 
            0, 
            (gchar *)"Build Status: OpenCV non-CUDA Support");
    #endif
    gtk_widget_set_name(status_bar, "myStatusBar");

    gtk_widget_set_margin_top(GTK_WIDGET(status_bar), 0);
    gtk_widget_set_margin_bottom(GTK_WIDGET(status_bar), 0);
    gtk_container_add(GTK_CONTAINER(maintable), status_bar);
}

void universal_ctrl_setup(GtkWidget *maintable)
{
    GtkWidget *top_grid = gtk_grid_new();

    check_button_stream_info    = gtk_check_button_new();
    check_button_resize_window  = gtk_check_button_new();
    button_exit_streaming       = gtk_button_new();

    separator_top_tab           = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    static def_element definitions[] = {
        {.widget = check_button_resize_window,
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON,
         .parent = NULL,
         .label_str = "Resize Window"},
        {.widget = check_button_stream_info, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Display Stream Info"},
        {.widget = button_exit_streaming, 
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "EXIT"},
    };
    iterate_def_elements(definitions, SIZE(definitions));

    int row;
    int col;
    static grid_elements elements[] = {
        {.widget = check_button_resize_window,.col=col=0,   .row=row=0,  .width =1},
        {.widget = check_button_stream_info,  .col = 1,     .row =row,   .width =1},
        {.widget = button_exit_streaming,     .col = 2,     .row =row++, .width =1},
        {.widget = separator_top_tab,         .col = 0,     .row =row++, .width =3},
    };
    iterate_grid_elements(top_grid, elements, SIZE(elements));

    static element_callback callbacks[] = {
        {.widget = check_button_resize_window,
         .signal = "toggled",
         .handler = G_CALLBACK(enable_resize_window),
         .data = NULL},
        {.widget = check_button_stream_info, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_display_mat_info), 
         .data = NULL},
        {.widget = button_exit_streaming, 
         .signal = "clicked", 
         .handler = G_CALLBACK(exit_loop), 
         .data = NULL},
    };
    iterate_element_cb(callbacks, SIZE(callbacks));


    gtk_container_add(GTK_CONTAINER(maintable), top_grid);
    gtk_grid_set_column_homogeneous (GTK_GRID(top_grid), TRUE);
    gtk_widget_set_hexpand (top_grid, TRUE);
    gtk_widget_set_halign (top_grid, GTK_ALIGN_FILL);
    gtk_widget_set_margin_top(GTK_WIDGET(top_grid), 0);
    gtk_widget_set_margin_bottom(GTK_WIDGET(top_grid), 0);

}
/** set up css for GUI look */
void css_setup()
{
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(
        screen, 
        GTK_STYLE_PROVIDER(provider), 
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    if (g_file_test("../gui_style.css", G_FILE_TEST_EXISTS)) 
    {    
        gtk_css_provider_load_from_file(
            provider, 
            g_file_new_for_path("../gui_style.css"), 
            NULL);
    }
    else
    {        
        gtk_css_provider_load_from_file(
            provider, 
            g_file_new_for_path("./gui_style.css"), 
            NULL);
    }
    g_object_unref(provider);
}

/** 
 * main function for GUI, put everything before gtk_main() 
 * set up menu bar, notebook/grids and status bar
 */
void gui_run()
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Control");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(window, 500, 100);
    //gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_UTILITY);
    const char* icon1path = "./pic/leopard_cam_tool.jpg";  
    const char* icon2path = "../pic/leopard_cam_tool.jpg"; 

    /// whether use Makefile or cmake
    if (g_file_test(icon1path, G_FILE_TEST_EXISTS)) 
        gtk_window_set_default_icon_from_file(icon1path, NULL);
    else 
        gtk_window_set_default_icon_from_file(icon2path, NULL);
    
    GtkWidget *maintable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    menu_bar_setup(maintable);
    universal_ctrl_setup(maintable);
    notebook_setup(maintable);
    statusbar_setup(maintable);
 
    gtk_container_add(GTK_CONTAINER(window), maintable);
    
    list_all_window_signals(window);
    gtk_widget_show_all(window);

    gtk_main();
}

/** 
 * fail
 * g++ ui_control.cpp -o test2 `gtk-config --cflags --libs`
 * pass
 * g++ ui_control.cpp -o test `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
 * 
 * int init_control_gui(int argc, char *argv[])
 */
void ctrl_gui_main(int socket)
//void ctrl_gui_main()
{
    v4l2_dev = recv_fd(socket);
    gui_init();
    css_setup();
    gui_run();
}