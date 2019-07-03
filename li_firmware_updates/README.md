# LI Firmware Update Tool

A simple and easy to use firmware update tool for Leopard USB3 camera.

- [LI Firmware Update Tool](#LI-Firmware-Update-Tool)
  - [Installation](#Installation)
    - [Install Firmware Update Tool](#Install-Firmware-Update-Tool)
    - [Uninstall Firmware Update Tool](#Uninstall-Firmware-Update-Tool)
  - [Firmware Updates](#Firmware-Updates)
  - [Code Structure](#Code-Structure)


## Installation
### Install Firmware Update Tool
__First time running__
To install the rules and configs for firmware updates 
```sh
sudo chmod 777 firmware.sh
./firmware.sh install 
```
### Uninstall Firmware Update Tool
To uninstall the rules and configs for firmware updates
```sh
./firmware.sh uninstall
```
---

## Firmware Updates
1. Erase the current firmware in this USB3 board
  - Run linux camera tool (``` ./leopard_cam```)
  - Inside __FW Updates__, click __Erase Firmware__. (_Linux camera tool_ then will exit)
2. Power cycle your USB3 board, do a ```lsusb```, make sure a device called ```04b4:00f3 Cypress Semiconductor Corp.``` is listed. 
   Go to __4__ if you find your Cypress device, otherwise, go over __1,2__ again
3. Go inside __li_firmware_updates__, have your new firmware update binary with you(end with __img__/__lif__ file)
    ```sh
    cd li_firmware_updates
    ./firmware.sh update_firmware xxx(new img/lif file)
    ```
    Log file in terminal will tell you whether it succeed or not.
    If it failed, try to __power cycle your board__ to clear things in RAM and restart from __1__


---

## Code Structure
```
.
├── bin
│   ├── cyfxflashprog.img       # this is needed to recognize cypress device after erase firmware
│   ├── download_fx3            # executable for firmware update for FX3
│   ├── download_fx3.c          # firmware update source code
│   ├── eos.conf.pb
│   ├── eos.log
│   ├── libcyusb.so             # shared library
│   └── libcyusb.so.1           # shared library
│ 
├── firmware.sh                 # shell script to install, uninstall and update firmware
├── cyfxuvc_rev1425.lif         # test lif file 1
│
├── Flash_factory.img           # test img file 1
├── Flash_update.img            # test img file 2
├── install
│   ├── 88-cyusb.rules          
│   ├── cy_renumerate.sh
│   ├── cyusb.conf
│   ├── install.sh              # script to install rules and configs
│   └── uninstall.sh            # script to uninstall rules and configs
└── README.md                   
```