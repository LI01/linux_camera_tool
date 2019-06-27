#!/bin/bash
if [[ "$EUID" = 0 ]]; then
    echo "(1) already root"
    rm -f /etc/udev/rules.d/88-cyusb.rules
    rm -f /etc/cyusb.conf
    rm -f /usr/local/bin/cy_renumerate.sh
    echo "uninstall successfully"
else
    sudo -k # make sure to ask for password on next sudo
    if sudo true; then
        echo "(2) correct password"
        rm -f /etc/udev/rules.d/88-cyusb.rules
        rm -f /etc/cyusb.conf
        rm -f /usr/local/bin/cy_renumerate.sh
        echo "uninstall successfully"
    else
        echo "(3) wrong password"
        exit 1
    fi
fi
