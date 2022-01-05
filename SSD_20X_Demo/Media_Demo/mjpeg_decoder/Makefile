CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm
C_FLAGS  = $(COM_FLAGS) -std=gnu11
CPP_FLAGS  = $(COM_FLAGS) -std=gnu++11

INCLUDES  += -I./v4l2/inc -I./v4l2/src -I./v4l2/inc/internal -I. -I../../../project/release/include -I./libjpeg-turbo/include -I./libyuv/include

TARGET_NAME  = UvcPlayer

CPP_SRCS  =  libyuv_transform_C.cpp

C_SRCS    =  common.c main.c v4l2.c jdec.c sstardisp.c

C_SRCS += v4l2/src/libv4l2.c\
	v4l2/src/libv4lcontrol.c\
	v4l2/src/libv4lconvert.c\
	v4l2/src/libv4lprocessing.c\
	v4l2/src/v4l2convert.c\
	v4l2/src/whitebalance.c\
	v4l2/src/autogain.c\
	v4l2/src/rgbyuv.c\
	v4l2/src/jidctflt.c\
	v4l2/src/tinyjpeg.c\
	v4l2/src/log.c\
	v4l2/src/bayer.c\
	v4l2/src/crop.c\
	v4l2/src/flip.c\
	v4l2/src/gamma.c\
	v4l2/src/helper.c\
	v4l2/src/mr97310a.c\
	v4l2/src/pac207.c\
	v4l2/src/sn9c10x.c\
	v4l2/src/sn9c2028-decomp.c\
	v4l2/src/sn9c20x.c\
	v4l2/src/spca501.c\
	v4l2/src/spca561-decompress.c\
	v4l2/src/sq905c.c\
	v4l2/src/hm12.c

CPP_OBJS = $(patsubst %.cpp, %.cpp.o, $(CPP_SRCS))
C_OBJS = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_PATH  = -L. -L../../../project/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic -L./libjpeg-turbo/lib32 -L./libyuv/lib
LIB_NAME += -lm -lrt -lmi_vdec -lmi_sys -lmi_venc -lmi_divp -lmi_disp -lmi_panel -ldl -lmi_common -lturbojpeg -lyuv


.PHONY: all prepare clean

all: prepare $(TARGET_NAME) finish

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo


clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)

finish:
	@echo "make done"
	@echo "<<<<========================================================"
	@echo

$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
