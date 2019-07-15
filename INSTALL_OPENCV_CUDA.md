# Use CUDA to Accelerate OpenCV
This OpenCV CUDA support is mainly for accelerating Leopard Imaging USB3.0 raw camera image processing capability
Refer to:https://docs.opencv.org/3.4/d6/d15/tutorial_building_tegra_cuda.html

## Test Environment
- ubuntu 18.04 64-bit
- NVIDIA GTX 1080
- OpenCV 3.4.3 
- cuda 10.1
  
## Preinstalled Libraries
Some of them might be unnecessary but I installed these all...
```sh
sudo apt-get-repository universe
sudo apt-get update
sudo apt-get install libtbb-dev
sudo apt-get install libpng12-dev
sudo apt-get install libswscale-dev
sudo apt-get install libavcodec-dev
sudo apt-get install flake8
sudo apt-get install pylint
sudo apt-get install libvtk6-dev
sudo apt-get install libopenblas-dev
sudo apt-get install libopenblas-base
sudo apt-get install x11-apps
sudo apt-get install libhdf5-serial-dev
sudo apt-get install libavformat-dev
sudo apt-get install libjasper-dev
sudo apt-get install libtiff5-dev
sudo apt-get install libjpeg8-dev
sudo apt-get install ffmpeg libv4l-dev
sudo apt-get remove libeigen3-dev # couldn't find the right directory while make, so I remove it
```

## Install Driver
1. remove the GPU driver you previously installed
```sh
sudo apt-get --purge remove nvidia-*
sudo apt-get --purge xserver-xorg-video-nouveau
```

2. NVIDIA driver couldn't be installed under GUI environment, switch to command line, press ```Ctrl + Alt + F1``` or try ```F1``` until ```F7``` see which can switch to x server. After switching to your x server, type the following
```sh
sudo /etc/init.d/lightdm stop
sudo ./NVIDIA-Linux-x86_64-390.25.RUN
```

3. For installation you just choose yes or no. Most important thing is that when it asks your if using nv config file, choose yes, otherwise, x-window won't start when restarting pc. After finishing installing, run
```sh
# to restart your pc
sudo update-initramfs -u
```
If you run
```sh
sudo lcpci | grep nouveau
```
and see no output, it means you have disabled open source nouvear correctly

## Install CUDA
1. Similarly, we enter the command line x-server for installing __CUDA__.(Enter it by trying from ```ctl+alt+F1~F6``` to enter, ```ctrl+alt+F7``` is to return)
```sh
sudo /etc/init.d/lightdm stop
# switch to cuda installation package directory, and run
sudo ./cuda_10.1xxxxxx.run
sudo /etc/init.d/lightdm start # open GUI environment
```

2. Add this following lines in your ```~/.bashrc ```(or ```~/.zshrc```)
```
# for cuda
export PATH=/usr/local/cuda-10.1/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda-10.1/lib64:$LD_LIBRARY_PATH
export CUDA_ROOT=$CUDA_ROOT:/usr/local/cuda-10.1/
export CUDA_INC_DIR=/usr/local/cuda-10.1/include:$CUDA_INC_DIR
```

and then source it
```sh
source ～/.bashrc # for normal bash terminal
source ~/.zshrc   # for zsh terminal
```

## Install cuDNN
__cuDNN__ is a deep learning framework designed to work on GPU accelerating. 
1. go to website: https://developer.nvidia.com/cudnn to download it
2. then run the following
```sh
tar xvf cudnn-10.1-linux-x64-v7.1.tgz
cd cuda
sudo cp cuda/include/cudnn.h /usr/local/cuda-10.1/include
sudo cp -a cuda/lib64/libcudnn*  /usr/local/cuda-10.1/lib64
sudo chmod a+r /usr/local/cuda-10.1/lib64/libcudnn*
```

## Testing on CUDA Installation
```sh
cd ~/samples/NIVIDIA_CUDA_10.1_Samples/1_Utilities/deviceQuery
make
./deviceQuery # after run that, you will able to see Result = PASS

nvidia-smi # see info about your nvidia GPU's current workload
nvcc -V i  # see info about nvidia CUDA compiler 
```

## Rebuild OpenCV
```sh
mkdir opencv_cuda
cd opencv_cuda
git clone https://github.com/opencv/opencv.git
cd opencv
git checkout -b 3.4.3 # I tried 4.0.0, couldn't compile ><

cd ..
git clone https://github.com/opencv/opencv_contrib.git
cd opencv_contrib
git checkout -b 3.4.3 #opencv_contrib should be same version as your OpenCV

cd ../opencv
mkdir build 
cd build

# just my builds, you can adjust it to fit your need
cmake -D WITH_CUDA=ON -D WITH_OPENGL=ON -D ENABLE_PRECOMPILED_HEADERS=OFF -D BUILD_opencv_cudacodec=OFF -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON -D WITH_GTK=ON -D WITH_GTK3=ON -D WITH_GTK_2_X=OFF -D WITH_VTK=OFF -D CUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-10.1 -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..

# this will still take a long time to make
# I did that before I got off work, so don't know the exact time it will take
make -j12
sudo make install -j12
```



