#/bin/sh
sudo apt-get update
sudo apt-get -y install build-essential cmake pkg-config    #for g++ build tool
sudo apt-get -y install clang                               #for clang, optional
sudo apt-get -y install libgtk-3-dev                        #for gtk3 GUI
sudo apt-get -y install libomp-dev                          #for openmp
sudo apt-get -y install v4l-utils libv4l-dev                #for v4l2
sudo apt-get -y install libudev-dev                         #for udev
sudo apt-get -y install libopencv-dev                       #for opencv dependencies
sudo apt-get -y install libjson-c-dev                       #for json parser
sudo apt-get -y install libpng12-dev                        #for capturing png
