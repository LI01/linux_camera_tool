# Leopard USB3.0 Camera Linux Camera Tool

  This is the sample code for Leopard USB3.0 camera linux camera tool. It is a simple interface for capturing, viewing, controlling video from v4l2 devices, with a special emphasis for the linux uvc driver. 
  

- [Leopard USB3.0 Camera Linux Camera Tool](#Leopard-USB30-Camera-Linux-Camera-Tool)
  - [Installation](#Installation)
    - [Dependencies](#Dependencies)
    - [OpenCV Prerequisites](#OpenCV-Prerequisites)
    - [Build Camera Tool](#Build-Camera-Tool)
    - [Install leopard_cam](#Install-leopard_cam)
    - [Uninstall leopard_cam](#Uninstall-leopard_cam)
  - [Code Structure](#Code-Structure)
  - [Declarations](#Declarations)
  - [Regarding Exposure Time Calculation](#Regarding-Exposure-Time-Calculation)
    - [Examples](#Examples)
      - [Test on RAW sensor 12 Megapixel IMX477](#Test-on-RAW-sensor-12-Megapixel-IMX477)
      - [Test on RAW sensor 5 Megapixel OS05A20](#Test-on-RAW-sensor-5-Megapixel-OS05A20)
      - [Test on AR1335-ICP3 YUV 12 Megapixel Cam](#Test-on-AR1335-ICP3-YUV-12-Megapixel-Cam)
      - [Exit Camera Tool](#Exit-Camera-Tool)
      - [Kill Camera Tool Windows Left Over](#Kill-Camera-Tool-Windows-Left-Over)
  - [Test Platforms](#Test-Platforms)
  - [Known Bugs](#Known-Bugs)
    - [Exposure & Gain Control Momentarily Split Screen](#Exposure--Gain-Control-Momentarily-Split-Screen)
    - [SerDes Camera Experiences First Frame Bad When Uses Trigger](#SerDes-Camera-Experiences-First-Frame-Bad-When-Uses-Trigger)

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
### Install leopard_cam
```sh
sudo cp ./leopard_cam /usr/bin
```
### Uninstall leopard_cam
```sh
sudo rm -f /usr/bin/leopard_cam 
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
├── config.json
├── BatchCmd.txt
|
├── includes
│   ├── shortcuts.h                       # common header
│   ├── cam_property.h
│   ├── extend_cam_ctrl.h
│   ├── core_io.h
|   ├── json_parser.h
│   ├── batch_cmd_parser.h
│   ├── ui_control.h
│   ├── uvc_extension_unit_ctrl.h
│   └── v4l2_devices.h
│
├── src
│   ├── cam_property.cpp                  # gain, exposure, ptz ctrl
│   ├── extend_cam_ctrl.cpp               # camera stream. capture ctrl
|   ├── core_io.cpp                       # string manipulation for parser
|   ├── json_parser.cpp                   # json parser
|   ├── batch_cmd_parser.cpp              # BatchCmd.txt parser
|   ├── ui_control.cpp                    # control GUI
│   ├── uvc_extension_unit_ctrl.cpp       # all uvc extension unit ctrl
│   └── v4l2_device.cpp                   # udev ctrl
│   
└── test
    └── main.c                              
```
---
## Declarations 
_Auto white balance_, _gamma correction_ and _auto brightness and contrast_ are done by mainly using opencv, since histogram matrix calculations are involved, enabling these features will slow down the streaming a lot.
_Auto exposure_ is usually implemented on sensor/isp, which when enabled, won't further slow down the streaming, need to check with camera driver for auto exposure support. If some sensor don't have AE support built-in, this button won't work.

## Regarding Exposure Time Calculation
Since _Linux V4L2 API_ doesn't provide a easier way to tell you what exact exposure time in _ms_ like what _Windows_ does, here is the explanation for helping you figuring out your camera's current exposure time in _ms_:

```Exposure time = exposure_time_line_number / (frame_rate * total_line_number)```

, where 
1. __exposure_time_line_number__ is the value that display is linux camera tool exposure control
2. you can get __frame_rate__ from log _Get Current Frame Rate=_, don't refer to _Current Fps_ that displays in _cam_, that's the frame rate your PC can process, not the physically USB received frame rate from camera.
3. __total_line_number__ is usually around the height of the camera's current resolution, usually slightly larger than that since VD needs blanking. To get the exact __total_line_number__, you might want to read __VTS__ register value for your sensor, which might be a hassle for you so I will only tell you this roughly calculation method...
4. Noticing for exposure time change in Linux Camera Tool Control GUI, the exposure time range will usually be over the camera's resolution height. If you have the exposure value set to be over camera's resolution height, it might affect your frame rate, since camera exposure time line number is greater than the total line number, that will slow down your camera's frame rate. But this varies from different sensors. Some sensors don't allow that happen, so the exposure time will stick the same when it is over total line number.
## Run Camera Tool

```leopard_cam```

### Examples
  
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
4. Unexpected quit results in defunct process, use ps to see if the leopard_cam still alive,
   if it is, use __killall -9 leopard_cam__ to kill the zombie process

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




