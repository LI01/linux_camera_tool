/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for the camera tool
  control GUI using Gtk3. Gtk3 and Gtk2 don't live together peaceful. If you 
  have problem running Gtk3 with your current compiled openCV, please refer to
  README.md guide to rebuild your opencv for supporting Gtk3.

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#include "../includes/shortcuts.h"
#include "../includes/ui_control.h"
#include "../includes/v4l2_devices.h"
#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/cam_property.h"
#include "../includes/json_parser.h"
/*****************************************************************************
**                      	Global data 
*****************************************************************************/
GtkWidget *label_device, *label_hw_rev, *label_fw_rev, *button_exit_streaming;
GtkWidget *label_datatype, *hbox_datatype;
GtkWidget *radio_raw10, *radio_raw12, *radio_yuyv, *radio_raw8;

GtkWidget *label_bayer, *hbox_bayer;
GtkWidget *radio_bg, *radio_gb, *radio_rg, *radio_gr, *radio_mono;

GtkWidget *check_button_auto_exposure, *check_button_awb, *check_button_auto_gain;
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
GtkWidget *grid;
GtkWidget *maintable;
GtkWidget *menu_bar, *menu_item, *file_menu, *help_menu, *help_item;


int address_width_flag;
int value_width_flag;

extern int v4l2_dev;
extern int fw_rev;

/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/
//TODO:
static void menu_response (GtkWidget *menu_item, gpointer data)
{
    if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "New") == 0 )
    {
        g_print("You pressed new");
    }
    if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "Exit") == 0 )
    {
        gtk_main_quit();
    }
    if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "About") == 0 )
    {
        g_print("You pressed about");
    }
}
/** callback for sensor datatype updates*/
void radio_datatype(GtkWidget *widget, gpointer data)
{
    (void)widget;
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
    g_print("exposure is %d lines\n", exposure_time);
}

/** callback for updating analog gain */
void hscale_gain_up(GtkRange *widget)
{
    int gain;
    gain = (int)gtk_range_get_value(widget);
    set_gain(v4l2_dev, gain);
    g_print("gain is %d\n", gain);
}

/** callback for enabling/disabling auto exposure */
void enable_ae(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
        set_exposure_auto(v4l2_dev, 0);
    else
        set_exposure_auto(v4l2_dev, 1);
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

/** callback for enabling/disabling auto brightness and contrast optimization */
void enable_abc(GtkWidget *widget, GtkToggleButton *toggle_button)
{
    (void)widget;
    if (gtk_toggle_button_get_active(toggle_button))
    {
        g_print("auto brightness and contrast optimization enable\n");
        abc_enable(1);
    }
    else
    {
        g_print("auto brightness and contrast optimization disable\n");
        abc_enable(0);
    }
}

/** callback for updating register address length 8/16 bits */
void toggled_addr_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    if (strcmp((char *)data, "1") == 0)
        address_width_flag = 1;

    if (strcmp((char *)data, "2") == 0)
        address_width_flag = 2;
}

/** callback for updating register value length 8/16 bits */
void toggled_val_length(GtkWidget *widget, gpointer data)
{
    (void)widget;
    if (strcmp((char *)data, "1") == 0)
        value_width_flag = 1;

    if (strcmp((char *)data, "2") == 0)
        value_width_flag = 2;
}

/** generate register address width flag for callback in register read/write */
int addr_width_for_rw(int address_width_flag)
{
    if (address_width_flag == 1)
        return 1;
    if (address_width_flag == 2)
        return 2;
    return 1;
}

/** generate register value width flag for callback in register read/write */
int val_width_for_rw(int value_width_flag)
{
    if (value_width_flag == 1)
        return 1;
    if (value_width_flag == 2)
        return 2;
    return 1;
}

/** callback for register write */
void register_write(GtkWidget *widget)
{
    (void)widget;

    /** sensor read */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
                             NULL, 16);
        int regVal = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_val)),
                            NULL, 16);
        sensor_reg_write(v4l2_dev, regAddr, regVal);
    }

    /** generic i2c slave read */
    else
    {
        int slaveAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)),
                               NULL, 16);
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
                             NULL, 16);
        int regVal = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_val)),
                            NULL, 16);
        unsigned char buf[2];
        buf[0] = regVal & 0xff;
        buf[1] = (regVal >> 8) & 0xff;

        /** write 8/16 bit register addr and register data */
        if (addr_width_for_rw(address_width_flag) == 1 &&
            (val_width_for_rw(value_width_flag) == 1))
            generic_I2C_write(v4l2_dev, 0x81, 1, slaveAddr, regAddr, buf);
        else if (addr_width_for_rw(address_width_flag) == 2 &&
                 (val_width_for_rw(value_width_flag) == 1))
            generic_I2C_write(v4l2_dev, 0x82, 1, slaveAddr, regAddr, buf);
        else if (addr_width_for_rw(address_width_flag) == 1 &&
                 (val_width_for_rw(value_width_flag) == 2))
            generic_I2C_write(v4l2_dev, 0x81, 2, slaveAddr, regAddr, buf);
        else
            generic_I2C_write(v4l2_dev, 0x82, 2, slaveAddr, regAddr, buf);
    }
}

/** callback for register read */
void register_read(GtkWidget *widget)
{
    (void)widget;
    /** sensor read */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor)))
    {
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
                             NULL, 16);

        int regVal = sensor_reg_read(v4l2_dev, regAddr);
        char buf[6];
        snprintf(buf, sizeof(buf), "0x%x", regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }

    /** generic i2c slave read */
    else
    {
        int slaveAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)),
                               NULL, 16);
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
                             NULL, 16);
        int regVal = generic_I2C_read(v4l2_dev, 0x02, 1, slaveAddr, regAddr);
        char buf[6];
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
    add_gamma_val(gamma);
    g_print("gamma = %f\n", gamma);
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
    int black_level = atof((char *)gtk_entry_get_text(GTK_ENTRY(entry_blc)));
    add_black_level_correction(black_level);
    g_print("black level correction = %d\n", black_level);
}

/** exit streaming loop */
void exit_loop(GtkWidget *widget)
{
    (void)widget;
    set_loop(0);
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
**                      	GUI Layout Setup, DON'T CHANGE
*****************************************************************************/
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

void list_all_def_elements ()
{
    char device_buf[64];
    snprintf(device_buf, sizeof(device_buf), "Device: %s - %s",
             get_manufacturer_name(), get_product());

    char fw_rev_buf[20];
    snprintf(fw_rev_buf, sizeof(fw_rev_buf), "Firmware Rev: %d",
             fw_rev);

    static def_element definitions[] = {
        {label_device,      GTK_WIDGET_TYPE_LABEL, NULL, device_buf},
        {label_fw_rev,      GTK_WIDGET_TYPE_LABEL, NULL, fw_rev_buf},
        {label_exposure,    GTK_WIDGET_TYPE_LABEL, NULL, "Exposure:"},
        {label_gain,        GTK_WIDGET_TYPE_LABEL, NULL, "Gain:"},   
        {label_i2c_addr,    GTK_WIDGET_TYPE_LABEL, NULL, "I2C Addr:"},
        {label_datatype,    GTK_WIDGET_TYPE_LABEL, NULL, "Sensor Datatype:"},
        {label_bayer,       GTK_WIDGET_TYPE_LABEL, NULL, "Raw Camera Pixel Format:"},
        {label_addr_width,  GTK_WIDGET_TYPE_LABEL, NULL, "Register Addr Width:"},
        {label_val_width,   GTK_WIDGET_TYPE_LABEL, NULL, "Register Value Width:"},
        {label_reg_addr,    GTK_WIDGET_TYPE_LABEL, NULL, "Reg Addr:"},     
        {label_reg_val,     GTK_WIDGET_TYPE_LABEL, NULL, "Reg Value:"},
        {label_trig,        GTK_WIDGET_TYPE_LABEL, NULL, "Trigger Sensor:"},
        {label_capture,     GTK_WIDGET_TYPE_LABEL, NULL, "Capture:"},     
        {label_gamma,       GTK_WIDGET_TYPE_LABEL, NULL, "Gamma Correction:"},
        {label_blc,         GTK_WIDGET_TYPE_LABEL, NULL, "Black Level Correction:"},


        {button_exit_streaming, GTK_WIDGET_TYPE_BUTTON, NULL, "EXIT"},
        {button_write,          GTK_WIDGET_TYPE_BUTTON, NULL, "Write"}, 
        {button_read,           GTK_WIDGET_TYPE_BUTTON, NULL, "Read"}, 
        {button_capture_bmp,    GTK_WIDGET_TYPE_BUTTON, NULL, "Capture bmp"},  
        {button_capture_raw,    GTK_WIDGET_TYPE_BUTTON, NULL, "Capture raw"},  
        {button_trig,           GTK_WIDGET_TYPE_BUTTON, NULL, "Shot 1 Trigger"},
        {button_apply_gamma ,   GTK_WIDGET_TYPE_BUTTON, NULL, "Apply"},   
        {button_apply_blc,      GTK_WIDGET_TYPE_BUTTON, NULL, "Apply"},

        {check_button_auto_exposure, GTK_WIDGET_TYPE_CHECK_BUTTON, NULL, "Enable auto exposure"},
        {check_button_awb ,          GTK_WIDGET_TYPE_CHECK_BUTTON, NULL, "Enable auto white balance"},
        {check_button_auto_gain ,    GTK_WIDGET_TYPE_CHECK_BUTTON, NULL, "Enable auto brightness&contrast"},        
        {check_button_just_sensor,   GTK_WIDGET_TYPE_CHECK_BUTTON, NULL, "Just sensor read/write"},
        {check_button_trig_en,       GTK_WIDGET_TYPE_CHECK_BUTTON, NULL, "Enable"},

        {hscale_exposure,   GTK_WIDGET_TYPE_HSCALE, NULL, "2505"},
        {hscale_gain,       GTK_WIDGET_TYPE_HSCALE, NULL, "64"},
        
        {entry_i2c_addr,    GTK_WIDGET_TYPE_ENTRY, NULL, "0x"},
        {entry_reg_addr,    GTK_WIDGET_TYPE_ENTRY, NULL, "0x"},
        {entry_reg_val,     GTK_WIDGET_TYPE_ENTRY, NULL, "0x"},    
        {entry_gamma,       GTK_WIDGET_TYPE_ENTRY, NULL, "1"},
        {entry_blc,         GTK_WIDGET_TYPE_ENTRY, NULL, "0"},

        {radio_raw10, GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_datatype, "RAW10"},
        {radio_raw12, GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_datatype, "RAW12"},
        {radio_yuyv,  GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_datatype, "YUYV"},
        {radio_raw8,  GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_datatype, "RAW8"},
        
        {radio_bg,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_bayer, "BGGR"},
        {radio_gb,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_bayer, "GBBR"},
        {radio_rg,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_bayer, "RGGB"},
        {radio_gr,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_bayer, "GRBG"},
        {radio_mono, GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_bayer, "MONO"},

        {radio_8bit_addr,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_addr_width, "8-bit"},
        {radio_16bit_addr,  GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_addr_width, "16-bit"},

        {radio_8bit_val,   GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_val_width, "8-bit"},
        {radio_16bit_val,  GTK_WIDGET_TYPE_RADIO_BUTTON, hbox_val_width,"16-bit"},

    };
    iterate_def_elements(definitions, SIZE(definitions));
}


/** iterate over grid element for GUI layout */
void iterate_grid_elements(
    grid_elements *elements, size_t members)
{
    FOREACH_NELEM(elements, members, ele)
    {
        g_assert(ele->widget != NULL);
        gtk_grid_attach(GTK_GRID(grid), ele->widget, ele->col,
                        ele->row, ele->width, 1);
    }
}



/** grid_elements to hold all grid elements layout info */
void list_all_grid_elements()
{
    int row;
    int col;
    static grid_elements elements[] = {
        {label_device,          col = 0,    row = 0,1},
        {label_fw_rev,          ++col,      row,    1},
        {button_exit_streaming, ++col,      row++,  1},

        {label_datatype,        col = 0,    row,    1},
        {hbox_datatype,         ++col,      row++,  2},

        {label_bayer,           col = 0,    row,    1},
        {hbox_bayer,            ++col,      row++,  2},

        {check_button_auto_exposure,col = 0,    row,    1},
        {check_button_awb,          ++col,      row,    1},
        {check_button_auto_gain,    ++col,      row++,  1},

        {label_exposure,       col = 0,    row,    1},
        {hscale_exposure,      ++col,      row++,  3},

        {label_gain,           col = 0,    row,    1},
        {hscale_gain,          ++col,      row++,  3},

        {label_i2c_addr,            col = 0,    row,    1},
        {entry_i2c_addr,            ++col,      row,    1},
        {check_button_just_sensor,  ++col,      row++,  1},

        {label_addr_width,     col = 0,    row,    1},
        {hbox_addr_width,      ++col,      row++,  1},

        {label_val_width,      col = 0,    row,    1},
        {hbox_val_width,       ++col,      row++,  1},

        {label_reg_addr,       col = 0,    row,    1},
        {entry_reg_addr,       ++col,      row,    1},
        {button_read,          ++col,      row++,  1},

        {label_reg_val,        col = 0,    row,    1},
        {entry_reg_val,        ++col,      row,    1},
        {button_write,         ++col,      row++,  1},

        {label_capture,        col = 0,    row,    1},
        {button_capture_bmp,   ++col,      row,    1},
        {button_capture_raw,   ++col,      row++,  1},

        {label_gamma,          col = 0,    row,    1},
        {entry_gamma,          ++col,      row,    1},
        {button_apply_gamma,   ++col,      row++,  1},

        {label_trig,           col = 0,    row,    1},
        {check_button_trig_en, ++col,      row,    1},
        {button_trig,          ++col,      row++,  1},

        {label_blc,            col = 0,    row,    1},
        {entry_blc,            ++col,      row,    1},
        {button_apply_blc,     ++col,      row++,  1},

    };
    iterate_grid_elements(elements, SIZE(elements));
}

/** iterate over element callbacks */
void iterate_element_cb(element_callback *callbacks, size_t members)
{
    FOREACH_NELEM(callbacks, members, cb) {
        g_signal_connect (cb->widget, cb->signal, cb->handler, cb->data);
    }
}

/** element_callback to hold all elements callback info */
void list_all_element_callbacks()
{
    static element_callback callbacks[] = {
        {button_exit_streaming, "clicked", G_CALLBACK(exit_loop), NULL},

        {radio_raw10,       "toggled", G_CALLBACK(radio_datatype), (gpointer)"1"},
        {radio_raw12,       "toggled", G_CALLBACK(radio_datatype), (gpointer)"2"},
        {radio_yuyv,        "toggled", G_CALLBACK(radio_datatype), (gpointer)"3"},
        {radio_raw8,        "toggled", G_CALLBACK(radio_datatype), (gpointer)"4"},

        {radio_bg,          "toggled", G_CALLBACK(radio_bayerpattern), (gpointer)"1"},
        {radio_gb,          "toggled", G_CALLBACK(radio_bayerpattern), (gpointer)"2"},
        {radio_rg,          "toggled", G_CALLBACK(radio_bayerpattern), (gpointer)"3"},
        {radio_gr,          "toggled", G_CALLBACK(radio_bayerpattern), (gpointer)"4"},
        {radio_mono,        "toggled", G_CALLBACK(radio_bayerpattern), (gpointer)"5"},

        {check_button_auto_exposure, "toggled", G_CALLBACK(enable_ae), NULL},
        {check_button_awb,           "toggled", G_CALLBACK(enable_awb), NULL},
        {check_button_auto_gain,     "toggled", G_CALLBACK(enable_abc), NULL},

        {hscale_exposure,   "value_changed", G_CALLBACK(hscale_exposure_up), NULL},
        {hscale_gain,       "value_changed", G_CALLBACK(hscale_gain_up), NULL},

        {radio_8bit_addr,   "toggled", G_CALLBACK(toggled_addr_length), (gpointer)"1"},
        {radio_16bit_addr,  "toggled", G_CALLBACK(toggled_addr_length), (gpointer)"2"},

        {radio_8bit_val,    "toggled", G_CALLBACK(toggled_val_length), (gpointer)"1"},
        {radio_16bit_val,   "toggled", G_CALLBACK(toggled_val_length), (gpointer)"2"},

        {button_read,   "clicked", G_CALLBACK(register_read), NULL},
        {button_write,  "clicked", G_CALLBACK(register_write), NULL},

        {button_capture_bmp, "clicked", G_CALLBACK(capture_bmp), NULL},
        {button_capture_raw, "clicked", G_CALLBACK(capture_raw), NULL},

        {button_apply_gamma, "clicked", G_CALLBACK(gamma_correction), NULL},

        {check_button_trig_en,  "toggled", G_CALLBACK(enable_trig), NULL},
        {button_trig,           "clicked", G_CALLBACK(send_trigger), NULL},
        
        {button_apply_blc,      "clicked", G_CALLBACK(black_level_correction), NULL},

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

/** init all the widgets here, sequence doesn't matter
 * FIXME: callback in the iterate function so that we don't need to do this here?
 */
void init_all_widgets()
{
      
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

    check_button_auto_exposure  = gtk_check_button_new();
    check_button_awb            = gtk_check_button_new();
    check_button_auto_gain      = gtk_check_button_new();    
    check_button_trig_en        = gtk_check_button_new();
    check_button_just_sensor    = gtk_check_button_new();

    entry_i2c_addr  = gtk_entry_new();
    entry_reg_addr  = gtk_entry_new();
    entry_reg_val   = gtk_entry_new();
    entry_gamma     = gtk_entry_new();
    entry_blc       = gtk_entry_new();


    hscale_exposure = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                               0.0, 2505.0, 1.0);
    hscale_gain     = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                           0.0, 63.0, 1.0);

    hbox_val_width  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_addr_width = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_bayer      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    hbox_datatype   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    radio_raw10 = gtk_radio_button_new(NULL);
    radio_raw12 = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_raw10)));
    radio_yuyv  = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_raw10)));
    radio_raw8  = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_raw10)));

    radio_bg    = gtk_radio_button_new(NULL);
    radio_gb    = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)));
    radio_rg    = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)));
    radio_gr    = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)));
    radio_mono  = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)));

    radio_8bit_addr    = gtk_radio_button_new(NULL);
    radio_16bit_addr   = gtk_radio_button_new(gtk_radio_button_get_group
                (GTK_RADIO_BUTTON(radio_8bit_addr)));

    radio_8bit_val     = gtk_radio_button_new(NULL);
    radio_16bit_val    = gtk_radio_button_new(gtk_radio_button_get_group
                (GTK_RADIO_BUTTON(radio_8bit_val)));

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

void grid_setup()
{
    grid = gtk_grid_new();

    /** --- Grid Layout, DON'T CHANGE --- */
    init_all_widgets();
    list_all_def_elements ();
    list_all_grid_elements();
    list_all_element_callbacks();

    /** --- Grid Setup --- */
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 2);
    gtk_container_add(GTK_CONTAINER(maintable), grid);
}

void menu_bar_setup()
{
    menu_bar = gtk_menu_bar_new();
    file_menu = gtk_menu_new();
    help_menu = gtk_menu_new();

    menu_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);


    help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    g_signal_connect(help_item, "activate", G_CALLBACK(menu_response), NULL);

    menu_item = gtk_menu_item_new_with_label("New");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(menu_response), NULL);

    menu_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);

    menu_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), menu_item);


    gtk_container_add(GTK_CONTAINER(maintable), menu_bar);
}
void gui_run()
{
    static GtkWidget *window = NULL; /** the main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Control");
    gtk_widget_set_size_request(window, 500, 100);

    char icon1path[100] = "./pic/leopard_cam_tool.jpg"; /* window icon */
    if (g_file_test(icon1path, G_FILE_TEST_EXISTS))
        gtk_window_set_icon_from_file(GTK_WINDOW(window), icon1path, NULL);

    maintable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    menu_bar_setup();
    grid_setup();
    gtk_container_add(GTK_CONTAINER(window), maintable);


    list_all_window_signals(window);
    gtk_widget_show_all(window);
    gtk_main();
}

//fail
//g++ ui_control.cpp -o test2 `gtk-config --cflags --libs`
//pass
//g++ ui_control.cpp -o test `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
//int init_control_gui(int argc, char *argv[])
void ctrl_gui_main()
{
    gui_init();
    gui_run();
}