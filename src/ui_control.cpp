
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "../includes/shortcuts.h"
#include "ui_control.h"
/****************************************************************************
**                      	Global data 
*****************************************************************************/
GtkWidget *label_device, *label_hw_rev, *label_fw_rev;
GtkWidget *label_datatype, *vbox2, *radio01, *radio02, *radio03;
GtkWidget *label_bayer, *vbox3, *radio_bg, *radio_gb, *radio_rg, *radio_gr;
GtkWidget *label_exposure, *label_gain;
GtkWidget *hscale_exposure, *hscale_gain;
GtkWidget *label_i2c_addr, *entry_i2c_addr;
GtkWidget *label_addr_width, *vbox, *radio1, *radio2;
GtkWidget *label_reg_addr, *entry_reg_addr;
GtkWidget *label_reg_val, *entry_reg_val;
GtkWidget *button_read, *button_write;
GtkWidget *check_button_just_sensor;
GtkWidget *check_button_auto_exposure;
GtkWidget *label_capture, *button_capture_bmp, *button_capture_raw;
GtkWidget *label_gamma, *entry_gamma, *button_apply_gamma;
GtkWidget *label_trig, *check_button_trig_en, *button_trig;

/****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern int v4l2_dev;
extern int fw_rev;

extern char *get_manufacturer_name();
extern char *get_product();
extern char *get_serial();
extern void video_capture_save_bmp();
extern void video_capture_save_raw();

extern void set_exposure_absolute(int fd, int exposure_absolute);
extern void set_gain(int fd, int analog_gain);
extern void set_exposure_auto(int fd, int exposure_auto);

extern int read_cam_uuid_hwfw_rev(int fd);
extern void soft_trigger(int fd);
extern void trigger_enable(int fd, int ena, int enb);
extern void sensor_reg_write(int fd,int regAddr, int regVal);
extern int sensor_reg_read(int fd,int regAddr);
extern void generic_I2C_write(int fd,int rw_flag, int bufCnt,
					   int slaveAddr, int regAddr, unsigned char *i2c_data);
extern int generic_I2C_read(int fd,int rw_flag, int bufCnt,
					  int slaveAddr, int regAddr);
extern void change_datatype(void *datatype);
extern void change_bayerpattern(void *bayer);


/****************************************************************************
**                      	Internal Callbacks
*****************************************************************************/

void radio_datatype(GtkWidget *widget, gpointer data)
{
    change_datatype(data);
}

void radio_bayerpattern (GtkWidget *widget, gpointer data)
{
    change_bayerpattern(data);
}

void hscale_exposure_up(GtkRange *widget, gpointer data)
{
    int exposure_time;
    exposure_time = (int)gtk_range_get_value(widget);
    set_exposure_absolute(v4l2_dev, exposure_time);
    g_print("exposure is %d lines\n", exposure_time);

}

void hscale_gain_up(GtkRange *widget, gpointer data)
{
    int gain;
    gain = (int)gtk_range_get_value(widget);
    set_gain(v4l2_dev, gain);
    g_print("gain is %d\n", gain);
}

void capture_bmp(GtkWidget *widget)
{
    video_capture_save_bmp();
}
void capture_raw(GtkWidget *raw)
{
    video_capture_save_raw();
}

void register_write(GtkWidget *widget)
{
    //sensor write
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor))) 
    {
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
             NULL, 16);
        int regVal = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_val)),
             NULL, 16);
        sensor_reg_write(v4l2_dev,regAddr, regVal);
    }
    //generic write
    else
    {
        int slaveAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)),
             NULL, 16);
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
             NULL, 16);
        int regVal = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_val)),
             NULL, 16);
        generic_I2C_write(v4l2_dev,0x81, 1,
			slaveAddr, regAddr, (unsigned char*)regVal);
    }
}

void register_read(GtkWidget *widget)
{
    //sensor read
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_just_sensor))) 
    {
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
             NULL, 16);
             
        int regVal = sensor_reg_read(v4l2_dev,regAddr);
        char buf[6];
        snprintf(buf, sizeof(buf), "0x%x",regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);
    }
    //generic read
    else
    {
        int slaveAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr)),
             NULL, 16);
        int regAddr = strtol((char *)gtk_entry_get_text(GTK_ENTRY(entry_reg_addr)),
             NULL, 16);
        int regVal = generic_I2C_read(v4l2_dev,0x02, 1, slaveAddr, regAddr);
        char buf[6];
        snprintf(buf, sizeof(buf), "0x%x",regVal);
        gtk_entry_set_text(GTK_ENTRY(entry_reg_val), buf);

    }
}

void toggled_addr_length(GtkWidget *widget, gpointer data)
{
    if (strcmp((char *)data, "1") == 0)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("8-bit %s button is set to active\n", (char *)data);
        else
            g_print("8-bit %s button is set to deactive\n", (char *)data);
    }
    if (strcmp((char *)data, "2") == 0)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("16-bit %s button is set to active\n", (char *)data);
        else
            g_print("16-bit %s button is set to deactive\n", (char *)data);
    }
}

void enable_ae(GtkToggleButton *toggle_button)
{
    if (gtk_toggle_button_get_active(toggle_button))
        set_exposure_auto(v4l2_dev, 0);
    else
        set_exposure_auto(v4l2_dev, 1);
    
}
void send_trigger(GtkWidget *widget)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_trig_en))) {
        //positive edge
        trigger_enable(v4l2_dev, 1, 1);
        g_print("trigger enabled\n");
        soft_trigger(v4l2_dev);
        g_print("send one trigger\n");   
    }
    else 
    {   
        trigger_enable(v4l2_dev, 0, 0);
        g_print("trigger disabled\n");
    }
}

//fail
//g++ ui_control.cpp -o test2 `gtk-config --cflags --libs`
//pass
//g++ ui_control.cpp -o test `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
int init_control_gui(int argc, char *argv[])
{

    /* --- GTK initialization --- */
    gtk_init(&argc, &argv);
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Control");
    gtk_widget_set_size_request(window, 500, 100);

    GtkWidget *grid;
    grid = gtk_grid_new();

    /* --- row 0 --- */
    char device_buf[64];
    snprintf(device_buf, sizeof(device_buf), "Device: %s - %s",
        get_manufacturer_name(), get_product());
    label_device = gtk_label_new("Device:");
    gtk_label_set_text(GTK_LABEL(label_device), device_buf);


    
    char hw_rev_buf[20];
    snprintf(hw_rev_buf, sizeof(hw_rev_buf), "Hardware Rev: %d",
        fw_rev);
    // label_hw_rev = gtk_label_new("Hardware Rev:");
    // gtk_label_set_text(GTK_LABEL(label_hw_rev), hw_rev_buf);

    char fw_rev_buf[20];
    snprintf(fw_rev_buf, sizeof(fw_rev_buf), "Firmware Rev: %d",
        fw_rev);
    label_fw_rev = gtk_label_new("Firmware Rev:");
    gtk_label_set_text(GTK_LABEL(label_fw_rev), fw_rev_buf);


    /* --- row 1 --- */
    label_datatype = gtk_label_new("Sensor Datatype:");
    gtk_label_set_text(GTK_LABEL(label_datatype), "Sensor Datatype:");
    vbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    radio01 = gtk_radio_button_new_with_label(NULL, "RAW10");
    gtk_box_pack_start(GTK_BOX(vbox2), radio01, 0, 0, 0);
    radio02 = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio01)), "RAW12");
    gtk_box_pack_start(GTK_BOX(vbox2), radio02, 0, 0, 0);
    radio03 = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio01)), "YUYV");
    gtk_box_pack_start(GTK_BOX(vbox2), radio03, 0, 0, 0);
    //TODO: callback
    g_signal_connect(radio01, "toggled", G_CALLBACK(radio_datatype),
                     (gpointer) "1");
    g_signal_connect(radio02, "toggled", G_CALLBACK(radio_datatype),
                     (gpointer) "2");
    g_signal_connect(radio03, "toggled", G_CALLBACK(radio_datatype),
                     (gpointer) "3");

    /* --- row 2 --- */
    //GtkWidget *label_bayer, *vbox3, *radio_bg, *radio_gb, *radio_rg, *radio_gr;
    label_bayer = gtk_label_new("Raw Camera Pixel Format:");
    gtk_label_set_text(GTK_LABEL(label_bayer), "Raw Camera Pixel Format:");
    
    vbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    radio_bg = gtk_radio_button_new_with_label(NULL, "BGGR");
    gtk_box_pack_start(GTK_BOX(vbox3), radio_bg, 0, 0, 0);
    radio_gb = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)), "GBBR");
    gtk_box_pack_start(GTK_BOX(vbox3), radio_gb, 0, 0, 0);
    radio_rg = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)), "RGGB");
    gtk_box_pack_start(GTK_BOX(vbox3), radio_rg, 0, 0, 0);
    radio_gr = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_bg)), "GRBG");
    gtk_box_pack_start(GTK_BOX(vbox3), radio_gr, 0, 0, 0);
    //TODO: callback
    g_signal_connect(radio_bg, "toggled", G_CALLBACK(radio_bayerpattern),
                     (gpointer) "1");
    g_signal_connect(radio_gb, "toggled", G_CALLBACK(radio_bayerpattern),
                     (gpointer) "2");
    g_signal_connect(radio_rg, "toggled", G_CALLBACK(radio_bayerpattern),
                     (gpointer) "3");   
    g_signal_connect(radio_gr, "toggled", G_CALLBACK(radio_bayerpattern),
                     (gpointer) "4");   

    /* --- row 3 and row 4 --- */
    label_exposure = gtk_label_new("Exposure:");
    gtk_label_set_text(GTK_LABEL(label_exposure), "Exposure:");

    label_gain = gtk_label_new("Gain:");
    gtk_label_set_text(GTK_LABEL(label_gain), "Gain:");

    hscale_exposure = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                               0.0, 4000.0, 1.0);
    hscale_gain = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                           0.0, 63.0, 1.0);

    gtk_widget_set_hexpand(hscale_exposure, TRUE);
    gtk_widget_set_hexpand(hscale_gain, TRUE);

    //TODO:callback
    g_signal_connect(G_OBJECT(hscale_exposure), "value_changed",
                     G_CALLBACK(hscale_exposure_up), NULL);
    g_signal_connect(G_OBJECT(hscale_gain), "value_changed",
                     G_CALLBACK(hscale_gain_up), NULL);

    /* --- third row ---*/
    label_i2c_addr = gtk_label_new("I2C Addr:");
    gtk_label_set_text(GTK_LABEL(label_i2c_addr), "I2C Addr:");
    entry_i2c_addr = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_i2c_addr), "0x");
    gtk_entry_set_width_chars(GTK_ENTRY(entry_i2c_addr), -1);
    check_button_just_sensor = gtk_check_button_new_with_label("Just sensor read/write");
    //TODO: callback

    /* --- fourth row --- */
    label_addr_width = gtk_label_new("I2C Addr Width");
    gtk_label_set_text(GTK_LABEL(label_addr_width), "I2C Addr Width:");
    vbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    radio1 = gtk_radio_button_new_with_label(NULL, "8 bit");
    gtk_box_pack_start(GTK_BOX(vbox), radio1, 0, 0, 0);
    radio2 = gtk_radio_button_new_with_label(
        gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)), "16 bit");
    gtk_box_pack_start(GTK_BOX(vbox), radio2, 0, 0, 0);
    check_button_auto_exposure = gtk_check_button_new_with_label("Enable auto exposure");
    //TODO: callback
    g_signal_connect(radio1, "toggled", G_CALLBACK(toggled_addr_length),
                     (gpointer) "1");
    g_signal_connect(radio2, "toggled", G_CALLBACK(toggled_addr_length),
                     (gpointer) "2");
    g_signal_connect(GTK_TOGGLE_BUTTON(check_button_auto_exposure), "toggled",
        G_CALLBACK(enable_ae), NULL);

    /* ---fifth row--- */
    label_reg_addr = gtk_label_new("Reg Addr:");
    gtk_label_set_text(GTK_LABEL(label_reg_addr), "Reg Addr:");
    entry_reg_addr = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_reg_addr), "0x");
    gtk_entry_set_width_chars(GTK_ENTRY(entry_reg_addr), -1);
    button_read = gtk_button_new_with_label("Read");
    //TODO: callback
    g_signal_connect(button_read, "clicked", G_CALLBACK(register_read), NULL);

    /* --- sixth row --- */
    label_reg_val = gtk_label_new("Reg Value:");
    gtk_label_set_text(GTK_LABEL(label_reg_val), "Reg Value:");
    entry_reg_val = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_reg_val), "0x");
    gtk_entry_set_width_chars(GTK_ENTRY(entry_reg_val), -1);
    button_write = gtk_button_new_with_label("Write");
    //TODO: callback
    g_signal_connect(button_write, "clicked", G_CALLBACK(register_write), NULL);

    /* --- seventh row --- */
    label_capture = gtk_label_new("Capture:");
    gtk_label_set_text(GTK_LABEL(label_capture), "Capture:");
    button_capture_bmp = gtk_button_new_with_label("Capture bmp");
    button_capture_raw = gtk_button_new_with_label("Capture raw");
    //TODO: callback
    g_signal_connect(button_capture_bmp, "clicked", G_CALLBACK(capture_bmp), NULL);
    g_signal_connect(button_capture_raw, "clicked", G_CALLBACK(capture_raw), NULL);

    /* --- eighth row --- */
    label_gamma = gtk_label_new("Gamma Correction:");
    gtk_label_set_text(GTK_LABEL(label_gamma), "Gamma Correction:");
    entry_gamma = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry_gamma), -1);
    button_apply_gamma = gtk_button_new_with_label("Apply");


    /* --- eighth row --- */
    label_trig = gtk_label_new("Trigger Sensor:");
    check_button_trig_en = gtk_check_button_new_with_label("Enable");
    button_trig = gtk_button_new_with_label("Shot 1 trigger");
    g_signal_connect(button_trig, "clicked", G_CALLBACK(send_trigger), NULL);

    /* --- Layout, don't change --- */
    // zero row: device info, hw, fw revision
    int row = 0;
    int col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_device, col++, row, 1, 1);
    //gtk_grid_attach(GTK_GRID(grid), label_hw_rev, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_fw_rev, col++, row, 1, 1);

    //first row: choose sensor datatype for data shift
    row ++;
    col = 0;   
    gtk_grid_attach(GTK_GRID(grid), label_datatype, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vbox2, col++, row, 2, 1);
    // second row: bayer pattern
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_bayer, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vbox3, col++, row, 2, 1);
    // third row: exposure
    row ++;
    col = 0;    
    gtk_grid_attach(GTK_GRID(grid), label_exposure, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hscale_exposure, col++, row, 3, 1);
    // forth row: gain
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_gain, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hscale_gain, col++, row, 3, 1);
    // fifth row: i2c addr
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_i2c_addr, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_i2c_addr, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button_just_sensor, col++, row, 1, 1);
    // sixth row: reg addr width
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_addr_width, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vbox, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button_auto_exposure, col++, row, 1, 1);
    // seventh row: reg addr
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_reg_addr, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_reg_addr, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_read, col++, row, 1, 1);
    // eighth row: reg val
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_reg_val, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_reg_val, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_write, col++, row, 1, 1);
    // ninth row: capture bmp, raw
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_capture, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_capture_bmp, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_capture_raw, col++, row, 1, 1);
    // tenth row: gamma correction
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_gamma, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_gamma, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_apply_gamma, col++, row, 1, 1);
    // eleventh row: trigger
    row ++;
    col = 0;
    gtk_grid_attach(GTK_GRID(grid), label_trig, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button_trig_en, col++, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_trig, col++, row, 1, 1);
    row++;
    col = 0;
    /* --- Grid Setup --- */
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 2);

    gtk_container_add(GTK_CONTAINER(window), grid);
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit),
                     NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}