CROSS_COMPILE ?=

CPP            := $(CROSS_COMPILE)g++
CFLAGS	?= -O2 -W -Wall `pkg-config --cflags opencv gtk+-3.0` 
#CPPFLAGS := -g -Wall -Wextra $(LIB_PATH) $(INC_PATH)
CPPFLAGS := -g -Wall -Wextra `pkg-config --cflags opencv gtk+-3.0` 

# OpenCV trunk
LIB_PATH = -L/home/danyu/libs/opencv-trunk/release/installed/libs \
			-Wl, -rpath=/home/danyu/libs/opencv-trunk/release/installed/libs 

INC_PATH = -I/home/danyu/libs/opencv-trunk/release/installed/include

LDLIBS = $(shell pkg-config --libs gtk+-3.0)
LDLIBCV = $(shell pkg-config --libs opencv)
# Opencv 2.4.8
#CPPFLAGS += -L/home/danyu/libs/opencv-3.4.2/release/installed/libs \
	   -I/home/danyu/libs/opencv-3.4.2/release/installed/include

LDFLAGS	+= \
		-fopenmp \
		-lv4l2 \
		$(LDLIBS) \
		$(LDLIBCV)

APP := leopard_cam

SRCS := \
	test/main.cpp \
	src/uvc_extension_unit_ctrl.cpp \
	src/extend_cam_ctrl.cpp \
	src/ui_control.cpp \
	src/v4l2_device.cpp \
	src/cam_property.cpp
	

OBJS := $(SRCS:.cpp=.o)

all: $(APP)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<

$(APP): $(OBJS)
	$(CPP) -o $@ $(OBJS) $(CPPFLAGS) $(LDFLAGS)
clean:
	-rm -f *.o $(OBJS)
	-rm -f $(APP)

