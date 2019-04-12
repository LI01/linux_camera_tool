# tool macros
CROSS_COMPILE ?=
CPP      	:= $(CROSS_COMPILE)clang++ #clang++ is better for debug but slower on compile
#CPP      	:= $(CROSS_COMPILE)g++ 
DBGFLAGS 	:= -g
CPPFLAGS 	:= -Wall -Wextra `pkg-config --cflags opencv gtk+-3.0` 
CPPOBJFLAGS	:= $(CPPFLAGS) -c 

LDLIBS = $(shell pkg-config --libs gtk+-3.0)
LDLIBCV = $(shell pkg-config --libs opencv)

LDFLAGS	+= \
		-fopenmp \
		-lv4l2 \
		-ludev \
		-lm \
		$(LDLIBS) \
		$(LDLIBCV)

# path marcros
SRC_PATH := src

APP := leopard_cam

SRCS := \
	test/main.cpp \
	$(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))

OBJS := $(SRCS:.cpp=.o)

all: $(APP)

# dependencies
%.o: %.c*
	@echo -e "\t" $(CPP) $(CPPOBJFLAGS) $< -o $@
	$(CPP) $(CPPOBJFLAGS) $(DBGFLAG) -o $@ $< -fopenmp


$(APP): $(OBJS)
	$(CPP) -o $@ $(OBJS) $(CPPFLAGS) $(LDFLAGS)

clean:
	-rm -f *.o $(OBJS)
	-rm -f $(APP)

