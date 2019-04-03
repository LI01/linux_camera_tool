
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "ui_control.h"

extern void video_capture_save_bmp();
extern void video_capture_save_raw();

void capture_bmp (GtkWidget* widget) {
    video_capture_save_bmp();
}
void capture_raw (GtkWidget* raw) {
    video_capture_save_raw();
}
void radio_datatype(GtkWidget* widget, gpointer data) {
    if (strcmp((char*)data, "1") == 0) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("RAW10 %s button is set to active\n", (char*)data);
        else
            g_print("RAW10 %s button is set to deactive\n",(char*)data);
    }
    if (strcmp((char*)data, "2") == 0) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("RAW12 %s button is set to active\n", (char*)data);
        else
            g_print("RAW12 %s button is set to deactive\n",(char*)data);
    }

    if (strcmp((char*)data, "3") == 0) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("YUV422 %s button is set to active\n", (char*)data);
        else
            g_print("YUV422 %s button is set to deactive\n",(char*)data);
    }
}

void hscale_exposure_up(GtkRange *widget, gpointer data) {
    int exposure_time;
    exposure_time = (int)gtk_range_get_value(widget);
    g_print("exposure is %d lines\n", exposure_time);
}

void hscale_gain_up(GtkRange *widget, gpointer data) {
    int gain;
    gain = (int)gtk_range_get_value(widget);
    //set_gain(dev, gain);
    g_print("gain is %d\n", gain);
}

void toggled_addr_length(GtkWidget* widget, gpointer data) {
    if (strcmp((char*)data, "1") == 0) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("8-bit %s button is set to active\n", (char*)data);
        else
            g_print("8-bit %s button is set to deactive\n",(char*)data);
    }
    if (strcmp((char*)data, "2") == 0) {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            g_print("16-bit %s button is set to active\n", (char*)data);
        else
            g_print("16-bit %s button is set to deactive\n",(char*)data);
    }
}


// void trim_leading_0x(char *hex_val) {
//     if (hex_val == NULL) return;
//     if (hex_val[0] == '0' && hex_val[1] == 'x') {
//         hex_val[0] = ' ';
//         hex_val[1] = ' ';
//     }
// }

void reg_update(GtkWidget *widget, gpointer data) {

   // int i2c_addr;
   // i2c_addr = atoi(trim_leading_0x((char *) 
     //       gtk_entry_get_text(GTK_ENTRY(entry_i2c_addr))));
    //g_print("i2c addr = 0x%s", i2c_addr);
}


//fail
//g++ ui_control.cpp -o test2 `gtk-config --cflags --libs`
//pass
//g++ ui_control.cpp -o test `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
int init_control_gui(int argc, char* argv[]) {
//int main(int argc, char* argv[]) {
    /* --- GTK initialization --- */
    gtk_init(&argc, &argv);
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Control");
    gtk_widget_set_size_request(window, 500, 100);

    GtkWidget *grid;
    grid = gtk_grid_new();

    GtkWidget *label_datatype,*vbox2, *radio01, *radio02, *radio03;
    GtkWidget *entry_i2c_addr;

    GtkWidget *label_exposure, *label_gain;
    GtkWidget *hscale_exposure, *hscale_gain;
    GtkWidget *label_i2c_addr;
    GtkWidget *label_addr_width,*vbox, *radio1, *radio2;
    GtkWidget *label_reg_addr, *entry_reg_addr;
    GtkWidget *label_reg_val, *entry_reg_val;
    GtkWidget *button_read, *button_write;
    GtkWidget *check_button_just_sensor;
    GtkWidget *label_capture, *button_capture_bmp, *button_capture_raw; 
    GtkWidget *label_gamma, *entry_gamma, *button_apply_gamma;

    /* --- row 0 --- */
    label_datatype = gtk_label_new("Sensor Datatype:");
    gtk_label_set_text(GTK_LABEL(label_datatype), "Sensor Datatype:");
    vbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
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
   (gpointer)"1");
       g_signal_connect(radio02, "toggled", G_CALLBACK(radio_datatype), 
   (gpointer)"2");
       g_signal_connect(radio03, "toggled", G_CALLBACK(radio_datatype), 
   (gpointer)"3");


    /* --- row 1 and row 2 --- */
    label_exposure = gtk_label_new("Exposure:");
    gtk_label_set_text(GTK_LABEL(label_exposure), "Exposure:");

    label_gain = gtk_label_new("Gain:");
    gtk_label_set_text(GTK_LABEL(label_gain), "Gain:");
   
    hscale_exposure = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 
                        0.0, 400.0, 1.0);
    hscale_gain = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 
                        0.0, 63.0, 1.0);
    
    gtk_widget_set_hexpand (hscale_exposure, TRUE);
    gtk_widget_set_hexpand (hscale_gain, TRUE);
    
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
    vbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    radio1 = gtk_radio_button_new_with_label(NULL, "8 bit");
    gtk_box_pack_start(GTK_BOX(vbox), radio1, 0, 0, 0);
    radio2 = gtk_radio_button_new_with_label(
    gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)), "16 bit");
    gtk_box_pack_start(GTK_BOX(vbox), radio2, 0, 0, 0);    
    //TODO: callback
    g_signal_connect(radio1, "toggled", G_CALLBACK(toggled_addr_length),
    (gpointer)"1");
    g_signal_connect(radio2, "toggled", G_CALLBACK(toggled_addr_length),
    (gpointer)"2");    

    /* ---fifth row--- */
    label_reg_addr = gtk_label_new("Reg Addr:");
    gtk_label_set_text(GTK_LABEL(label_reg_addr), "Reg Addr:");
    entry_reg_addr = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_reg_addr), "0x");
    gtk_entry_set_width_chars(GTK_ENTRY(entry_reg_addr), -1);
    button_read = gtk_button_new_with_label("Read");
    //TODO: callback

    /* --- sixth row --- */
    label_reg_val = gtk_label_new("Reg Value:");
    gtk_label_set_text(GTK_LABEL(label_reg_val), "Reg Value:");
    entry_reg_val = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_reg_val), "0x");
    gtk_entry_set_width_chars(GTK_ENTRY(entry_reg_val), -1);
    button_write = gtk_button_new_with_label("Write");
    //TODO: callback
    //g_signal_connect(button_write, "clicked", G_CALLBACK(reg_update), NULL);
    
    /* --- seventh row --- */
    label_capture = gtk_label_new("Capture:");
    gtk_label_set_text(GTK_LABEL(label_capture), "Capture:");
    button_capture_bmp = gtk_button_new_with_label("Capture bmp");
    button_capture_raw = gtk_button_new_with_label("Capture raw");
    //TODO: callback
    g_signal_connect(button_capture_bmp, "clicked", G_CALLBACK(capture_bmp), NULL);
    g_signal_connect(button_capture_raw, "clicked", G_CALLBACK(capture_raw),NULL);

    /* --- eighth row --- */
    label_gamma = gtk_label_new("Gamma Correction:");
    gtk_label_set_text(GTK_LABEL(label_gamma), "Gamma Correction:");
    entry_gamma = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry_gamma), -1);
    button_apply_gamma = gtk_button_new_with_label("Apply");

    /* --- Layout, don't change --- */
    // zero row: choose sensor datatype for data shift 
    gtk_grid_attach(GTK_GRID(grid), label_datatype, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vbox2, 1, 0, 2, 1);
    // first row: exposure 
    gtk_grid_attach(GTK_GRID(grid), label_exposure, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hscale_exposure, 1, 1, 3, 1);
    // second row: gain
    gtk_grid_attach(GTK_GRID(grid), label_gain, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hscale_gain, 1, 2, 3, 1);    
    // third row: i2c addr 
    gtk_grid_attach(GTK_GRID(grid), label_i2c_addr, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_i2c_addr, 1, 3, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid), check_button_just_sensor, 2, 3, 1, 1);
    // fourth row: reg addr width
    gtk_grid_attach(GTK_GRID(grid), label_addr_width, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vbox, 1, 4, 1, 1);  
    // fifth row: reg addr 
    gtk_grid_attach(GTK_GRID(grid), label_reg_addr, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_reg_addr, 1, 5, 1, 1); 
    gtk_grid_attach(GTK_GRID(grid), button_read, 2, 5, 1, 1); 
    // sixth row: reg val 
    gtk_grid_attach(GTK_GRID(grid), label_reg_val, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_reg_val, 1, 6, 1, 1);      
    gtk_grid_attach(GTK_GRID(grid), button_write, 2, 6, 1, 1);  
    // seventh row: capture bmp, raw
    gtk_grid_attach(GTK_GRID(grid), label_capture, 0, 7, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid), button_capture_bmp, 1, 7, 1, 1);      
    gtk_grid_attach(GTK_GRID(grid), button_capture_raw, 2, 7, 1, 1);       
    // eight row: gamma correction
    gtk_grid_attach(GTK_GRID(grid), label_gamma, 0, 8, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid), entry_gamma, 1, 8, 1, 1);      
    gtk_grid_attach(GTK_GRID(grid), button_apply_gamma, 2, 8, 1, 1);       
        


    /* --- Grid Setup --- */
	gtk_grid_set_column_homogeneous (GTK_GRID(grid), FALSE);
	gtk_widget_set_hexpand (grid, TRUE);
	gtk_widget_set_halign (grid, GTK_ALIGN_FILL);
	gtk_grid_set_row_spacing (GTK_GRID(grid), 6);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 6);
	gtk_container_set_border_width (GTK_CONTAINER (grid), 2);


    gtk_container_add(GTK_CONTAINER(window), grid);
    g_signal_connect(window,"delete-event",G_CALLBACK(gtk_main_quit),
                NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}