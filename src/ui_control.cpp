/*****************************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc., 
  
  This is the sample code for Leopard USB3.0 camera, mainly for the camera tool
  control GUI using Gtk3. Gtk3 and Gtk2 don't live together peaceful. If you 
  have problem running Gtk3 with your current compiled openCV, please refer to
  README.md guide to rebuild your OpenCV for supporting Gtk3.

  Author: Danyu L
  Last edit: 2019/08
*****************************************************************************/

#include "../includes/shortcuts.h"
#include "../includes/ui_control.h"
#include "../includes/v4l2_devices.h"
#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/cam_property.h"
#include "../includes/json_parser.h"
#include "../includes/core_io.h"

/*****************************************************************************
**                      	Global data 
*****************************************************************************/
static GtkWidget *window = NULL;
/// grid1 elements
GtkWidget *label_device, *label_fw_rev, *button_exit_streaming;
GtkWidget *label_datatype, *hbox_datatype;
GtkWidget *radio_raw10, *radio_raw12, *radio_yuyv, *radio_raw8;
GtkWidget *label_bayer, *hbox_bayer;
GtkWidget *radio_bg, *radio_gb, *radio_rg, *radio_gr, *radio_mono;
GtkWidget *check_button_auto_exp, *check_button_awb, *check_button_clahe;
GtkWidget *label_exposure, *label_gain;
GtkWidget *hscale_exposure, *hscale_gain;
GtkWidget *label_i2c_addr, *entry_i2c_addr;
GtkWidget *label_addr_width, *hbox_addr_width, *radio_8bit_addr, *radio_16bit_addr;
GtkWidget *label_val_width, *hbox_val_width, *radio_8bit_val, *radio_16bit_val;
GtkWidget *label_reg_addr, *entry_reg_addr;
GtkWidget *label_reg_val, *entry_reg_val;
GtkWidget *button_read, *button_write;
GtkWidget *check_button_just_sensor;
GtkWidget *label_capture, *button_capture_bmp, *button_capture_raw;
GtkWidget *label_gamma, *entry_gamma, *button_apply_gamma;
GtkWidget *label_trig, *check_button_trig_en, *button_trig;
GtkWidget *label_blc, *entry_blc, *button_apply_blc;
GtkWidget *seperator_tab1_1, *seperator_tab1_2, *seperator_tab1_3;
/// grid2 elements
GtkWidget *check_button_rgb_gain, *button_update_rgb_gain;
GtkWidget *label_red_gain, *label_green_gain, *label_blue_gain;
GtkWidget *entry_red_gain, *entry_green_gain, *entry_blue_gain;
GtkWidget *label_red_offset, *label_green_offset, *label_blue_offset;
GtkWidget *entry_red_offset, *entry_green_offset, *entry_blue_offset;
GtkWidget *seperator_tab2_1, *seperator_tab2_2;
GtkWidget *check_button_rgb_matrix, *button_update_rgb_matrix;
GtkWidget *label_red_hor, *label_green_hor, *label_blue_hor;
GtkWidget *label_red_ver, *label_green_ver, *label_blue_ver;
GtkWidget *entry_red_red, *entry_red_green, *entry_red_blue; 
GtkWidget *entry_green_red, *entry_green_green, *entry_green_blue; 
GtkWidget *entry_blue_red, *entry_blue_green, *entry_blue_blue; 
GtkWidget *check_button_soft_ae , *check_button_flip;
GtkWidget *check_button_edge, *check_button_mirror;
GtkWidget *check_button_rgb_ir_color, *check_button_rgb_ir_ir;
GtkWidget *check_button_dual_stereo, *check_button_stream_info;
GtkWidget *label_alpha, *label_beta, *label_sharpness;
GtkWidget *check_button_abc;
GtkWidget *hscale_alpha, *hscale_beta, *hscale_sharpness;
GtkWidget *label_edge_low_thres, *hscale_edge_low_thres;
/// grid3 elements
GtkWidget *check_button_resize_window;

static int address_width_flag;
static int value_width_flag;

extern int v4l2_dev;/** global variable, file descriptor for camera device */

char icon1path[30] = "./pic/leopard_cam_tool.jpg"; /** window icon for makefile */
char icon2path[30] = "../pic/leopard_cam_tool.jpg"; /** window icon for cmake */
char version_number[10] = "v0.4.3";
/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
/** callback for enabling/disabling auto white balance */
void enable_resize_window(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("resize window enable\n");
        resize_window_enable(1);
    }
    else
    {
        g_print("resize window disable\n");
        resize_window_enable(0);
    }
}
/**-------------------------menu bar callbacks------------------------------*/
/** callback for find and load config files */
void open_config_dialog(GtkWidget *widget, gpointer window)
{
    (void)widget;
    GtkWidget *file_dialog = gtk_file_chooser_dialog_new("Load Config File",
    GTK_WINDOW(window),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    ("_Open"),
    GTK_RESPONSE_OK,
    ("_Cancel"),
    GTK_RESPONSE_CANCEL,
    NULL);
    gtk_window_set_transient_for(GTK_WINDOW(file_dialog), GTK_WINDOW(window));
    gtk_widget_show(file_dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(file_dialog));
    //gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_dialog), "/");
    // gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_dialog), 
    // g_get_home_dir());

    if (resp == GTK_RESPONSE_OK) 
    {
        char *file_name = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(file_dialog));

        if ((strcmp(get_file_extension(file_name), "json") == 0) | \
            (strcmp(get_file_extension(file_name), "bin") == 0) | \
            (strcmp(get_file_extension(file_name), "txt") == 0))
        {
            //g_print("Load %s\r\n", get_file_basename(file_name));
            load_control_profile(v4l2_dev, file_name);
        }
        else
            g_print("Choose another type of file\r\n");

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
void about_info(GtkWidget *widget)
{

    (void)widget;
    GtkWidget *dialog_about = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog_about), 
        "Linux Camera Tool");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog_about), 
        version_number);
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
    /// disable a bunch of function if it is YUV camera
    if (strcmp((char *)data, "3") == 0)
    {
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

/** callback for bayer pattern choice updates*/
void radio_bayerpattern(GtkWidget *widget, gpointer data)
{
    (void)widget;
    change_bayerpattern(data);
}

/** callback for updating exposure time line */
void hscale_exposure_up(GtkRange *widget)
{
    int exposure_time;
    exposure_time = (int)gtk_range_get_value(widget);
    set_exposure_absolute(v4l2_dev, exposure_time);
    g_print("exposure is %d lines\n",get_exposure_absolute(v4l2_dev));
}

/** callback for updating analog gain */
void hscale_gain_up(GtkRange *widget)
{
    int gain;
    gain = (int)gtk_range_get_value(widget);
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

/** callback for enabling/disabling auto white balance */
void enable_awb(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("awb enable\n");
        awb_enable(1);
    }
    else
    {
        g_print("awb disable\n");
        awb_enable(0);
    }
}

/** callback for enabling/disabling CLAHE optimization */
void enable_clahe(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("contrast limited adaptive histogram equalization enable\n");
        clahe_enable(1);
    }
    else
    {
        g_print("contrast limited adaptive histogram equalization disable\n");
        clahe_enable(0);
    }
}

/** callback for updating register address length 8/16 bits */
void toggled_addr_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    if (strcmp((char *)data, "1") == 0)
        address_width_flag = _8BIT_FLG;

    if (strcmp((char *)data, "2") == 0)
        address_width_flag = _16BIT_FLG;
}

/** callback for updating register value length 8/16 bits */
void toggled_val_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    if (strcmp((char *)data, "1") == 0)
        value_width_flag = _8BIT_FLG;

    if (strcmp((char *)data, "2") == 0)
        value_width_flag = _16BIT_FLG;
}



/** callback for register write */
void register_write(GtkWidget *widget)
{
    (void)widget;

    /** sensor read */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        gtk_widget_set_sensitive(entry_i2c_addr,0);
        int regAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)));
        int regVal = hex_or_dec_interpreter_c_string((char *)  \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_val)));
        sensor_reg_write(v4l2_dev, regAddr, regVal);
    }

    /** generic i2c slave read */
    else
    {
        gtk_widget_set_sensitive(entry_i2c_addr,1);
        int slaveAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)));
        int regAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)));
        int regVal = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_val)));
        unsigned char buf[2];
        buf[0] = regVal & 0xff;
        buf[1] = (regVal >> 8) & 0xff;

        /** write 8/16 bit register addr and register data */
        generic_I2C_write(v4l2_dev, (GENERIC_REG_WRITE_FLG | \
            addr_width_for_rw(address_width_flag)),         
            val_width_for_rw(value_width_flag),             
            slaveAddr, regAddr, buf);
    }
}

/** callback for register read */
void register_read(GtkWidget *widget)
{
    (void)widget;
    /** sensor read */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        int regAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)));

        int regVal = sensor_reg_read(v4l2_dev, regAddr);
        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }

    /** generic i2c slave read */
    else
    {
        int slaveAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)));
        int regAddr = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)));
        int regVal;
        
        regVal = generic_I2C_read(v4l2_dev, GENERIC_REG_READ_FLG | 
                addr_width_for_rw(address_width_flag), 
                val_width_for_rw(value_width_flag), 
                slaveAddr, regAddr);

        char buf[10];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }
}

/** callback for capturing bmp */
void capture_bmp(GtkWidget *widget)
{
    (void)widget;
    video_capture_save_bmp();
}

/** callback for capturing raw */
void capture_raw(GtkWidget *widget)
{
    (void)widget;
    video_capture_save_raw();
}

/** callback for gamma correction */
void gamma_correction(GtkWidget *widget)
{
    (void)widget;
    float gamma = atof((char *)gtk_entry_get_text(GTK_ENTRY(entry_gamma)));
    if (isgreater(gamma, 0.0)) 
    {
        add_gamma_val(gamma);
        g_print("gamma = %f\n", gamma);
    }
    else
        g_print("gamma should be greater than 0, you entered gamma = %f\n", gamma);
}

/** callback for triggering sensor functionality */
void send_trigger(GtkWidget *widget)
{
    (void)widget;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_trig_en)))
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
void enable_trig(GtkWidget *widget)
{
    (void)widget;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_trig_en)))
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
void black_level_correction(GtkWidget *widget)
{
    (void)widget;
    float float_blc;
    float_blc = atof((char *)gtk_entry_get_text(GTK_ENTRY(entry_blc)));
    if (floor(float_blc) == ceil(float_blc))
    {
        add_blc((int)float_blc);
        g_print("Apply black level correction = %d\n", (int)float_blc);
    }
    else
    {
         g_print("Please enter an integer for each pixel black level correction\r\n");
    }
    
}

/**-------------------------grid2 callbacks-------------------------------*/
/** 
 * callback for update rgb gain
 * limit the input on MAX/MIN_RGB macros
 */
void set_rgb_gain_offset(GtkWidget *widget)
{
    (void) widget;
    int red_gain = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_red_gain)));
    red_gain = set_limit(red_gain, MAX_RGB_GAIN, MIN_RGB_GAIN);

    int green_gain = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_green_gain)));
    green_gain = set_limit(green_gain, MAX_RGB_GAIN, MIN_RGB_GAIN);
    
    int blue_gain = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_blue_gain)));
    blue_gain = set_limit(blue_gain, MAX_RGB_GAIN, MIN_RGB_GAIN);
    
    int red_offset = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_red_offset)));
    red_offset = set_limit(red_offset, MAX_RGB_OFFSET, MIN_RGB_OFFSET);

    int green_offset = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_green_offset)));
    green_offset = set_limit(green_offset, MAX_RGB_OFFSET, MIN_RGB_OFFSET);

    int blue_offset = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_blue_offset)));
    blue_offset = set_limit(blue_offset, MAX_RGB_OFFSET, MIN_RGB_OFFSET);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_rgb_gain))) {
        enable_rgb_gain_offset (red_gain, green_gain, blue_gain, red_offset, 
        green_offset, blue_offset);
    }
    else
    {
        disable_rgb_gain_offset(); 
        g_print("set rgb gain offset disabled\r\n");
    }
    

}
/**
 *  callback for update rgb2rgb color matrix
 *  limit the input on MAX/MIN_RGB macros
 * 
 *  http://www.imatest.com/docs/colormatrix/
 */
void set_rgb_matrix(GtkWidget *widget)
{
    (void) widget;
    int red_red = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_red_red)));
    red_red = set_limit (red_red, 512, -512);

    int red_green = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_red_green)));
    red_green = set_limit (red_green, 512, -512);

    int red_blue = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_red_blue)));
    red_blue = set_limit (red_blue, 512, -512);

    int green_red = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_green_red)));
    green_red = set_limit (green_red, 512, -512);

    int green_green = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_green_green)));
    green_green = set_limit (green_green, 512, -512);

    int green_blue = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_green_blue)));
    green_blue = set_limit (green_blue, 512, -512);

    int blue_red = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_blue_red)));
    blue_red = set_limit (blue_red, 512, -512);

    int blue_green = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_blue_green)));
    blue_green = set_limit (blue_green, 512, -512);

    int blue_blue = hex_or_dec_interpreter_c_string((char *) \
            gtk_entry_get_text(GTK_ENTRY(entry_blue_blue)));
    blue_blue = set_limit (blue_blue, 512, -512);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_rgb_matrix))) {
        enable_rgb_matrix (red_red, red_green, red_blue, 
        green_red, green_green, green_blue,
        blue_red, blue_green, blue_blue);
    }
    else
    {
        disable_rgb_matrix(); 
        g_print("set rgb matrix disabled\r\n");        
    }
    

}

/** callback for enabling/disabling auto white balance */
void enable_soft_ae(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("software ae enable\n");
        soft_ae_enable(1);
        gtk_widget_set_sensitive(hscale_exposure, 0); //disable change for exposure
        gtk_widget_set_sensitive(hscale_gain,0);//disable change for gain
    }
    else
    {
        g_print("software ae disable\n");
        soft_ae_enable(0);
        gtk_widget_set_sensitive(hscale_exposure, 1); //enable change for exposure
        gtk_widget_set_sensitive(hscale_gain,1);    //enable change for gain
    }
}
/** callback for enabling/disabling image flip */
void enable_flip(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("flip enable\n");
        flip_enable(1);
    }
    else
    {
        g_print("flip disable\n");
        flip_enable(0);
    }
}
/** callback for enabling/disabling image mirror */
void enable_mirror(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("mirror enable\n");
        mirror_enable(1);
    }
    else
    {
        g_print("mirror disable\n");
        mirror_enable(0);
    }
}
/** callback for enabling/disabling show image edge */
void enable_show_edge(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("show canny edge enable\n");
        canny_filter_enable(1);
        gtk_widget_set_sensitive(hscale_edge_low_thres,1);
    }
    else
    {
        g_print("show canny edge disable\n");
        canny_filter_enable(0);
        gtk_widget_set_sensitive(hscale_edge_low_thres,0);
    }
}
/** 
 * callback for enabling/disabling displaying RGB IR color
 * channel after color correction
 */
void enable_rgb_ir_color(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("RGB-IR color correction enable\n");
        rgb_ir_correction_enable(1);
    }
    else
    {
        g_print("RGB-IR color correction disable\n");
        rgb_ir_correction_enable(0);
    }
}
/** allback for enabling/disabling displaying RGB IR IR channel */
void enable_rgb_ir_ir(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("RGB-IR IR display enable\n");
        rgb_ir_ir_display_enable(1);
    }
    else
    {
        g_print("RGB-IR IR display disable\n");
        rgb_ir_ir_display_enable(0);
    }
}

/** callback for enabling/disabling separate show dual stereo cam */
void enable_display_dual_stereo(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("Separate display dual stereo cam enable\n");
        separate_dual_enable(1);
    }
    else
    {
        g_print("Separate display dual stereo cam disable\n");
        separate_dual_enable(0);
    }
}
/** callback for enabling/disabling display camera info */
void enable_display_mat_info(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("stream info display enable\n");
        display_info_enable(1);
    }
    else
    {
        g_print("stream info display disable\n");
        display_info_enable(0);
    }
}
/** callback for enabling/disabling auto white balance */
void enable_abc(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("auto brightness and contrast enable\n");
        abc_enable(1);
    }
    else
    {
        g_print("auto brightness and contrast disable\n");
        abc_enable(0);
    }
}

/** callback for update alpha value */
void hscale_alpha_up(GtkRange *widget)
{
    int alpha = (int)gtk_range_get_value(widget);
    add_alpha_val(alpha);
}
/** callback for update beta value */
void hscale_beta_up(GtkRange *widget)
{
    int beta = (int)gtk_range_get_value(widget);
    add_beta_val(beta);
}
/** callback for update sharpness value */
void hscale_sharpness_up(GtkRange *widget)
{
    int sharpness = (int)gtk_range_get_value(widget);
    add_sharpness_val(sharpness);
}
/** callback for update edge threashold value */
void hscale_edge_thres_up(GtkRange *widget)
{
    int edge_low_thres = (int)gtk_range_get_value(widget);
    add_edge_thres_val(edge_low_thres);
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
gboolean check_escape(GtkWidget *widget, GdkEventKey *event)
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
**                      	Helper functions
*****************************************************************************/
/** generate register address width flag for callback in register read/write */
int addr_width_for_rw(int address_width_flag)
{
    if (address_width_flag == _8BIT_FLG)
        return 1;
    if (address_width_flag == _16BIT_FLG)
        return 2;
    return 1;
}

/** generate register value width flag for callback in register read/write */
int val_width_for_rw(int value_width_flag)
{
    if (value_width_flag == _8BIT_FLG)
        return 1;
    if (value_width_flag == _16BIT_FLG)
        return 2;
    return 1;
}
/** interpret c string for both hex and decimal */
int hex_or_dec_interpreter_c_string(char *in_string)
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
	 * this part is to get the gain and exposure maximum value defined in firmware
	 * and update the range in GUI
	 * if MAX_EXPOSURE_TIME is undefined in firmware, will put a upper limit for 
	 * exposure time range so the range won't be as large as 65535 which confuses people
	 * 
	 *  if UNSET_MAX_EXPOSURE_LINE, firmware didn't define MAX_EXPOSURE_TIME...
	 * 	place holder = 3, it should be enough for most cases
	 *  if you adjust exposure to maximum, and image is still dark 
	 *  try to increase 3 to 4, 5, 6 etc
	 *  or ask for a firmware update, firmware exposure time mapping might be wrong
	 */
    int exposure_max = query_exposure_absolute_max(v4l2_dev);
    int gain_max = query_gain_max(v4l2_dev);
    if (exposure_max == UNDEFINED_MAX_EXPOSURE_LINE)
        exposure_max = get_current_height(v4l2_dev) * 3;

    label_device   = gtk_label_new(NULL);
    label_fw_rev   = gtk_label_new(NULL);
    label_datatype = gtk_label_new(NULL);
    label_bayer    = gtk_label_new(NULL);
    label_exposure = gtk_label_new(NULL);
    label_gain     = gtk_label_new(NULL);  
    label_i2c_addr = gtk_label_new(NULL);
    label_addr_width = gtk_label_new(NULL);
    label_val_width  = gtk_label_new(NULL);
    label_reg_addr = gtk_label_new(NULL);
    label_reg_val  = gtk_label_new(NULL);
    label_capture  = gtk_label_new(NULL);  
    label_gamma    = gtk_label_new(NULL);
    label_trig     = gtk_label_new(NULL);
    label_blc      = gtk_label_new(NULL);

    button_exit_streaming   = gtk_button_new();
    button_read             = gtk_button_new();
    button_write            = gtk_button_new();
    button_apply_gamma      = gtk_button_new();
    button_trig             = gtk_button_new();
    button_capture_bmp      = gtk_button_new();
    button_capture_raw      = gtk_button_new();
    button_apply_blc        = gtk_button_new();

    check_button_auto_exp  = gtk_check_button_new();
    check_button_awb            = gtk_check_button_new();
    check_button_clahe      = gtk_check_button_new();    
    check_button_trig_en        = gtk_check_button_new();
    check_button_just_sensor    = gtk_check_button_new();

    entry_i2c_addr  = gtk_entry_new();
    entry_reg_addr  = gtk_entry_new();
    entry_reg_val   = gtk_entry_new();
    entry_gamma     = gtk_entry_new();
    entry_blc       = gtk_entry_new();

    hscale_exposure = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                               0.0, (double)exposure_max, 1.0);
    hscale_gain     = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                           0.0, (double)gain_max, 1.0);

    hbox_val_width  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_addr_width = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_bayer      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_datatype   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    radio_raw10 = gtk_radio_button_new(NULL);
    radio_raw12 = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_raw10)));
    radio_yuyv  = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_raw10)));
    radio_raw8  = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_raw10)));

    radio_bg    = gtk_radio_button_new(NULL);
    radio_gb    = gtk_radio_button_new(gtk_radio_button_get_group   
        (GTK_RADIO_BUTTON(radio_bg)));
    radio_rg    = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_bg)));
    radio_gr    = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_bg)));
    radio_mono  = gtk_radio_button_new(gtk_radio_button_get_group
        (GTK_RADIO_BUTTON(radio_bg)));

    radio_8bit_addr    = gtk_radio_button_new(NULL);
    radio_16bit_addr   = gtk_radio_button_new(gtk_radio_button_get_group
                (GTK_RADIO_BUTTON(radio_8bit_addr)));

    radio_8bit_val     = gtk_radio_button_new(NULL);
    radio_16bit_val    = gtk_radio_button_new(gtk_radio_button_get_group
                (GTK_RADIO_BUTTON(radio_8bit_val)));

    seperator_tab1_1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    seperator_tab1_2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    seperator_tab1_3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
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
    check_button_rgb_ir_color   = gtk_check_button_new();
    check_button_rgb_ir_ir      = gtk_check_button_new();
    check_button_dual_stereo    = gtk_check_button_new();
    check_button_stream_info    = gtk_check_button_new();
    check_button_abc            = gtk_check_button_new();
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(check_button_stream_info), TRUE);
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
    entry_red_red               = gtk_entry_new();
    entry_red_green             = gtk_entry_new();
    entry_red_blue              = gtk_entry_new();
    entry_green_red             = gtk_entry_new();
    entry_green_green           = gtk_entry_new();
    entry_green_blue            = gtk_entry_new();
    entry_blue_red              = gtk_entry_new();
    entry_blue_green            = gtk_entry_new();
    entry_blue_blue             = gtk_entry_new();

    seperator_tab2_1           = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    seperator_tab2_2           = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    
    hscale_alpha                = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL, 1.0, (double)ALPHA_MAX, 1.0);
    hscale_beta = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL, 0.0, (double)BETA_MAX, 1.0);
    hscale_sharpness = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL, 1.0, (double)SHARPNESS_MAX, 1.0);
    hscale_edge_low_thres = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL, 0.0, (double)LOW_THRESHOLD_MAX, 1.0);  
    gtk_widget_set_sensitive(hscale_edge_low_thres,0);                                              
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
                gtk_label_set_text(GTK_LABEL(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_BUTTON:
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_VBOX:
                break;
            case GTK_WIDGET_TYPE_RADIO_BUTTON:  
                gtk_box_pack_start(GTK_BOX(def->parent), def->widget, 0, 0, 0);
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_CHECK_BUTTON:
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_ENTRY:
                gtk_entry_set_text(GTK_ENTRY(def->widget), def->label_str);
                gtk_entry_set_width_chars(GTK_ENTRY(def->widget), -1);
                break;
            case GTK_WIDGET_TYPE_HSCALE:
                gtk_widget_set_hexpand(def->widget, TRUE);
                break;
            default:
                break;

       } 
    }
}
/** iterate over definition element for grid1 widgets setup */
void init_grid1_def_elements ()
{
    char device_buf[64];
    snprintf(device_buf, sizeof(device_buf), "Device: %s - %s",
             get_manufacturer_name(), get_product());

    char fw_rev_buf[20];
    snprintf(fw_rev_buf, sizeof(fw_rev_buf), "Firmware Rev: %d",
             get_fw_rev());

    static def_element definitions[] = {
        {.widget = label_device,      
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = device_buf},
        {.widget = label_fw_rev,      
         .wid_type = GTK_WIDGET_TYPE_LABEL, 
         .parent = NULL, 
         .label_str = fw_rev_buf},
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

        {.widget = button_exit_streaming, 
         .wid_type = GTK_WIDGET_TYPE_BUTTON, 
         .parent = NULL, 
         .label_str = "EXIT"},
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
        {.widget = entry_red_red, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "256"},
        {.widget = entry_red_green, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_red_blue, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_green_red, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_green_green, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "256"},
        {.widget = entry_green_blue, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_blue_red, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},
        {.widget = entry_blue_green, 
         .wid_type = GTK_WIDGET_TYPE_ENTRY, 
         .parent = NULL, 
         .label_str = "0"},        
        {.widget = entry_blue_blue, 
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
        {.widget = check_button_rgb_ir_color, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "RGB-IR ColorCorrectEna"},
        {.widget = check_button_rgb_ir_ir, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Display RGB-IR IR"},
        {.widget = check_button_dual_stereo, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Separate Dual Stereo"},
        {.widget = check_button_stream_info, 
         .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, 
         .parent = NULL, 
         .label_str = "Display Stream Info"},
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

/** iterate over grid element for GUI layout */
void iterate_grid_elements(
    GtkWidget *grid,
    grid_elements *elements, 
    size_t members)
{
    FOREACH_NELEM(elements, members, ele)
    {
        g_assert(ele->widget != NULL);
        gtk_grid_attach(GTK_GRID(grid), ele->widget, ele->col,
                        ele->row, ele->width, 1);
    }
}

/** grid_elements to hold all grid1 elements layout info */
void list_all_grid1_elements(GtkWidget *grid)
{

    int row;
    int col;
    grid_elements elements[] = {
        {.widget = label_device,             .col =col=0, .row =row=0, .width =1},
        {.widget = label_fw_rev,             .col =++col, .row =row,   .width =1},
        {.widget = button_exit_streaming,    .col =++col, .row =row++, .width =1},
        {.widget = seperator_tab1_1,         .col =col=0, .row =row++, .width =2},
        {.widget = label_datatype,           .col =col=0, .row =row,   .width =1},
        {.widget = hbox_datatype,            .col =++col, .row =row++, .width =2},
        {.widget = label_bayer,              .col =col=0, .row =row,   .width =1},
        {.widget = hbox_bayer,               .col =++col, .row =row++, .width =2},
        {.widget = check_button_auto_exp,    .col =col=0, .row =row,   .width =1},
        {.widget = check_button_awb,         .col =++col, .row =row,   .width =1},
        {.widget = check_button_clahe,   .col =++col, .row =row++, .width =1},
        {.widget = label_exposure,           .col =col=0, .row =row,   .width =1},
        {.widget = hscale_exposure,          .col =++col, .row =row++, .width =3},
        {.widget = label_gain,               .col =col=0, .row =row,   .width =1},
        {.widget = hscale_gain,              .col =++col, .row =row++, .width =3},
        {.widget = seperator_tab1_2,         .col =col=0, .row =row++, .width =3},
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
        {.widget = seperator_tab1_3,         .col =col=0, .row =row++, .width =3},
        {.widget = label_capture,            .col =col=0, .row =row,   .width =1},
        {.widget = button_capture_bmp,       .col =++col, .row =row,   .width =1},
        {.widget = button_capture_raw,       .col =++col, .row =row++, .width =1},
        {.widget = label_gamma,              .col =col=0, .row =row,   .width =1},
        {.widget = entry_gamma,              .col =++col, .row =row,   .width =1},
        {.widget = button_apply_gamma,       .col =++col, .row =row++, .width =1},
        {.widget = label_trig,               .col =col=0, .row =row,   .width =1},
        {.widget = check_button_trig_en,     .col =++col, .row =row,   .width =1},
        {.widget = button_trig,              .col =++col, .row =row++, .width =1},
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
    grid_elements elements[] = { 
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
        {.widget = seperator_tab2_1,         .col =col=0, .row =++row, .width =4},

        {.widget = check_button_rgb_matrix,  .col =col=0, .row =++row, .width =2},
        {.widget = button_update_rgb_matrix, .col =col=3, .row =row,   .width =1},
        {.widget = label_red_hor,            .col =col=1, .row =++row, .width =1},
        {.widget = label_green_hor,          .col =++col, .row =row,   .width =1},
        {.widget = label_blue_hor,           .col =++col, .row =row++, .width =1},
        {.widget = label_red_ver,            .col =col=0, .row =row,   .width =1},
        {.widget = label_green_ver,          .col =col,   .row =++row, .width =1},
        {.widget = label_blue_ver,           .col =col++, .row =++row, .width =1},
        {.widget = entry_red_red,            .col =col,   .row =row-=2,.width =1},
        {.widget = entry_red_green,          .col =++col, .row =row,   .width =1},
        {.widget = entry_red_blue,           .col =++col, .row =row++, .width =1},
        {.widget = entry_green_red,          .col =col-=2,.row =row,   .width =1},
        {.widget = entry_green_green,        .col =++col, .row =row,   .width =1},
        {.widget = entry_green_blue,         .col =++col, .row =row++, .width =1}, 
        {.widget = entry_blue_red,           .col =col-=2,.row =row,   .width =1},
        {.widget = entry_blue_green,         .col =++col, .row =row,   .width =1},
        {.widget = entry_blue_blue,          .col =++col, .row =row++, .width =1},
        {.widget = seperator_tab2_2,         .col =col=0, .row =row++, .width =4},
        {.widget = check_button_soft_ae,     .col =col++, .row =row,   .width =1},
        {.widget = check_button_flip,        .col =col++, .row =row,   .width =1},
        {.widget = check_button_mirror,      .col =col++, .row =row,   .width =1},
        {.widget = check_button_edge,        .col =col++, .row =row++, .width =1},
        {.widget = check_button_rgb_ir_color,.col =col=0, .row =row,   .width =1},
        {.widget = check_button_rgb_ir_ir,   .col =++col, .row =row,   .width =1},
        {.widget = check_button_dual_stereo, .col =++col, .row =row,   .width =1},
        {.widget = check_button_stream_info, .col =++col, .row =row++, .width =1},
        {.widget = label_alpha,              .col =col=0, .row =row++, .width =1},
        {.widget = label_beta,               .col =col,   .row =row++, .width =1},
        {.widget = label_sharpness,          .col =col,   .row =row++, .width =1},
        {.widget = label_edge_low_thres,     .col =col++, .row =row++, .width =1},
        {.widget = hscale_alpha,             .col =col,   .row =row-=4,.width =2},
        {.widget = check_button_abc,         .col =col+=2,.row =row,   .width =1},
        {.widget = hscale_beta,              .col =col-=2,.row =++row, .width =3},
        {.widget = hscale_sharpness,         .col =col,   .row =++row, .width =3},
        {.widget = hscale_edge_low_thres,    .col =col,   .row =++row, .width =3},
    };
    iterate_grid_elements(grid, elements, SIZE(elements));
}

/** iterate over element callbacks */
void iterate_element_cb(element_callback *callbacks, size_t members)
{
    FOREACH_NELEM(callbacks, members, cb) {
        g_assert(cb->widget != NULL);
        g_assert(cb->handler != NULL);
        g_signal_connect (cb->widget, cb->signal, cb->handler, cb->data);
    }
}
/** element_callback to hold all grid1 elements callback info */
void list_all_grid1_element_callbacks()
{
    static element_callback callbacks[] = {
        {.widget = button_exit_streaming, 
         .signal = "clicked", 
         .handler = G_CALLBACK(exit_loop), 
         .data = NULL},
 
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
        {.widget = check_button_rgb_ir_color, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_rgb_ir_color), 
         .data = NULL},
        {.widget = check_button_rgb_ir_ir, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_rgb_ir_ir), 
         .data = NULL},
        {.widget = check_button_dual_stereo, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_display_dual_stereo), 
         .data = NULL},
        {.widget = check_button_stream_info, 
         .signal = "toggled", 
         .handler = G_CALLBACK(enable_display_mat_info), 
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

/** iterate over windows signals */
void iterate_window_signals(GtkWidget *widget,
                    window_signal *signals, size_t members)
{
    FOREACH_NELEM(signals, members, sig)
    {
        g_signal_connect(widget, sig->signal, sig->handler, sig->data);
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


/*****************************************************************************
**                      	Main GUI
*****************************************************************************/

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
    check_button_resize_window  = gtk_check_button_new();
    gtk_button_set_label(GTK_BUTTON(check_button_resize_window), "resize_window");
    gtk_grid_attach(GTK_GRID(grid), check_button_resize_window, 0, 0, 1, 1);
    g_signal_connect (check_button_resize_window, "toggled", G_CALLBACK(enable_resize_window), NULL);
}
void notebook_setup(GtkWidget *maintable)
{
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
 
    GtkWidget *tab1 = gtk_label_new("tab1");
    GtkWidget *tab2 = gtk_label_new("tab2");
    GtkWidget *tab3 = gtk_label_new("tab3");

    GtkWidget *grid1 = gtk_grid_new();
    GtkWidget *grid2 = gtk_grid_new();
    GtkWidget *grid3 = gtk_grid_new();

    grid1_setup(grid1);
    grid2_setup(grid2);
    grid3_setup(grid3);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid1, tab1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, tab2);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid3, tab3);
    gtk_container_add(GTK_CONTAINER(maintable), notebook);

}

void menu_bar_setup(GtkWidget *maintable)
{

    GtkWidget *menu_bar = gtk_menu_bar_new();
    gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menu_bar), 
        GTK_PACK_DIRECTION_LTR); 
    
    /** init all menus */
    GtkWidget *device_menu = gtk_menu_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *fw_update_menu = gtk_menu_new();
    GtkWidget *help_menu = gtk_menu_new();

    /** device items */
    GtkWidget *device_item = gtk_menu_item_new_with_label("Devices");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(device_item), device_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), device_item);


    /** config file items */
    GtkWidget *config_file_item = gtk_menu_item_new_with_label("Config File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config_file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), config_file_item);

    config_file_item = gtk_menu_item_new_with_label("Load json");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), window);

    config_file_item = gtk_menu_item_new_with_label("Load BatchCmd.txt");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), window);

    config_file_item = gtk_menu_item_new_with_label("Program Flash");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), window);
  
    config_file_item = gtk_menu_item_new_with_label("Program EEPROM");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), window);
    

    /** firmware update items */
    GtkWidget *fw_update_item = gtk_menu_item_new_with_label("FW Update");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fw_update_item), fw_update_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), fw_update_item);

    fw_update_item = gtk_menu_item_new_with_label("Reset Camera");
    gtk_menu_shell_append(GTK_MENU_SHELL(fw_update_menu), fw_update_item);
    g_signal_connect(fw_update_item, "activate", G_CALLBACK(fw_update_clicked), NULL);

    fw_update_item = gtk_menu_item_new_with_label("Erase Firmware");
    gtk_menu_shell_append(GTK_MENU_SHELL(fw_update_menu), fw_update_item);
    g_signal_connect(fw_update_item, "activate", G_CALLBACK(fw_update_clicked), NULL);

    /** help items */
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);

    help_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_item);
    g_signal_connect(help_item, "activate", G_CALLBACK(about_info), NULL);

    help_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_item);
    g_signal_connect(help_item, "activate", G_CALLBACK(exit_loop), NULL);

    gtk_container_add(GTK_CONTAINER(maintable), menu_bar);
}

void gui_run()
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Control");
    gtk_widget_set_size_request(window, 500, 100);

    /// whether use makefile or cmake
    if (g_file_test(icon1path, G_FILE_TEST_EXISTS)) 
        gtk_window_set_default_icon_from_file(icon1path, NULL);
    else 
        gtk_window_set_default_icon_from_file(icon2path, NULL);
    
    GtkWidget *maintable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    menu_bar_setup(maintable);
    notebook_setup(maintable);
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
void ctrl_gui_main()
{
    gui_init();
    gui_run();
}