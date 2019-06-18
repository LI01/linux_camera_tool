/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for the camera tool
  control GUI using Gtk3. Gtk3 and Gtk2 don't live together peaceful. If you 
  have problem running Gtk3 with your current compiled openCV, please refer to
  README.md guide to rebuild your opencv for supporting Gtk3.

  Author: Danyu L
  Last edit: 2019/06
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
GtkWidget *menu_bar, *config_file_item, *file_menu, *help_menu, *help_item;
GtkWidget *device_menu, *device_item;
GtkWidget *fw_update_item, *fw_update_menu;
GtkWidget *file_dialog;
//GtkWiget *progress_fw_update, *progress_flash_eeprom;

int address_width_flag;
int value_width_flag;
static GtkWidget *window = NULL; /** the main window */
extern int v4l2_dev;
extern int fw_rev;

/*****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/

void open_config_dialog(GtkWidget *widget, gpointer window)
{
    (void)widget;
    file_dialog = gtk_file_chooser_dialog_new("Load Config File",
    GTK_WINDOW(window),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    ("_Open"),
    GTK_RESPONSE_OK,
    ("_Cancel"),
    GTK_RESPONSE_CANCEL,
    NULL);
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


void fw_update_clicked (GtkWidget *item)
{
    if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(item)), "Erase") == 0 )
    {
        /// close the camera first before erase 
        //FIXME: camera tool exit??
        set_loop(0);
        firmware_erase(v4l2_dev);
        //gtk_main_quit();
        
    }
    else if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(item)), "Reset Camera") == 0 )
    {
        reboot_camera(v4l2_dev);
    }
    else if (strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(item)), "FW Update") == 0 )
    {
        g_print("You pressed fw update. I haven't finished the code yet\r\n");
        //TODO:
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
        generic_I2C_write(v4l2_dev, (GENERIC_REG_WRITE_FLG | 
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
        int regVal;

        regVal = generic_I2C_read(v4l2_dev, GENERIC_REG_READ_FLG | 
                addr_width_for_rw(address_width_flag), 
                val_width_for_rw(value_width_flag), 
                slaveAddr, regAddr);

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
int count;
/*****************************************************************************
**                      	GUI Layout Setup, DON'T CHANGE
*****************************************************************************/
void iterate_def_elements (
    def_element *definitions, size_t members)
{   
    count = 0;
    FOREACH_NELEM(definitions, members, def)
    {

        switch (def->wid_type) 
        {
            case GTK_WIDGET_TYPE_LABEL:
                //def->widget = gtk_label_new(NULL);
                //printf("widget %d is %x\r\n", count++, def->widget);
                gtk_label_set_text(GTK_LABEL(def->widget), def->label_str);
               break;
            case GTK_WIDGET_TYPE_BUTTON:
                //def->widget = gtk_button_new();
               // printf("widget %d is %x\r\n", count++, def->widget);
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_VBOX:
                //def->widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
            
                //printf("widget %d is %x\r\n", count++, def->widget);
                break;
            case GTK_WIDGET_TYPE_RADIO_BUTTON:
                //printf("widget %d is %x\r\n", count++, def->widget);
                gtk_box_pack_start(GTK_BOX(def->parent), def->widget, 0, 0, 0);
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_CHECK_BUTTON:
                //def->widget =  gtk_check_button_new();
                //printf("widget %d is %x\r\n", count++, def->widget);
                gtk_button_set_label(GTK_BUTTON(def->widget), def->label_str);
                break;
            case GTK_WIDGET_TYPE_ENTRY:
                //def->widget =  gtk_entry_new();
                //printf("widget %d is %x\r\n", count++, def->widget);
                gtk_entry_set_text(GTK_ENTRY(def->widget), def->label_str);
                gtk_entry_set_width_chars(GTK_ENTRY(def->widget), -1);
                break;
            case GTK_WIDGET_TYPE_HSCALE:
                //printf("widget %d is %x\r\n", count++, def->widget);
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
        {.widget = label_device,      .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = device_buf},
        {.widget = label_fw_rev,      .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = fw_rev_buf},
        {.widget = label_exposure,    .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Exposure:"},
        {.widget = label_gain,        .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Gain:"},   
        {.widget = label_i2c_addr,    .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "I2C Addr:"},
        {.widget = label_datatype,    .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Sensor Datatype:"},
        {.widget = label_bayer,       .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Raw Camera Pixel Format:"},
        {.widget = label_addr_width,  .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Register Addr Width:"},
        {.widget = label_val_width,   .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Register Value Width:"},
        {.widget = label_reg_addr,    .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Reg Addr:"},     
        {.widget = label_reg_val,     .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Reg Value:"},
        {.widget = label_trig,        .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Trigger Sensor:"},
        {.widget = label_capture,     .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Capture:"},     
        {.widget = label_gamma,       .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Gamma Correction:"},
        {.widget = label_blc,         .wid_type = GTK_WIDGET_TYPE_LABEL, .parent = NULL, .label_str = "Black Level Correction:"},

        {.widget = button_exit_streaming, .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "EXIT"},
        {.widget = button_write,          .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Write"}, 
        {.widget = button_read,           .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Read"}, 
        {.widget = button_capture_bmp,    .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Capture bmp"},  
        {.widget = button_capture_raw,    .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Capture raw"},  
        {.widget = button_trig,           .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Shot 1 Trigger"},
        {.widget = button_apply_gamma ,   .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Apply"},   
        {.widget = button_apply_blc,      .wid_type = GTK_WIDGET_TYPE_BUTTON, .parent = NULL, .label_str = "Apply"},

        {.widget = check_button_auto_exposure, .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, .parent = NULL, .label_str = "Enable auto exposure"},
        {.widget = check_button_awb ,          .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, .parent = NULL, .label_str = "Enable auto white balance"},
        {.widget = check_button_auto_gain ,    .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, .parent = NULL, .label_str = "Enable auto brightness&contrast"},        
        {.widget = check_button_just_sensor,   .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, .parent = NULL, .label_str = "Just sensor read/write"},
        {.widget = check_button_trig_en,       .wid_type = GTK_WIDGET_TYPE_CHECK_BUTTON, .parent = NULL, .label_str = "Enable"},

        {.widget = hscale_exposure,   .wid_type = GTK_WIDGET_TYPE_HSCALE, .parent =  NULL, .label_str = "2505"},
        {.widget = hscale_gain,       .wid_type = GTK_WIDGET_TYPE_HSCALE, .parent =  NULL, .label_str = "64"},
 
        {.widget = entry_i2c_addr,    .wid_type = GTK_WIDGET_TYPE_ENTRY, .parent = NULL, .label_str = "0x"},
        {.widget = entry_reg_addr,    .wid_type = GTK_WIDGET_TYPE_ENTRY, .parent = NULL, .label_str = "0x"},
        {.widget = entry_reg_val,     .wid_type = GTK_WIDGET_TYPE_ENTRY, .parent = NULL, .label_str = "0x"},    
        {.widget = entry_gamma,       .wid_type = GTK_WIDGET_TYPE_ENTRY, .parent = NULL, .label_str = "1"},
        {.widget = entry_blc,         .wid_type = GTK_WIDGET_TYPE_ENTRY, .parent = NULL, .label_str = "0"},

        {.widget = radio_raw10, .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_datatype, .label_str = "RAW10"},
        {.widget = radio_raw12, .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_datatype, .label_str = "RAW12"},
        {.widget = radio_yuyv,  .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_datatype, .label_str = "YUYV"},
        {.widget = radio_raw8,  .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_datatype, .label_str = "RAW8"},
        
        {.widget = radio_bg,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_bayer, .label_str = "BGGR"},
        {.widget = radio_gb,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_bayer, .label_str = "GBBR"},
        {.widget = radio_rg,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_bayer, .label_str = "RGGB"},
        {.widget = radio_gr,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_bayer, .label_str = "GRBG"},
        {.widget = radio_mono, .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_bayer, .label_str = "MONO"},

        {.widget = radio_8bit_addr,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_addr_width, .label_str = "8-bit"},
        {.widget = radio_16bit_addr,  .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_addr_width, .label_str = "16-bit"},

        {.widget = radio_8bit_val,   .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_val_width, .label_str = "8-bit"},
        {.widget = radio_16bit_val,  .wid_type = GTK_WIDGET_TYPE_RADIO_BUTTON, .parent = hbox_val_width, .label_str = "16-bit"},

    };
    iterate_def_elements(definitions, SIZE(definitions));
}


/** iterate over grid element for GUI layout */
void iterate_grid_elements(
    grid_elements *elements, size_t members)
{
    FOREACH_NELEM(elements, members, ele)
    {
       // printf("element is %x, col=%d, row=%d\r\n", ele->widget, ele->col, ele->row);
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
    grid_elements elements[] = {
        {.widget = label_device,          .col = col = 0,    .row = row= 0, .width = 1},
        {.widget = label_fw_rev,          .col = ++col,      .row = row,    .width = 1},
        {.widget = button_exit_streaming, .col = ++col,      row++,  .width = 1},
 
        {.widget = label_datatype,        .col = col = 0,    .row = row,    .width = 1},
        {.widget = hbox_datatype,         .col = ++col,      row++,  .width = 2},
 
        {.widget = label_bayer,           .col = col = 0,    .row = row,    .width = 1},
        {.widget = hbox_bayer,            .col = ++col,      row++,  .width = 2},
 
        {.widget = check_button_auto_exposure,.col = col = 0,    .row = row,    .width = 1},
        {.widget = check_button_awb,          .col = ++col,      .row = row,    .width = 1},
        {.widget = check_button_auto_gain,    .col = ++col,      row++,  .width = 1},
 
        {.widget = label_exposure,       .col = col = 0,    .row = row,    .width = 1},
        {.widget = hscale_exposure,      .col = ++col,      row++,  .width = 3},
 
        {.widget = label_gain,           .col = col = 0,    .row = row,    .width = 1},
        {.widget = hscale_gain,          .col = ++col,      row++,  .width = 3},
 
        {.widget = label_i2c_addr,            .col = col = 0,    .row = row,    .width = 1},
        {.widget = entry_i2c_addr,            .col = ++col,      .row = row,    .width = 1},
        {.widget = check_button_just_sensor,  .col = ++col,      row++,  .width = 1},
 
        {.widget = label_addr_width,     .col = col = 0,    .row = row,    .width = 1},
        {.widget = hbox_addr_width,      .col = ++col,      row++,  .width = 1},
 
        {.widget = label_val_width,      .col = col = 0,    .row = row,    .width = 1},
        {.widget = hbox_val_width,       .col = ++col,      row++,  .width = 1},
 
        {.widget = label_reg_addr,       .col = col = 0,    .row = row,    .width = 1},
        {.widget = entry_reg_addr,       .col = ++col,      .row = row,    .width = 1},
        {.widget = button_read,          .col = ++col,      row++,  .width = 1},
 
        {.widget = label_reg_val,        .col = col = 0,    .row = row,    .width = 1},
        {.widget = entry_reg_val,        .col = ++col,      .row = row,    .width = 1},
        {.widget = button_write,         .col = ++col,      row++,  .width = 1},
 
        {.widget = label_capture,        .col = col = 0,    .row = row,    .width = 1},
        {.widget = button_capture_bmp,   .col = ++col,      .row = row,    .width = 1},
        {.widget = button_capture_raw,   .col = ++col,      row++,  .width = 1},
 
        {.widget = label_gamma,          .col = col = 0,    .row = row,    .width = 1},
        {.widget = entry_gamma,          .col = ++col,      .row = row,    .width = 1},
        {.widget = button_apply_gamma,   .col = ++col,      row++,  .width = 1},
 
        {.widget = label_trig,           .col = col = 0,    .row = row,    .width = 1},
        {.widget = check_button_trig_en, .col = ++col,      .row = row,    .width = 1},
        {.widget = button_trig,          .col = ++col,      row++,  .width = 1},
 
        {.widget = label_blc,            .col = col = 0,    .row = row,    .width = 1},
        {.widget = entry_blc,            .col = ++col,      .row = row,    .width = 1},
        {.widget = button_apply_blc,     .col = ++col,      row++,  .width = 1},

    };
    iterate_grid_elements(elements, SIZE(elements));
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


/** element_callback to hold all elements callback info */
void list_all_element_callbacks()
{
    static element_callback callbacks[] = {
        {.widget = button_exit_streaming, .signal = "clicked", .handler = G_CALLBACK(exit_loop), .data = NULL},
 
        {.widget = radio_raw10,       .signal = "toggled", .handler = G_CALLBACK(radio_datatype), .data = (gpointer)"1"},
        {.widget = radio_raw12,       .signal = "toggled", .handler = G_CALLBACK(radio_datatype), .data = (gpointer)"2"},
        {.widget = radio_yuyv,        .signal = "toggled", .handler = G_CALLBACK(radio_datatype), .data = (gpointer)"3"},
        {.widget = radio_raw8,        .signal = "toggled", .handler = G_CALLBACK(radio_datatype), .data = (gpointer)"4"},
 
        {.widget = radio_bg,          .signal = "toggled", .handler = G_CALLBACK(radio_bayerpattern), .data = (gpointer)"1"},
        {.widget = radio_gb,          .signal = "toggled", .handler = G_CALLBACK(radio_bayerpattern), .data = (gpointer)"2"},
        {.widget = radio_rg,          .signal = "toggled", .handler = G_CALLBACK(radio_bayerpattern), .data = (gpointer)"3"},
        {.widget = radio_gr,          .signal = "toggled", .handler = G_CALLBACK(radio_bayerpattern), .data = (gpointer)"4"},
        {.widget = radio_mono,        .signal = "toggled", .handler = G_CALLBACK(radio_bayerpattern), .data = (gpointer)"5"},
 
        {.widget = check_button_auto_exposure, .signal = "toggled", .handler = G_CALLBACK(enable_ae),  .data = NULL},
        {.widget = check_button_awb,           .signal = "toggled", .handler = G_CALLBACK(enable_awb), .data = NULL},
        {.widget = check_button_auto_gain,     .signal = "toggled", .handler = G_CALLBACK(enable_abc), .data = NULL},
 
        {.widget = hscale_exposure,   .signal = "value_changed", .handler = G_CALLBACK(hscale_exposure_up), .data = NULL},
        {.widget = hscale_gain,       .signal = "value_changed", .handler = G_CALLBACK(hscale_gain_up), .data = NULL},
 
        {.widget = radio_8bit_addr,   .signal = "toggled", .handler = G_CALLBACK(toggled_addr_length), .data = (gpointer)"1"},
        {.widget = radio_16bit_addr,  .signal = "toggled", .handler = G_CALLBACK(toggled_addr_length), .data = (gpointer)"2"},
 
        {.widget = radio_8bit_val,    .signal = "toggled", .handler = G_CALLBACK(toggled_val_length), .data = (gpointer)"1"},
        {.widget = radio_16bit_val,   .signal = "toggled", .handler = G_CALLBACK(toggled_val_length), .data = (gpointer)"2"},
 
        {.widget = button_read,   .signal = "clicked", .handler = G_CALLBACK(register_read), .data = NULL},
        {.widget = button_write,  .signal = "clicked", .handler = G_CALLBACK(register_write), .data = NULL},
 
        {.widget = button_capture_bmp, .signal = "clicked", .handler = G_CALLBACK(capture_bmp), .data = NULL},
        {.widget = button_capture_raw, .signal = "clicked", .handler = G_CALLBACK(capture_raw), .data =  NULL},
 
        {.widget = button_apply_gamma, .signal = "clicked", .handler = G_CALLBACK(gamma_correction), .data = NULL},
 
        {.widget = check_button_trig_en,  .signal = "toggled", .handler = G_CALLBACK(enable_trig), .data = NULL},
        {.widget = button_trig,           .signal = "clicked", .handler = G_CALLBACK(send_trigger), .data = NULL},
      
        {.widget = button_apply_blc,      .signal = "clicked", .handler = G_CALLBACK(black_level_correction), NULL},
 
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
    gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menu_bar), 
        GTK_PACK_DIRECTION_LTR); 
    
    device_menu = gtk_menu_new();
    file_menu = gtk_menu_new();
    fw_update_menu = gtk_menu_new();
    help_menu = gtk_menu_new();


    /** device items */
    device_item = gtk_menu_item_new_with_label("Devices");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(device_item), device_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), device_item);


    /** config file items */
    config_file_item = gtk_menu_item_new_with_label("Config File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config_file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), config_file_item);

    config_file_item = gtk_menu_item_new_with_label("Load json");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), NULL);

    config_file_item = gtk_menu_item_new_with_label("Load BatchCmd.txt");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), NULL);

    config_file_item = gtk_menu_item_new_with_label("Program Flash");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), NULL);
  
    config_file_item = gtk_menu_item_new_with_label("Program EEPROM");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), config_file_item);
    g_signal_connect(config_file_item, "activate", G_CALLBACK(open_config_dialog), NULL);
    

    /** firmware update items */
    fw_update_item = gtk_menu_item_new_with_label("FW Update");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fw_update_item), fw_update_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), fw_update_item);

    fw_update_item = gtk_menu_item_new_with_label("Reset Camera");
    gtk_menu_shell_append(GTK_MENU_SHELL(fw_update_menu), fw_update_item);
    g_signal_connect(fw_update_item, "activate", G_CALLBACK(fw_update_clicked), NULL);

    fw_update_item = gtk_menu_item_new_with_label("Erase");
    gtk_menu_shell_append(GTK_MENU_SHELL(fw_update_menu), fw_update_item);
    g_signal_connect(fw_update_item, "activate", G_CALLBACK(fw_update_clicked), NULL);

    fw_update_item = gtk_menu_item_new_with_label("FW Update");
    gtk_menu_shell_append(GTK_MENU_SHELL(fw_update_menu), fw_update_item);
    g_signal_connect(fw_update_item, "activate", G_CALLBACK(fw_update_clicked), NULL);

    /** help items */
    help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    //g_signal_connect(help_item, "activate", G_CALLBACK(menu_response), NULL);


    help_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_item);

    help_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_item);

    gtk_container_add(GTK_CONTAINER(maintable), menu_bar);
}

void gui_run()
{
    // static GtkWidget *window = NULL; /** the main window */
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