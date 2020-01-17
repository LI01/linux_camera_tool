## Dockerfile is a definition of how to build the image
# it will take some time to build the image...
FROM ubuntu:16.04
LABEL maintainer "lidanyu0904@gmail.com"
#ENV OPENCV_VERSION 3.4.2

#RUN apt-get update
## install all dependencies
RUN apt-get update && apt-get install -y \
        build-essential \
        cmake \
        pkg-config \
        libgtk-3-dev \
        libomp-dev \
        git \
        v4l-utils \
        libv4l-dev \
        libjson-c-dev \
        libopencv-dev \
        libudev-dev

## install OpenCV, prepare build, install
RUN mkdir ~/opencv_build && cd ~/opencv_build \
&& git clone https://github.com/opencv/opencv.git \
&& git clone https://github.com/opencv/opencv_contrib.git \
&& cd opencv \
&& mkdir build && cd build \
&& cmake -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D WITH_TBB=ON \
        -D BUILD_NEW_PYTHON_EXAMPLES=ON \
        -D BUILD_EXAMPLES=ON \
        -D WITH_OPENGL=ON \
        -D WITH_GTK=ON \
        -D WITH_GTK3=ON \
        -D WITH_GTK_2_X=OFF .. \
&& make -j6 && make install -j6 \
&& sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv.conf' \
&& ldconfig 

RUN git clone https://github.com/danyu9394/linux_camera_tool ~/linux_camera_tool \
&& git pull 
#sudo docker exec -it frosty_keller bash
