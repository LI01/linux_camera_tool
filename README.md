# Leopard USB3.0 Camera Linux Camera Tool

  This is the sample code for Leopard USB3.0 camera linux camera tool. It is a simple interface for capturing, viewing, controlling video from v4l2 devices, with a special emphasis for the linux uvc driver. 
  

- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [OpenCV Prerequisites](#opencv-prerequisites)
  - [Build Camera Tool](#build-camera-tool)
- [Code Structure](#code-structure)
- [Run Camera Tool](#run-camera-tool)
  - [Examples](#examples)
    - [Declarations](#declarations)
    - [Test on RAW sensor 12 Megapixel IMX477](#test-on-raw-sensor-12-megapixel-imx477)
    - [Test on RAW sensor 5 Megapixel OS05A20](#test-on-raw-sensor-5-megapixel-os05a20)
    - [Test on AR1335-ICP3 YUV 12 Megapixel Cam](#test-on-ar1335-icp3-yuv-12-megapixel-cam)
  - [Exit Camera Tool](#exit-camera-tool)
  - [Kill Camera Tool Windows Left Over](#kill-camera-tool-windows-left-over)
- [Test Platforms](#test-platforms)
- [Known Bugs](#known-bugs)

---
## Installation

### Dependencies
Make sure the libraries have installed. Run configure.sh for completing installing all the required dependencies
```sh
chmod +X configure.sh
./configure.sh
```

### OpenCV Prerequisites
Make sure you have GTK 3 and OpenCV (3 at least) installed. The way you do to install this package varies by operational system.

Gtk3 and Gtk2 don't live together peaceful. If you try to run camera tool and got this error message:

    Gtk-ERROR **: GTK+ 2.x symbols detected. Using GTK+ 2.x and GTK+ 3 in the same proc

It is mostly you have OpenCV build with GTk2 support. The only way to fix it is rebuild OpenCV without GTk2:

    opencv_dir/release$cmake [other options] -D WITH_GTK_2_X=OFF ..

in order to disable Gtk2 from OpenCV.

```sh
#rebuild opencv, this might take a while
rm -rf build/
mkdir build && cd build/
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_NEW_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON -D WITH_GTK=ON -D WITH_GTK3=ON -D WITH_GTK_2_X=OFF ..
make -j6                     #do a $lscpu to see how many cores for CPU
sudo make install -j6

#link opencv
sudo sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
```

### Build Camera Tool

- Makefile
```sh
make
```

- CMake
```sh
mkdir build
cd build
cmake ../
make

# Add to your project
add_subdirectory(linux_camera_tool)
include_directories(
	linux_camera_tool/src/
)

...

target_link_libraries(<Your Target>
	leopard_tools
)
```

---

## Code Structure
```
$ linux_camera_tool .
├── Makefile
├── configure.sh    
├── CMakeLists.txt
│
├── README.md
├── CHANGELOG.md
├── LICENSE.md
├── AUTHOR.md
│
├── includes
│   ├── shortcuts.h                     # common header
│   ├── cam_property.h
│   ├── extend_cam_ctrl.h
|   ├── json_parser.h
│   ├── ui_control.h
│   ├── uvc_extension_unit_ctrl.h
│   └── v4l2_devices.h
│
├── src
│   ├── cam_property.cpp
│   ├── extend_cam_ctrl.cpp
|   ├── json_parser.cpp
│   ├── uvc_extension_unit_ctrl.cpp
│   └── v4l2_device.cpp
│   
└── test
    ├── ui_control.cpp
    └── main.c
```
---
## Run Camera Tool
```
./leopard_cam
```

### Examples

#### Declarations 
_Auto white balance_, _gamma correction_ and _auto brightness and contrast_ are done by mainly using opencv, since histogram matrix calculations are involved, enabling these features will slow down the streaming a lot.
_Auto exposure_ is usually implemented on sensor/isp, which when enabled, won't further slow down the streaming, need to check with camera driver for auto exposure support.

#### Test on RAW sensor 12 Megapixel IMX477
__Original streaming for IMX477:__ 
> image is dark and blue
> <img src="pic/imx477.jpg" width="1000">

__Modified streaming for IMX477:__
> enabled software AWB, gamma correction
> read & write register from IMX477
> <img src="pic/imx477regCtrl.jpg" width="1000">

> enable software auto brightness and contrast
> <img src="pic/imx477Abc.jpg" width="1000">

#### Test on RAW sensor 5 Megapixel OS05A20
__Modified streaming for OS05A20 full resolution__
> change bayer pattern to GRBG
> enable software AWB, gamma correction
> increase exposure, gain
> <img src="pic/os05a20.jpg" width="1000">


__Modified OS05A20 resolution to an available one__
> binning mode
> capture raw and bmp, save them in the camera tool directory
<img src="pic/changeResOS05A20.jpg" width="1000">

#### Test on AR1335-ICP3 YUV 12 Megapixel Cam

__Original streaming for AR1335 ICP3 YUV cam:__
> Default ae enabled -> change exposure&gain takes no effects
> <img src="pic/aeEnableNotChangeExp.jpg" width="1000">

__Disable ae:__
> Being able to change exposure & gain taking effective
><img src="pic/aeDisable.jpg" width="1000">

---

#### Exit Camera Tool
1. Use __ESC__ on both of GUI.
2. Use __EXIT__ button on the control panel to exit the whole program
3. User __Ctrl + C__ to exit the control panel after exit from camera streaming GUI

#### Kill Camera Tool Windows Left Over
If you forget to exit both windows and tried to run the camera tool again, it will give you the error of 
```sh
VIDIOC_DQBUF: Invalid argument
```
Please check your available processes and kill leopard_cam, then restart the camera tool application
```sh
ps
killall -9 leopard_cam
```
---
## Test Platforms
- __4.18.0-17-generic #18~18.04.1-Ubuntu SMP__ 
- __4.15.0-32-generic #35~16.04.1-Ubuntu__
- __4.15.0-20-generic #21~Ubuntu 18.04.1 LTS__

---
## Known Bugs

### Exposure & Gain Control Momentarily Split Screen
When changing exposure and gain under linux, camera tool will experience split screen issue at the moment change is happened. This issue happens for the USB3 camera that use manual DMA in the FX3 driver. For the camera that utilizes auto DMA, the image will be ok when exposure and gain change happens. 

For updating driver from manual DMA to auto DMA, you need to ensure:
1. PTS embedded information is not needed in each frame
2. The camera board has a Crosslink FPGA on the tester board that is able to add uvc header to quality for auto DMA
3. FX3 driver also need to be updated.

### SerDes Camera Experiences First Frame Bad When Uses Trigger
When use triggering mode instead of master free running mode for USB3 SerDes camera, the very first frame received will be bad and should be tossed away. It is recommended to use an external function generator or a dedicated triggering signal for triggering the cameras for the purpose of syncing different SerDes cameras. 

The included "shot 1 trigger" function is only a demonstration on generating one pulse and let camera output one frame after "shot 1 trigger" is clicked once. User should not fully rely on this software generated trigger but use a hardware trigger for sync the camera streaming.




