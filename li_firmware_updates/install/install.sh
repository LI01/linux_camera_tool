#!/bin/bash
if [[ "$EUID" = 0 ]]; then
    echo "(1) already root"
    # create udev rules
    cp li_firmware_updates/install/88-cyusb.rules /etc/udev/rules.d
    cp li_firmware_updates/install/cyusb.conf /etc
    
    cp li_firmware_updates/install/cy_renumerate.sh /usr/local/bin
    echo "install successfully"
else
    sudo -k # make sure to ask for password on next sudo
    if sudo true; then
        echo "(2) correct password"
        cp li_firmware_updates/install/88-cyusb.rules /etc/udev/rules.d
        cp li_firmware_updates/install/cyusb.conf /etc
        cp li_firmware_updates/install/cy_renumerate.sh /usr/local/bin
        echo "install successfully"
    else
        echo "(3) wrong password"
        exit 1
    fi
fi

